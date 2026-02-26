#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "server/server.hpp"

// Function to print usage instructions for running the server
void printUsage(const char* programName) {
  std::cout << "Usage: " << programName << " [ServerAddress] [port]\n";
  std::cout << "If no arguments are provided, the server defaults to running "
               "on 127.0.0.1 12345\n";
}

int main(int argc, char* argv[]) {
  // Defaults
  std::string serverAddress = "127.0.0.1";
  int port = 12345;

  try {
    if (argc == 1) {
      // Do nothing, default
    } else if (argc == 3) {
      serverAddress = argv[1];
      port = std::stoi(argv[2]);

      if (port < 1024 || port > 65535) {
        throw std::invalid_argument(
            "Invalid port number. Must be between 1024 and 65535.");
      }
    } else {
      printUsage(argv[0]);
      return EXIT_FAILURE;
    }

    // Create a server instance with the given parameters
    Server server(serverAddress, port, 30);
    server.start();  // Start the server
  } catch (const std::exception& e) {
    // Catch and display any errors that occur
    std::cerr << "Error: " << e.what() << std::endl;
    printUsage(argv[0]);  // Print usage instructions again
    return EXIT_FAILURE;  // Exit with error status
  }

  return EXIT_SUCCESS;  // Exit successfully
}
