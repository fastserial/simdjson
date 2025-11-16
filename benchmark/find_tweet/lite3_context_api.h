#pragma once

#ifdef SIMDJSON_COMPETITION_LITE3

#include <stdio.h>

#include "find_tweet.h"


namespace find_tweet {

struct lite3_context_api {
  using StringType=std::string_view;

  lite3_ctx *ctx;

  lite3_context_api() : ctx(NULL) {
    ctx = lite3_ctx_create();
    if (!ctx)
      perror("[Lite3] Failed to create context");
    if (lite3_ctx_json_dec_file(ctx, TWITTER_JSON) < 0)
      perror("[Lite3] Failed to decode TWITTER_JSON");
    // printf("[Lite3] buflen: %zu\n", ctx->buflen);
    // printf("[Lite3] bufsz: %zu\n", ctx->bufsz);
  }

  ~lite3_context_api() {
    lite3_ctx_destroy(ctx);
    ctx = NULL;
  }

  bool run(simdjson::padded_string &json, uint64_t find_id, std::string_view &result) {
    size_t statuses_ofs;
    if (lite3_ctx_get_arr(ctx, 0, "statuses", &statuses_ofs) < 0)
      return false;

    lite3_iter iter;
    if (lite3_ctx_iter_create(ctx, statuses_ofs, &iter) < 0)
      return false;
    
    int64_t id = -1;
    size_t tweet_ofs;
    while (lite3_ctx_iter_next(ctx, &iter, NULL, &tweet_ofs) == LITE3_ITER_ITEM) {
      if (lite3_ctx_get_i64(ctx, tweet_ofs, "id", &id) < 0)
        return false;
      if (id == find_id)
        break;
    }
    if (id < 0)
      return false;

    lite3_str text;
    if (lite3_ctx_get_str(ctx, tweet_ofs, "text", &text) < 0)
      return false;
    result = LITE3_STR(ctx->buf, text);

    // size_t json_length = json.size();
    // printf("[LITE3] json_length: %zu\n", json_length);
    return true;
  }
};

BENCHMARK_TEMPLATE(find_tweet, lite3_context_api)->UseManualTime();

} // namespace find_tweet

#endif // SIMDJSON_COMPETITION_LITE3
