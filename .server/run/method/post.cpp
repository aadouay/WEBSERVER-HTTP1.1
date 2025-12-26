#include <server.hpp>
#include <request.hpp>
#include <console.hpp>
#include <status.hpp>
#include <time.hpp>
#include <sys/socket.h>
#include <response.hpp>
#include <fstream>
class urlencoder {
  private:
    std::map<std::string, std::string> _fields;
  public:
    void parseBodyContent(std::string& content);
};

void urlencoder::parseBodyContent(std::string& content)
{
  std::size_t i = 0;
  while (i < content.length())
  {
    std::string key;
    std::string value;
    // split with &
    std::size_t pos = content.find('&', i);
    std::string pair;
    if (pos != std::string::npos)
    {
      pair = content.substr(i, pos - i);
      i = pos + 1;
    }
    else
    {
      pair = content.substr(i);
      i = content.length();
    }
    // split with =
    std::size_t pos_eq = pair.find('=');
    if (pos_eq != std::string::npos)
    {
      key = pair.substr(0, pos_eq);
      value = pair.substr(pos_eq + 1);
    }
    else
    {
      key = pair;
      value = "";
    }
    this->_fields[key] = value;
  }
}

void handle_multipart(const std::string& content, request& req) {
  std::string boundaryname = req.getHeaders().at("Content-Type");
  std::size_t boundary_pos = boundaryname.find("boundary=");
  if (boundary_pos != std::string::npos){
    std::string boundary = "--" + boundaryname.substr(boundary_pos + 9);
    std::string boundary_break = boundary + "--";
    std::size_t i = 0;
    while (i < content.length()) {
      std::size_t boundary_start = content.find(boundary, i);
      std::size_t boundary_break_pos = content.find(boundary_break, i);
      if (boundary_break_pos != std::string::npos) {
        break;
      }
      if (boundary_start != std::string::npos){
        std::size_t boundary_end = content.find(boundary, boundary_start + boundary.length());
        if (boundary_end != std::string::npos) {
          std::string part = content.substr(boundary_start + boundary.length() + 2, boundary_end - (boundary_start + boundary.length() + 2));
          if (part.find("Content-Disposition: form-data;") != std::string::npos)
          {
            std::size_t filename_pos = part.find("filename=");
            if (filename_pos != std::string::npos) {
              std::string filename = part.substr(filename_pos + 10, part.find("\r\n", filename_pos) - (filename_pos + 10) - 1);
              std::size_t data_start = part.find("\r\n\r\n");
              if (data_start != std::string::npos) {
                std::string filedata = part.substr(data_start + 4, part.length() - (data_start + 4) - 2);
                std::string filepath = "./uploads/" + filename;
                std::ofstream outfile(filepath.c_str());
                outfile << filedata;
                outfile.close();
              }
            }
            else {
              // Handle regular form field if necessary
              std::size_t name_pos = part.find("name=");
              if (name_pos != std::string::npos) {
                std::string name = part.substr(name_pos + 6, part.find("\r\n", name_pos) - (name_pos + 6) - 1);
                std::size_t data_start = part.find("\r\n\r\n");
                if (data_start != std::string::npos) {
                  std::string fielddata = part.substr(data_start + 4, part.length() - (data_start + 4) - 2);
                  std::string stored_data = "Field Name: " + name + ", Value: " + fielddata + "\n";
                  std::ofstream outfile("./uploads/form_fields.txt");
                  outfile << stored_data;
                  outfile.close();
                }
              }
            }
          }
        }
        i = boundary_end;
      }
      else {
        break;
      }
    }
  }
}

void handle_json(const std::string& content) {
  std::ofstream outfile("./uploads/data.json");
  outfile << content;
  outfile.close();
}


void methodPost(int client, request& req, ctr& currentServer, long long startRequestTime) {
  (void)currentServer;
  if (req.getBody().empty()) {
    std::map<std::string, std::string> headers;
    std::string body = "";
    response res(client, startRequestTime, 400, headers, body, req);
    res.sendResponse();
    return;
  }

  if (req.getHeaders().find("Content-Length") != req.getHeaders().end() && req.getHeaders().at("Content-Length") != "0") {
    std::string contentType = req.getHeaders().at("Content-Type");
    if (contentType.find("application/x-www-form-urlencoded") != std::string::npos)
    {
      std::string contentstored = req.getBody();
      urlencoder urlencoder;
      urlencoder.parseBodyContent(contentstored);
      std::map<std::string, std::string> headers;
      std::string body = "Data received:\n" + contentstored;
      response(client, startRequestTime, 200, headers, body, req).sendResponse();
      return;
    }
    else if (contentType.find("multipart/form-data") != std::string::npos)
    {
      std::string contentbody = req.getBody();
      handle_multipart(contentbody, req);
      std::map<std::string, std::string> headers;
      std::string body = "Multipart data received and processed.";
      response(client, startRequestTime, 200, headers, body, req).sendResponse();
      return;
    }
    else if (contentType.find("text/plain") != std::string::npos)
    {
      std::string contentstored = req.getBody();
      std::map<std::string, std::string> headers;
      std::string body = "Data received:\n" + contentstored;
      response(client, startRequestTime, 200, headers, body, req).sendResponse();
      return;
    }
    else if (contentType.find("application/json") != std::string::npos)
    {
      std::string content = req.getBody();
      handle_json(content);
      std::map<std::string, std::string> headers;
      std::string body = "JSON Data received:\n" + content;
      response(client, startRequestTime, 200, headers, body, req).sendResponse();
      return;
    }
    else {
      std::map<std::string, std::string> headers;
      std::string body = "";
      response(client, startRequestTime, 415, headers, body, req).sendResponse();
      return;
    }
  }
  else {
    std::map<std::string, std::string> headers;
    std::string body = "";
    response(client, startRequestTime, 411, headers, body, req).sendResponse();
    return;
  }
}