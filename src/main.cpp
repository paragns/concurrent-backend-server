#include <iostream>
#include <signal.h>

#include "server.h"

static Server* g_server = nullptr;

void signal_handler(int signal) {
  if (signal == SIGINT) {
    std::cout << "\nShutting down..." << std::endl;
    if (g_server) {
      g_server->stop();
    }
  }
}

int main() {
  const int PORT = 8080;

  Server server(PORT);
  g_server = &server;

  // Register signal handler for Ctrl+C
  signal(SIGINT, signal_handler);

  std::cout << "Starting Concurrent Backend Server (Chapter 1)..." << std::endl;

  if (!server.start()) {
    std::cerr << "Failed to start server" << std::endl;
    return 1;
  }

  return 0;
}
