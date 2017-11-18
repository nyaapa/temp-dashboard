project(temp-server)

cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "-std=c++1z -W -Wall -Werror -O3")
set(CMAKE_CXX_LINK_FLAGS "-lcurl")

add_executable(temp-server-mon monitor.cpp)
add_executable(temp-server server.cpp)

install(DIRECTORY service/ DESTINATION /etc/systemd/system/) 
install(CODE "execute_process(COMMAND systemctl enable temp-server-mon.timer)")
install(CODE "execute_process(COMMAND systemctl start temp-server-mon.timer)")
