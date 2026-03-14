#pragma once

#include <string>
#include <atomic>
#include "request_handler.h"
#include "thread_pool.h"
#include "rate_limiter.h"

class Server {
 public:
  Server(int port);
  ~Server();

  bool start();
  void stop();
  bool is_running() const;

 private:
  int port_;
  int server_socket_;
  std::atomic<bool> running_;  // ← Changed to atomic for thread safety
  RequestHandler request_handler_;
  ThreadPool thread_pool_;
  RateLimiter rate_limiter_;  // ← NEW: Rate limiting per client IP

  void accept_loop();
  void handle_client(int client_socket, const std::string& client_ip);
};
