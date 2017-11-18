project(temp-server)

cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "-std=c++1z -W -Wall -Werror")
set(CMAKE_CXX_LINK_FLAGS "-lcurl")

add_executable(monitor monitor.cpp)
add_executable(server server.cpp)

install(DIRECTORY service/ DESTINATION /etc/systemd/system/) 
install(CODE "execute_process(COMMAND systemctl enable smonitor.timer)")
install(CODE "execute_process(COMMAND systemctl start smonitor.timer)")
