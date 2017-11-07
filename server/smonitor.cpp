#include <array>
#include <algorithm>
#include <iostream>
#include <exception>
#include <string>
#include <cstring>

#include "error.h"
#include "string.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "netdb.h"

#include "ch.hpp"

namespace {
  constexpr char SERVER_HOST[] = "0.0.0.0";
  constexpr size_t SERVER_PORT = 7339;
  constexpr size_t TIMEOUT_SEC = 10;

  class socket_guard {
    public:
      socket_guard() {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
      }

      operator int() {
        return sockfd;
      }

      ~socket_guard() {
        if (sockfd > 0)
          close(sockfd);
      }
    private:
      int sockfd;
  };
};

bool is_alive() {
  auto server = gethostbyname(SERVER_HOST);
  if (!server)
    throw std::runtime_error(std::string("no such host: ") + strerror(errno));

  socket_guard sockfd;
  if (sockfd < 0) 
    throw std::runtime_error(std::string("error opening socket: ") + strerror(errno));

  struct sockaddr_in serv_addr;
  std::memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  std::memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
  serv_addr.sin_port = htons(SERVER_PORT);

  int flags = 0;
  if( (flags = fcntl(sockfd, F_GETFL, 0)) < 0)
    throw std::runtime_error(std::string("error getting socket flags: ") + strerror(errno));

  if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
    throw std::runtime_error(std::string("error adding O_NONBLOCK socket flag: ") + strerror(errno));

  struct timeval  ts;
  ts.tv_sec = TIMEOUT_SEC;
  if (connect(sockfd, (sockaddr*) &serv_addr, sizeof(serv_addr))) {
    if (errno != EINPROGRESS) {
      std::cerr << "error connecting: " << strerror(errno) << "\n";
      return false;
    }

    fd_set  rset, wset;
    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;
    if(auto ret = select(sockfd + 1, &rset, &wset, NULL, &ts); ret <= 0){
      if (ret)
        throw std::runtime_error(std::string("error select: ") + strerror(errno));
      std::cerr << "connection timeouted\n";
      return false;
    }

    int error;
    socklen_t len = sizeof(error);
    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
        if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
          throw std::runtime_error(std::string("error getting socket opt: ") + strerror(errno));
        } else if (error) {
          std::cerr << "error socket: " << strerror(error) << "\n";
          return false;
        }
    } else
        return false;
  }

  if(fcntl(sockfd, F_SETFL, flags) < 0)
    throw std::runtime_error(std::string("error removing O_NONBLOCK socket flag: ") + strerror(errno));

  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &ts, sizeof(ts)))
    throw std::runtime_error(std::string("error setting rcvtimeo socket opt: ") + strerror(errno));

  if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &ts, sizeof(ts)))
    throw std::runtime_error(std::string("error setting sndtimeo socket opt: ") + strerror(errno));

  if (write(sockfd, "\n", 1) < 0) {
    std::cerr << "error writing to socket: " << strerror(errno) << "\n";
    return false;
  } 

  std::array<char, 5> buffer;
  buffer.fill(0);
  if (read(sockfd, &buffer[0], buffer.size()) < 0) {
    std::cerr << "error reading from socket: " << strerror(errno) << "\n";
    return false;
  }

  return std::equal(buffer.begin(), buffer.end(), "nope\n");
}

int main(void) {
  temp::ch::post(R"(
    insert into Temperature.Monitor (
      App, 
      EventTime, 
      Alive
    ) 
    values 
      ('Server', now(), )" + std::string(is_alive() ? "1" : "0") + R"( )
  )", TIMEOUT_SEC);
  return 0;
}
