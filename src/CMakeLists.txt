add_executable(chatserver main.cpp Chatserver.cpp Greeter.cpp Client.cpp TcpBuf.cpp)
install(TARGETS chatserver RUNTIME DESTINATION bin)
