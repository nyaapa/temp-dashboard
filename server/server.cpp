#include <iostream>
#include <string>
#include <cstring>
#include <array>
#include <arpa/inet.h>

#include <exception>
#include <string>
#include <algorithm>
#include <iomanip>

#include "scommon.hpp"
#include "ch.hpp"

#include "lib/json.hpp"

namespace {
	constexpr int PORT = 4224;
	constexpr size_t TIMEOUT_SEC = 10;
};

int main() {
	using nlohmann::json;

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

	// if (listen(sockfd, 5))
	// 	throw std::runtime_error(std::string("Can't listen socket: ") + strerror(errno));

	while (true) {
		// // temp::scommon::socket_guard newsockfd{ accept(sockfd, nullptr, nullptr) };
		// // if (newsockfd < 0) 
		// // 	throw std::runtime_error(std::string("Can't accept connection: ") + strerror(errno));

		// timeval  ts;
		// ts.tv_sec = TIMEOUT_SEC;
		// ts.tv_usec = 0;
		// if (setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, &ts, sizeof(ts))) 
		// 	throw std::runtime_error(std::string("error setting rcvtimeo socket opt: ") + strerror(errno));

		std::array<char, 256> buffer;
		buffer.fill(0);
		auto n = recvfrom(sockfd, buffer.begin(), buffer.size(),  MSG_WAITALL, 0, 0);

		if (n < 0) {
			std::cerr << "error reading from socket: " << strerror(errno) << "\n";
			continue;
		}

		try {
			auto j = json::parse(buffer.begin(), &buffer[n]);
			
			auto name = j.find("name");
			if (name == j.end())
				throw std::runtime_error("No name");

			for (auto key : {"deca_pm25", "deca_pm10", "deca_kelvin", "deca_humidity", "co2", "gas"}) {
				if (auto it = j.find(key); it != j.end()) {
					temp::ch::post(R"(
							insert into Air.Redmond (
								EventTime,
								Source,
								EventType,
								Value
							) 
							values 
							(now(), ')" + std::string(name.value()) + std::string("', '") + std::string(key) + "'," + std::to_string(it.value().get<uint>()) + R"( )
						)", TIMEOUT_SEC);
				}
			}

			std::cout << std::string(buffer.begin(), buffer.end()) << "...\n";
		} catch (const std::exception& e) {
			std::cerr << "Failed parse " << std::quoted(std::string(buffer.begin(), buffer.end())) << ": " << e.what();
		}


		// if (idx && idx < (buffer.size() - 1)) {
		// 	buffer[idx] = '\0';
		// 	std::cout << type << ": " << buffer.data() << "\n";


		// 	if (write(newsockfd, "ok\n", 3) < 0) 
		// 		std::cerr << "error writing to socket: " << strerror(errno) << "\n";
		// } else { 
		// 	if (!std::equal(buffer.begin(), buffer.begin() + std::size("ping"), "ping\n")) {
		// 		buffer.back() = '\0';
		// 		std::cout << "read: '" << std::string(buffer.data()) << "'\n";
		// 	}

		// 	if (write(newsockfd, "nope\n", 5) < 0) 
		// 		std::cerr << "error writing to socket: " << strerror(errno) << "\n";
		// }
	}

	return 0; 
}
