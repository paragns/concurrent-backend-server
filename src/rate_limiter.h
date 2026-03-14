#pragma once

#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>

class RateLimiter {
 public:
  // max_requests: max requests allowed
  // window_seconds: time window in seconds
  RateLimiter(int max_requests, int window_seconds);

  // Check if client can make a request
  // Returns true if request is allowed, false if rate limited
  bool allow_request(const std::string& client_ip);

 private:
  struct ClientState {
    int tokens;                                          // Available tokens
    std::chrono::steady_clock::time_point last_update;  // Last refill time
  };

  int max_tokens_;
  int refill_rate_;  // Tokens per second
  std::chrono::seconds window_;
  std::unordered_map<std::string, ClientState> clients_;
  mutable std::mutex limiter_mutex_;

  // Refill tokens based on time elapsed
  void refill_tokens(ClientState& state);
};
