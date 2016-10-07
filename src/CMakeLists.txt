add_executable(chatserver main.cpp Chatserver.cpp Greeter.cpp Client.cpp TcpBuf.cpp)
target_link_libraries(chatserver ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS chatserver RUNTIME DESTINATION bin)
