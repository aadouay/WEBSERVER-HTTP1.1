#include <server.hpp>
#include <request.hpp>
#include <response.hpp>
#include <error.hpp>
#include <console.hpp>
#include <status.hpp>
#include <time.hpp>
#include <fstream>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>

void methodDelete(int client, request& req, ctr& currentServer, long long startRequestTime) {

  // find matching route at config file
  rt* route = NULL;
  for (std::size_t i = 0; i < currentServer.length(); i++) {
    if (currentServer.route(i).path() == req.getPath()) {
      route = &currentServer.route(i);
      break;
    }
  }

  std::string sourcePathToDelete;

  if (!route) {
    // absolute path
    sourcePathToDelete = currentServer.root() + req.getPath();
  } else {
    sourcePathToDelete = route->source();

    // 405 method not allowed - DELETE not allowed on this route if exist
    if (
      route->method(0) != "DELETE" ||
      (route->length() > 1 && route->method(1) != "DELETE") ||
      (route->length() > 2 && route->method(2) != "DELETE")
    ) {
      ;
    }
  }

  // if (!(route->method(0) == "DELETE") || !(route->length() > 1
  // && route->method(1) == "DELETE") == false
  // || !(route->length() > 2 && route->method(2) == "DELETE") == false) {
  //   std::string response = "HTTP/1.1 405 Method Not Allowed\r\n"
  //                         "Allow: GET, POST\r\n"
  //                         "Content-Type: text/html\r\n"
  //                         "Content-Length: 32\r\n\r\n"
  //                         "<h1>405 Method Not Allowed</h1>";
  //   send(client, response.c_str(), response.length(), 0);
  //   return;
  // }

  std::string filePath = route->source();

  // Check if file exists
  struct stat fileStat;
  if (stat(filePath.c_str(), &fileStat) != 0) {
    // 404 Not Found - File doesn't exist
    std::string response = "HTTP/1.1 404 Not Found\r\n"
                          "Content-Type: text/html\r\n"
                          "Content-Length: 23\r\n\r\n"
                          "<h1>404 Not Found</h1>";
    send(client, response.c_str(), response.length(), 0);
    return;
  }

  // Check if it's a directory (can't delete directories via HTTP DELETE)
  if (S_ISDIR(fileStat.st_mode)) {
    // 403 Forbidden - Can't delete directories
    std::string response = "HTTP/1.1 403 Forbidden\r\n"
                          "Content-Type: text/html\r\n"
                          "Content-Length: 21\r\n\r\n"
                          "<h1>403 Forbidden</h1>";
    send(client, response.c_str(), response.length(), 0);
    return;
  }

  // Try to delete the file
  if (unlink(filePath.c_str()) == 0) {
    // 204 No Content - Successfully deleted (most common for DELETE)
    std::string response = "HTTP/1.1 204 No Content\r\n\r\n";
    send(client, response.c_str(), response.length(), 0);
    console.METHODS("DELETE", req.getPath(), 204, time::calcl(startRequestTime, time::clock()));
  } else {
    // 500 Internal Server Error - Delete failed
    std::string response = "HTTP/1.1 500 Internal Server Error\r\n"
                          "Content-Type: text/html\r\n"
                          "Content-Length: 35\r\n\r\n"
                          "<h1>500 Internal Server Error</h1>";
    send(client, response.c_str(), response.length(), 0);
  }

}