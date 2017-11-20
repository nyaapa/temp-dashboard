#include <iostream>
#include <string>
#include <cstring>
#include <array>
#include <arpa/inet.h>

#include <exception>
#include <string>
#include <algorithm>

#include "scommon.hpp"
#include "ch.hpp"

namespace {
	constexpr int PORT = 7339;
	constexpr size_t TIMEOUT_SEC = 10;
};

int main() {
	sockaddr_in serv_addr;
	temp::scommon::socket_guard sockfd;
	if (sockfd < 0) 
		throw std::runtime_error(std::string("Can't open socket: ") + strerror(errno));

	std::memset((char*) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);
	if (bind(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		throw std::runtime_error(std::string("Can't bind socket: ") + strerror(errno));

	if (listen(sockfd, 5))
		throw std::runtime_error(std::string("Can't listen socket: ") + strerror(errno));

	while (true) {
		temp::scommon::socket_guard newsockfd{ accept(sockfd, nullptr, nullptr) };
		if (newsockfd < 0) 
			throw std::runtime_error(std::string("Can't accept connection: ") + strerror(errno));

		timeval  ts;
		ts.tv_sec = TIMEOUT_SEC;
		ts.tv_usec = 0;
		if (setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, &ts, sizeof(ts))) 
			throw std::runtime_error(std::string("error setting rcvtimeo socket opt: ") + strerror(errno));

		std::array<char, 128> buffer;
		buffer.fill(0);
		if (read(newsockfd, &buffer[0], buffer.size()) < 0) {
			std::cerr << "error reading from socket: " << strerror(errno) << "\n";
			continue;
		}

		size_t idx = 0;
		while (idx < buffer.size() && buffer[idx] >= '0' && buffer[idx] <= '9') 
			++idx;

		if (idx && idx < (buffer.size() - 1) && buffer[idx] == 'M' && buffer[idx + 1] == 'K') {
			buffer[idx] = '\0';
			std::cout << "temp: " << buffer.data() << "\n";

			temp::ch::post(R"(
    				insert into Temperature.Bay222 (
		      	  	  EventTime, 
      			  	  MegaKelvin
    				) 
	    			values 
   			   	  	  (now(), )" + std::string(buffer.data()) + R"( )
				)", TIMEOUT_SEC);
		} else if (!std::equal(buffer.begin(), buffer.begin() + std::size("ping"), "ping\n")) {
			buffer.back() = '\0';
			std::cout << "read: '" << std::string(buffer.data()) << "'\n";
		}
	}

	return 0; 
}
