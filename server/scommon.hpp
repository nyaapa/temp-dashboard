#pragma once

#include "sys/socket.h"
#include "unistd.h"

namespace temp::scommon {
	class socket_guard {
		public:
			socket_guard() {
				sockfd = socket(AF_INET, SOCK_STREAM, 0);
			}

			socket_guard(int sockfd) : sockfd{sockfd} {}

			socket_guard(const socket_guard&) = delete;
			socket_guard(socket_guard&&) = delete;
			socket_guard& operator=(const socket_guard&) = delete;
			socket_guard& operator=(socket_guard&&) = delete;

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
