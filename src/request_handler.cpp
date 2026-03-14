#include "request_handler.h"

#include <algorithm>
#include <iostream>
#include <sstream>

RequestHandler::RequestHandler() {}

std::string RequestHandler::handle_request(const std::string& request) {
  // Tokenize the request
  std::vector<std::string> tokens = tokenize(request);

  if (tokens.empty()) {
    return "ERROR: Empty request\n";
  }

  std::string command = tokens[0];

  // Convert command to uppercase for case-insensitive matching
  std::transform(command.begin(), command.end(), command.begin(), ::toupper);

  if (command == "SET") {
    return handle_set(tokens);
  } else if (command == "GET") {
    return handle_get(tokens);
  } else if (command == "DELETE") {
    return handle_delete(tokens);
  } else if (command == "PING") {
    return handle_ping(tokens);
  } else {
    return "ERROR: Unknown command: " + command + "\n";
  }
}

std::vector<std::string> RequestHandler::tokenize(const std::string& input) {
  std::vector<std::string> tokens;
  std::istringstream stream(input);
  std::string token;

  while (stream >> token) {
    tokens.push_back(token);
  }

  return tokens;
}

std::string RequestHandler::handle_set(const std::vector<std::string>& tokens) {
  // SET requires 3 tokens: SET key value
  if (tokens.size() != 3) {
    return "ERROR: SET requires 2 arguments (key value)\n";
  }

  std::string key = tokens[1];
  std::string value = tokens[2];

  store_[key] = value;

  std::cout << "[SET] " << key << " = " << value << std::endl;

  return "OK\n";
}

std::string RequestHandler::handle_get(const std::vector<std::string>& tokens) {
  // GET requires 2 tokens: GET key
  if (tokens.size() != 2) {
    return "ERROR: GET requires 1 argument (key)\n";
  }

  std::string key = tokens[1];

  auto it = store_.find(key);
  if (it != store_.end()) {
    std::cout << "[GET] " << key << " -> " << it->second << std::endl;
    return it->second + "\n";
  } else {
    std::cout << "[GET] " << key << " -> NOT FOUND" << std::endl;
    return "NOT_FOUND\n";
  }
}

std::string RequestHandler::handle_delete(
    const std::vector<std::string>& tokens) {
  // DELETE requires 2 tokens: DELETE key
  if (tokens.size() != 2) {
    return "ERROR: DELETE requires 1 argument (key)\n";
  }

  std::string key = tokens[1];

  auto it = store_.find(key);
  if (it != store_.end()) {
    store_.erase(it);
    std::cout << "[DELETE] " << key << std::endl;
    return "OK\n";
  } else {
    std::cout << "[DELETE] " << key << " -> NOT FOUND" << std::endl;
    return "NOT_FOUND\n";
  }
}

std::string RequestHandler::handle_ping(const std::vector<std::string>& tokens) {
  std::cout << "[PING]" << std::endl;
  return "PONG\n";
}
