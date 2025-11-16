#pragma once

#ifdef SIMDJSON_COMPETITION_LITE3

#include <stdio.h>

#include "find_tweet.h"


namespace find_tweet {

struct lite3 {
  using StringType=std::string_view;

  uint8_t *buf;
  size_t buflen;
  size_t bufsz;

  lite3() : buf(NULL), buflen(0), bufsz(1024*1024) {
    buf = (uint8_t *)malloc(bufsz);
    int ret;
    if ((ret = lite3_json_dec_file(buf, &buflen, bufsz, TWITTER_JSON)) < 0) {
      perror("[LITE3] Failed to decode TWITTER_JSON");
      printf("[LITE3] Failed to decode TWITTER_JSON\terrno: %i\n", errno);
    } 
    // printf("[LITE3] buflen: %zu\n", buflen);
    // printf("[LITE3] bufsz: %zu\n", bufsz);
  }

  ~lite3() {
    free(buf);
    buf = NULL;
  }

  bool run(simdjson::padded_string &json, uint64_t find_id, std::string_view &result) {
    size_t statuses_ofs;
    if (lite3_get_arr(buf, buflen, 0, "statuses", &statuses_ofs) < 0)
      return false;

    lite3_iter iter;
    if (lite3_iter_create(buf, buflen, statuses_ofs, &iter) < 0)
      return false;
    
    int64_t id = -1;
    size_t tweet_ofs;
    while (lite3_iter_next(buf, buflen, &iter, NULL, &tweet_ofs) == LITE3_ITER_ITEM) {
      if (lite3_get_i64(buf, buflen, tweet_ofs, "id", &id) < 0)
        return false;
      if (id == find_id)
        break;
    }
    if (id < 0)
      return false;

    lite3_str text;
    if (lite3_get_str(buf, buflen, tweet_ofs, "text", &text) < 0)
      return false;
    result = LITE3_STR(buf, text);

    // size_t json_length = json.size();
    // printf("[LITE3] json_length: %zu\n", json_length);
    return true;
  }
};

BENCHMARK_TEMPLATE(find_tweet, lite3)->UseManualTime();

} // namespace top_tweet

#endif // SIMDJSON_COMPETITION_LITE3
