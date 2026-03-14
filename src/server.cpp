#include "server.h"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(int port)
    : port_(port), server_socket_(-1), running_(false), thread_pool_(4) {}

Server::~Server() {
  if (server_socket_ != -1) {
    close(server_socket_);
  }
}

bool Server::start() {
  // Create socket
  server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket_ == -1) {
    std::cerr << "Failed to create socket" << std::endl;
    return false;
  }

  // Allow reusing the address
  int opt = 1;
  if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt,
                 sizeof(opt)) == -1) {
    std::cerr << "Failed to set socket options" << std::endl;
    close(server_socket_);
    return false;
  }

  // Bind to port
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(port_);

  if (bind(server_socket_, (struct sockaddr*)&server_addr,
           sizeof(server_addr)) == -1) {
    std::cerr << "Failed to bind socket to port " << port_ << std::endl;
    close(server_socket_);
    return false;
  }

  // Listen for connections
  if (listen(server_socket_, 5) == -1) {
    std::cerr << "Failed to listen on socket" << std::endl;
    close(server_socket_);
    return false;
  }

  running_ = true;
  std::cout << "Server listening on port " << port_ << std::endl;

  // Accept loop
  accept_loop();

  return true;
}

void Server::stop() {
  running_ = false;
  thread_pool_.shutdown();
  if (server_socket_ != -1) {
    close(server_socket_);
    server_socket_ = -1;
  }
}

bool Server::is_running() const { return running_; }

void Server::accept_loop() {
  while (running_) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Accept incoming connection
    int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr,
                                &client_addr_len);
    if (client_socket == -1) {
      if (running_) {
        std::cerr << "Failed to accept connection" << std::endl;
      }
      continue;
    }

    std::cout << "Client connected: "
              << inet_ntoa(client_addr.sin_addr) << ":"
              << ntohs(client_addr.sin_port) << std::endl;

    // Enqueue task to handle client (don't block)
    thread_pool_.enqueue([this, client_socket]() {
      handle_client(client_socket);
    });
  }
}

void Server::handle_client(int client_socket) {
  const int BUFFER_SIZE = 1024;
  char buffer[BUFFER_SIZE];

  // Read from client
  int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

  if (bytes_read > 0) {
    buffer[bytes_read] = '\0';
    std::string request(buffer);

    std::cout << "Received: " << request << std::endl;

    // Process request with RequestHandler
    std::string response = request_handler_.handle_request(request);

    // Send response
    send(client_socket, response.c_str(), response.length(), 0);
  } else if (bytes_read == 0) {
    std::cout << "Client closed connection" << std::endl;
  } else {
    std::cerr << "recv() error" << std::endl;
  }

  // Close the client socket
  close(client_socket);
  std::cout << "Client disconnected" << std::endl;
}
