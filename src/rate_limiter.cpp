#include "rate_limiter.h"

#include <iostream>

RateLimiter::RateLimiter(int max_requests, int window_seconds)
    : max_tokens_(max_requests),
      refill_rate_(max_requests / window_seconds),
      window_(window_seconds) {
  if (refill_rate_ == 0) {
    refill_rate_ = 1;  // Minimum 1 token per second
  }
  std::cout << "RateLimiter created: max=" << max_requests
            << " requests per " << window_seconds << " seconds" << std::endl;
}

bool RateLimiter::allow_request(const std::string& client_ip) {
  std::unique_lock<std::mutex> lock(limiter_mutex_);

  // Get or create client state
  auto it = clients_.find(client_ip);
  if (it == clients_.end()) {
    // New client: start with max tokens
    ClientState state;
    state.tokens = max_tokens_;
    state.last_update = std::chrono::steady_clock::now();
    clients_[client_ip] = state;
    it = clients_.find(client_ip);
  }

  ClientState& state = it->second;

  // Refill tokens based on time passed
  refill_tokens(state);

  // Check if we have tokens
  if (state.tokens > 0) {
    state.tokens--;
    std::cout << "[RATE_LIMITER] " << client_ip << " allowed (tokens left: "
              << state.tokens << ")" << std::endl;
    return true;
  } else {
    std::cout << "[RATE_LIMITER] " << client_ip << " rate limited" << std::endl;
    return false;
  }
}

void RateLimiter::refill_tokens(ClientState& state) {
  auto now = std::chrono::steady_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::seconds>(now - state.last_update);

  // Add tokens based on elapsed time
  int tokens_to_add = elapsed.count() * refill_rate_;
  state.tokens = std::min(state.tokens + tokens_to_add, max_tokens_);
  state.last_update = now;
}
