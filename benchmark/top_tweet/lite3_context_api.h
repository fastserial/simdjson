#pragma once

#ifdef SIMDJSON_COMPETITION_LITE3

#include <stdio.h>

#include "top_tweet.h"


namespace top_tweet {

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

  bool run(simdjson::padded_string &json, int64_t max_retweet_count, top_tweet_result<StringType> &result) {
    result.retweet_count = -1;

    size_t statuses_ofs;
    if (lite3_ctx_get_arr(ctx, 0, "statuses", &statuses_ofs) < 0)
      return false;

    lite3_iter iter;
    if (lite3_ctx_iter_create(ctx, statuses_ofs, &iter) < 0)
      return false;
    
    size_t top_tweet_ofs;
    size_t tweet_ofs;
    int64_t retweet_count;
    while (lite3_ctx_iter_next(ctx, &iter, NULL, &tweet_ofs) == LITE3_ITER_ITEM) {
      if (lite3_ctx_get_i64(ctx, tweet_ofs, "retweet_count", &retweet_count) < 0)
        false;
      if (retweet_count <= max_retweet_count && retweet_count >= result.retweet_count) {
        result.retweet_count = retweet_count;
        top_tweet_ofs = tweet_ofs;
      }
    }
    if (result.retweet_count < 0)
      return false;

    lite3_str text;
    if (lite3_ctx_get_str(ctx, top_tweet_ofs, "text", &text) < 0)
      return false;
    result.text = LITE3_STR(ctx->buf, text);

    size_t user_ofs;
    if (lite3_ctx_get_obj(ctx, top_tweet_ofs, "user", &user_ofs) < 0)
      return false;
    lite3_str screen_name;
    if (lite3_ctx_get_str(ctx, user_ofs, "screen_name", &screen_name) < 0)
      return false;
    result.screen_name = LITE3_STR(ctx->buf, screen_name);

    // size_t json_length = json.size();
    // printf("[LITE3] json_length: %zu\n", json_length);
    return true;
  }
};

BENCHMARK_TEMPLATE(top_tweet, lite3_context_api)->UseManualTime();

} // namespace top_tweet

#endif // SIMDJSON_COMPETITION_LITE3
