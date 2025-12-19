#include <server.hpp>
#include <console.hpp>
#include <time.hpp>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <request.hpp>

std::string getNetworkIP();
int run(long long start) {
  for (std::size_t i = 0; i < server.length(); i++) {

    (void)start;

    request req = request("GET /hello#?test=b HTTP/1.1\r\n\r\ns");

    std::cout << "Method: " << req.getMethod() << std::endl;
    std::cout << "Path: " << req.getPath() << std::endl;
    std::cout << "HTTP: " << req.getHTTP() << std::endl;
    std::map<std::string, std::string> headers = req.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
      std::cout << "Header: " << it->first << " => " << it->second << std::endl;
    }
    std::cout << "Body: " << req.getBody() << std::endl;

    if (req.getBadRequest())
      console.METHODS(req.getMethod(), req.getPath(), req.getBadRequest(), 1);
  }
  return 0;
}