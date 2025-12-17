#include <server.hpp>
#include <console.hpp>

int run(long long start) {
  (void)start;
  
  for (std::size_t i = 0; i < server.length(); i++) {
    ctr& s = server[i];
    std::cout << "Server #" << (i + 1) << ":" << std::endl;
    std::cout << "  Port:        " << s.port() << std::endl;
    std::cout << "  Name:        " << s.name() << std::endl;
    std::cout << "  Version:     " << s.version() << std::endl;
    std::cout << "  Root:        " << s.root() << std::endl;
    std::cout << "  Index:       " << s.index() << std::endl;
    std::cout << "  Not Found:   " << s.notfound() << std::endl;
    std::cout << "  Error Page:  " << s.servererror() << std::endl;
    std::cout << "  Log File:    " << s.log() << std::endl;
    std::cout << "  Upload Dir:  " << s.uploaddir() << std::endl;
    std::cout << "  Body Limit:  " << s.bodylimit() << " bytes" << std::endl;
    std::cout << "  Timeout:     " << s.timeout() << " ms" << std::endl;
    std::cout << "  Routes:      " << s.length() << std::endl;
    for (std::size_t j = 0; j < s.length(); j++) {
      rt& route = s.route(j);
      std::cout << "    Route #" << (j + 1) << ":" << std::endl;
      std::cout << "      Path:    " << route.path() << std::endl;
      std::cout << "      Source:  " << route.source() << std::endl;
      std::cout << "      Methods: ";
      for (std::size_t k = 0; k < route.length(); k++) {
        std::cout << route.method(k);
        if (k < route.length() - 1)
          std::cout << ", ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
  return 0;
}