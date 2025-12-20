#include <server.hpp>
#include <request.hpp>
#include <console.hpp>
#include <status.hpp>
#include <time.hpp>
#include <fstream>
#include <sys/socket.h>

void methodGet(int client, request& req, ctr& currentServer, long long startRequestTime) {

  std::string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<html><head><title>GET Method</title></head><body><h1>not found</h1></body></html>";

  std::size_t i = 0;
  while (i < currentServer.route(i).length()) {
    if (req.getPath() == currentServer.route(i).path()) {
      std::ifstream file;
      std::string sourcePath = currentServer.route(i).source();
      file.open(sourcePath.c_str());
      if (file.is_open()) {
        std::stringstream body;
        body << file.rdbuf();
        file.close();
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + body.str();
        console.METHODS(req.getMethod(), req.getPath(), 200, time::calcl(startRequestTime, time::clock()));
        send(client, response.c_str(), response.length(), 0);
        return;
      }
      break;
    }
    i++;
  }
  
  send(client, response.c_str(), response.length(), 0);
  console.METHODS(req.getMethod(), req.getPath(), 404, time::calcl(startRequestTime, time::clock()));
}