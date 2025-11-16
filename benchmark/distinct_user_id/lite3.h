#pragma once

#ifdef SIMDJSON_COMPETITION_LITE3

#include <stdio.h>

#include "distinct_user_id.h"


namespace distinct_user_id {

struct lite3 {
  using StringType=std::string_view;

  uint8_t *buf;
  size_t buflen;
  size_t bufsz;

  lite3() : buf(NULL), buflen(0), bufsz(1024*1024) {
    buf = (uint8_t *)malloc(bufsz);
    if (!buf)
      perror("[Lite3] Failed to allocate memory");
    if (lite3_json_dec_file(buf, &buflen, bufsz, TWITTER_JSON) < 0)
      perror("[Lite3] Failed to decode TWITTER_JSON");
    // printf("[Lite3] buflen: %zu\n", buflen);
    // printf("[Lite3] bufsz: %zu\n", bufsz);
  }

  ~lite3() {
    free(buf);
    buf = NULL;
  }

  bool run(simdjson::padded_string &json, std::vector<uint64_t> &result) {
    size_t statuses_ofs;
    if (lite3_get_arr(buf, buflen, 0, "statuses", &statuses_ofs) < 0)
      return false;

    lite3_iter iter;
    if (lite3_iter_create(buf, buflen, statuses_ofs, &iter) < 0)
      return false;
    
    size_t tweet_ofs;
    while (lite3_iter_next(buf, buflen, &iter, NULL, &tweet_ofs) == LITE3_ITER_ITEM) {
      do {
        size_t user_ofs;
        if (lite3_get_obj(buf, buflen, tweet_ofs, "user", &user_ofs) < 0)
          return false;
        uint64_t id;
        if (lite3_get_i64(buf, buflen, user_ofs, "id", (int64_t *)&id) < 0)
          return false;
        result.push_back(id);
      } while (lite3_get_obj(buf, buflen, tweet_ofs, "retweeted_status", &tweet_ofs) == 0);
    }
    // size_t json_length = json.size();
    // printf("[LITE3] json_length: %zu\n", json_length);
    return true;
  }
};

BENCHMARK_TEMPLATE(distinct_user_id, lite3)->UseManualTime();

} // namespace distinct_user_id

#endif // SIMDJSON_COMPETITION_LITE3
