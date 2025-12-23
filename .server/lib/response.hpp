#pragma once
#include <iostream>
#include <sstream>
#include <map>
#include <status.hpp>
#include <console.hpp>
#include <request.hpp>
#include <time.hpp>
#include <sys/socket.h>

class response {
  private:
    unsigned int _client;
    long long _startRequestTime;
    unsigned int _code;
    std::map<std::string, std::string> _headers;
    std::string _body;
    request _request;

  public:
    response(
      unsigned int client,
      long long startRequestTime,
      unsigned int code,
      std::map<std::string, std::string> headers,
      std::string body,
      request request
    )
    : _client(client), _startRequestTime(startRequestTime), _code(code), _headers(headers), _body(body), _request(request) {}

    void sendResponse(void) {
      std::stringstream response;
      response << "HTTP/1.1 " << this->_code << " " << status(this->_code).message() << "\r\n";

      for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++) {
        response << it->first << ": " << it->second << "\r\n";
      }

      response << "\r\n";
      response << this->_body;

      send(this->_client, response.str().c_str(), response.str().length(), 0);
      console.METHODS(this->_request.getMethod(), this->_request.getPath(), this->_code, time::calcl(this->_startRequestTime, time::clock()));
    }
};