#ifndef GREETER_H
#define GREETER_H

#include <utility>
#include <thread>

#include "ChatServer.hh"

class Greeter
{
public:
    Greeter() = delete;
    Greeter ( const int port, const int die, ChatServer &_s );
    void join();

private:
    std::thread in_thread;
    void input_thread();
    bool open_listening_port ( int port );
    int listen_fd, stop_now;
    ChatServer &server;
};

#endif // include guard
