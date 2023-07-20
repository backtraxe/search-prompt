#pragma once

#include <string>

class Client final {
  private:
    int fd;

  public:
    Client(const char *ip, int port);

    ~Client();

    void communicate();

    bool doRequest(const std::string &query);
};