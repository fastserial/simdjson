#pragma once

#ifdef SIMDJSON_COMPETITION_LITE3

#include <stdio.h>

#include "partial_tweets.h"


namespace partial_tweets {

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

  bool run(simdjson::padded_string &json, std::vector<tweet<std::string_view>> &result) {
    size_t statuses_ofs;
    if (lite3_ctx_get_arr(ctx, 0, "statuses", &statuses_ofs) < 0)
      return false;

    lite3_iter iter;
    if (lite3_ctx_iter_create(ctx, statuses_ofs, &iter) < 0)
      return false;
    
    lite3_val *val;
    size_t tweet_ofs;
    while (lite3_ctx_iter_next(ctx, &iter, NULL, &tweet_ofs) == LITE3_ITER_ITEM) {
      result.emplace_back();
      auto& tweet = result.back();
      if (lite3_ctx_get(ctx, tweet_ofs, "created_at", &val) < 0)
        return false;
      tweet.created_at = lite3_val_str(val);
      if (lite3_ctx_get_i64(ctx, tweet_ofs, "id", (int64_t *)&tweet.id) < 0)
        return false;
      if (lite3_ctx_get(ctx, tweet_ofs, "text", &val) < 0)
        return false;
      tweet.result = lite3_val_str(val);
      if (lite3_ctx_get_i64(ctx, tweet_ofs, "in_reply_to_status_id", (int64_t *)&tweet.in_reply_to_status_id) < 0)
        tweet.in_reply_to_status_id = 0;
      
      size_t user_ofs;
      if (lite3_ctx_get_obj(ctx, tweet_ofs, "user", &user_ofs) < 0)
        return false;
      if (lite3_ctx_get_i64(ctx, user_ofs, "id", (int64_t *)&tweet.user.id) < 0)
        return false;
      if (lite3_ctx_get(ctx, user_ofs, "screen_name", &val) < 0)
        return false;

      tweet.user.screen_name = lite3_val_str(val);
      if (lite3_ctx_get_i64(ctx, tweet_ofs, "retweet_count", (int64_t *)&tweet.retweet_count) < 0)
        return false;
      if (lite3_ctx_get_i64(ctx, tweet_ofs, "favorite_count", (int64_t *)&tweet.favorite_count) < 0)
        return false;
    }
    // size_t json_length = json.size();
    // printf("[LITE3] json_length: %zu\n", json_length);
    return true;
  }
};

BENCHMARK_TEMPLATE(partial_tweets, lite3_context_api)->UseManualTime();

} // namespace partial_tweets

#endif // SIMDJSON_COMPETITION_LITE3
