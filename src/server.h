#pragma once

#include <string>
#include "request_handler.h"
#include "thread_pool.h"

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
  bool running_;
  RequestHandler request_handler_;
  ThreadPool thread_pool_;

  void accept_loop();
  void handle_client(int client_socket);
};
