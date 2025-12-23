#pragma once
#include <iostream>
#include <sstream>
#include <map>
#include <status.hpp>

class response {
  private:
    unsigned int _code;
    std::map<std::string, std::string> _headers;
    std::string _body;

  public:
    response(unsigned int code, std::map<std::string, std::string> headers, std::string body)
    : _code(code), _headers(headers), _body(body) {}

    void send(void) {
      std::stringstream response;
      response << "HTTP/1.1 " << this->_code << " " << status(this->_code).message() << "\r\n";

      for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++) {
        response << it->first << ": " << it->second << "\r\n";
      }

      response << "\r\n";
      response << this->_body;
    }
};