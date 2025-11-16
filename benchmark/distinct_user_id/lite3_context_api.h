#pragma once

#ifdef SIMDJSON_COMPETITION_LITE3

#include <stdio.h>

#include "distinct_user_id.h"


namespace distinct_user_id {

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

  bool run(simdjson::padded_string &json, std::vector<uint64_t> &result) {
    size_t statuses_ofs;
    if (lite3_ctx_get_arr(ctx, 0, "statuses", &statuses_ofs) < 0)
      return false;

    lite3_iter iter;
    if (lite3_ctx_iter_create(ctx, statuses_ofs, &iter) < 0)
      return false;
    
    size_t tweet_ofs;
    while (lite3_ctx_iter_next(ctx, &iter, NULL, &tweet_ofs) == LITE3_ITER_ITEM) {
      do {
        size_t user_ofs;
        if (lite3_ctx_get_obj(ctx, tweet_ofs, "user", &user_ofs) < 0)
          return false;
        uint64_t id;
        if (lite3_ctx_get_i64(ctx, user_ofs, "id", (int64_t *)&id) < 0)
          return false;
        result.push_back(id);
      } while (lite3_ctx_get_obj(ctx, tweet_ofs, "retweeted_status", &tweet_ofs) == 0);
    }
    // size_t json_length = json.size();
    // printf("[LITE3] json_length: %zu\n", json_length);
    return true;
  }
};

BENCHMARK_TEMPLATE(distinct_user_id, lite3_context_api)->UseManualTime();

} // namespace distinct_user_id

#endif // SIMDJSON_COMPETITION_LITE3
