#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <status.hpp>
#include <error.hpp>
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
    ctr _server;

  public:
    response(
      unsigned int client,
      long long startRequestTime,
      unsigned int code,
      std::map<std::string, std::string> headers,
      std::string body,
      request request,
      ctr server
    )
    : _client(client), _startRequestTime(startRequestTime), _code(code), _headers(headers), _body(body), _request(request), _server(server) {}

    void sendResponse(void) {
      std::stringstream response;
      response << "HTTP/1.1 " << this->_code << " " << status(this->_code).message() << "\r\n";

      for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++) {
        response << it->first << ": " << it->second << "\r\n";
      }

      response << "\r\n";
      if (
        (!this->_body.empty()) || (this->_code >= 100 && this->_code < 200) ||
        (this->_code >= 200 && this->_code < 300)
      ) {
        response << this->_body;
      }
      else if ((this->_code >= 300 && this->_code < 400) || (this->_code >= 400 && this->_code < 600)) {
        std::string errorPage = error(this->_code).page();
        std::stringstream codeAsStream;
        codeAsStream << this->_code;
        std::string fileProvidedPath = this->_server.errorPages()[codeAsStream.str()];

        if (!fileProvidedPath.empty()) {
          std::fstream fileProvidedToOpen(fileProvidedPath.c_str());
          if (fileProvidedToOpen) {
            std::stringstream R;
            R << fileProvidedToOpen.rdbuf();
            errorPage = R.str();
          }
        }

        response << errorPage;
      }

      send(this->_client, response.str().c_str(), response.str().length(), 0);
      console.METHODS(this->_request.getMethod(), this->_request.getPath(), this->_code, time::calcl(this->_startRequestTime, time::clock()));
    }

    void sendGETchunks(std::string file) {
      std::fstream fileToOpen(file.c_str());
      if (!fileToOpen) {
        this->_code = 404;
        this->sendResponse();
        return;
      }

      fileToOpen.seekg(0, std::ios::end);
      size_t fileSize = fileToOpen.tellg();
      fileToOpen.seekg(0, std::ios::beg);

      std::stringstream response;
      response << "HTTP/1.1 200 OK\r\n";
      response << "Content-Length: " << fileSize << "\r\n";

      for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++) {
        response << it->first << ": " << it->second << "\r\n";
      }

      response << "\r\n";
      send(this->_client, response.str().c_str(), response.str().length(), 0);

      const size_t CHUNK_SIZE = 8192; // 8KB
      char buffer[CHUNK_SIZE];
    
      while (fileToOpen.read(buffer, CHUNK_SIZE) || fileToOpen.gcount() > 0) {
        if (time::calcl(this->_startRequestTime, time::clock()) > static_cast<long long>(this->_server.timeout())) {
          fileToOpen.close();
          console.METHODS(this->_request.getMethod(), this->_request.getPath(), 408, time::calcl(this->_startRequestTime, time::clock()));
          return;
        }
        send(this->_client, buffer, fileToOpen.gcount(), 0);
      }

      fileToOpen.close();
      console.METHODS(this->_request.getMethod(), this->_request.getPath(), 200, time::calcl(this->_startRequestTime, time::clock()));
    }
};