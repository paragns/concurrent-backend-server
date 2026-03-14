#pragma once

#include <string>

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

  void accept_loop();
  void handle_client(int client_socket);
};
