#pragma once
#include <iostream>
#include <vector>

class rt {
  private:
    std::string _path;
    std::string _source;
    std::vector<std::string> _methods;
  public:
    inline std::string& path(void) throw() { return this->_path; }
    inline std::string& source(void) throw() { return this->_source; }
    inline std::string& method(unsigned int index) throw() { return this->_methods[index]; }

    inline std::size_t length(void) const throw() { return this->_methods.size(); }

    std::string& create(void) throw() {
      std::string newN;
      this->_methods.push_back(newN);
      return this->_methods.back();
    }
    std::string& add(const std::string& str) throw() {
      this->_methods.push_back(str);
      return this->_methods.back();
    }
};

class ctr {
  private:
    std::size_t _port;
    std::string _name;
    std::string _version;
    std::string _notfound;
    std::string _servererror;
    std::string _log;
    std::size_t _bodylimit;
    std::size_t _timeout;
    std::string _uploaddir;
    std::string _index;
    std::string _root;
    std::vector<rt> _routes;
  public:
    inline std::size_t& port(void) throw() { return this->_port; }
    inline std::string& name(void) throw() { return this->_name; }
    inline std::string& version(void) throw() { return this->_version; }
    inline std::string& notfound(void) throw() { return this->_notfound; }
    inline std::string& servererror(void) throw() { return this->_servererror; }
    inline std::string& log(void) throw() { return this->_log; }
    inline std::size_t& bodylimit(void) throw() { return this->_bodylimit; }
    inline std::size_t& timeout(void) throw() { return this->_timeout; }
    inline std::string& uploaddir(void) throw() { return this->_uploaddir; }
    inline std::string& index(void) throw() { return this->_index; }
    inline std::string& root(void) throw() { return this->_root; }

    ctr(): _port(0), _bodylimit(0), _timeout(0) {}

    rt& route(unsigned int index) throw() { return this->_routes[index]; }
    inline std::size_t length(void) const throw() { return this->_routes.size(); }

    rt& create(void) throw() {
      rt newN;
      this->_routes.push_back(newN);
      return this->_routes.back();
    }
    rt& add(const rt& route) throw() {
      this->_routes.push_back(route);
      return this->_routes.back();
    }
};

class Server {
  private:
    std::vector<ctr> _servers;
  public:
    inline std::size_t length(void) const throw() { return this->_servers.size(); }
    ctr& operator[](unsigned int index) throw() { return this->_servers[index]; }

    ctr& create(void) throw() {
      ctr newN;
      this->_servers.push_back(newN);
      return this->_servers.back();
    }
    ctr& add(const ctr& server) throw() {
      this->_servers.push_back(server);
      return this->_servers.back();
    }
};

extern Server server;
/* global object declaration, see ./extern.cpp for actual definition */