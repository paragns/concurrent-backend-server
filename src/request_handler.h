#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

class RequestHandler {
 public:
  RequestHandler();

  // Main entry point: process a client request
  std::string handle_request(const std::string& request);

 private:
  // The key-value store
  std::unordered_map<std::string, std::string> store_;
  mutable std::mutex store_mutex_;  // Protects store_ from concurrent access

  // Helper methods
  std::vector<std::string> tokenize(const std::string& input);
  std::string handle_set(const std::vector<std::string>& tokens);
  std::string handle_get(const std::vector<std::string>& tokens);
  std::string handle_delete(const std::vector<std::string>& tokens);
  std::string handle_ping(const std::vector<std::string>& tokens);
};
