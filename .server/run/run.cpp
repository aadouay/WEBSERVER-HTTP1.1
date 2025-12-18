#include <server.hpp>
#include <console.hpp>
#include <time.hpp>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <ifaddrs.h>

std::string getNetworkIP() {
    struct ifaddrs *ifaddr, *ifa;
    
    if (getifaddrs(&ifaddr) == -1) {
        return "0.0.0.0";
    }
    
    std::string result = "0.0.0.0";
    
    for (ifa = ifaddr; ifa != 0; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == 0) continue;
        
        // Check for IPv4 address
        if (ifa->ifa_addr->sa_family == AF_INET) {
            char addressBuffer[INET_ADDRSTRLEN];
            void* addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr, addressBuffer, INET_ADDRSTRLEN);
            
            std::string ip = addressBuffer;
            // Skip loopback (127.x.x.x)
            if (ip.substr(0, 3) != "127") {
                result = ip;
                break;
            }
        }
    }
    
    freeifaddrs(ifaddr);
    return result;
}

int run(long long start) { (void)start;

  std::string networkIP = getNetworkIP();

  for (std::size_t i = 0; i < server.length(); i++) {

    // create socket
    int serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFD < 0) {
      console.issue("Failed to create socket for " + server[i].name());
      continue;
    }

    // Set socket options
    int opt = 1;
    setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(server[i].port()); // bind to specified port

    if (bind(serverFD, (struct sockaddr*)&address, sizeof(address)) < 0) {
      console.issue("Failed to bind socket for " + server[i].name());
      close(serverFD);
      continue;
    }

    // Listen for connections
    if (listen(serverFD, 10) < 0) {
      console.issue("Failed to listen on " + server[i].name());
      close(serverFD);
      continue;
    }

    console.init(server[i].port(), networkIP, server[i].name(), server[i].version());
    // console.success("Ready in " + (num2str(time::calc(start, time::clock()))) + "ms");

    // Accept and handle requests
    while (true) {
      struct sockaddr_in client_address;
      socklen_t client_len = sizeof(client_address);
      int client_fd = accept(serverFD, (struct sockaddr*)&client_address, &client_len);

      if (client_fd < 0) {
        continue;
      }

      // Read request (simple read)
      char buffer[4096] = {0};
      read(client_fd, buffer, sizeof(buffer) - 1);

      // Send HTTP response
      const char* response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Typpe: application/json\r\n" 
        "Content-Length: 5\r\n"
        "\r\n"
        "hello";

      send(client_fd, response, strlen(response), 0);
      close(client_fd);
    }

    close(serverFD);
  }
  return 0;
}