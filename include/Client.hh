#ifndef CLIENT_H
#define CLIENT_H

#include "TcpBuf.hh"

#include <iostream>
#include <thread>

class ChatServer;

// class for connected client

class Client
{
public:
    Client ( ChatServer &s, const int fd, int id ) : buf ( fd ), stream ( &buf ), server ( s ), id_ ( id ) {}

    ~Client() {
        if ( th.joinable() ) {
            // don't destroy the client while it's still running
            std::clog<<"client thread still running while destructor called!"<<std::endl;
            th.join();
        }
    }

    // starting and stopping client thread
    void start();
    void stop();
    void join();

    // operators
    template <typename T> friend std::ostream& operator<< ( Client&, const T& );
    bool operator== ( const Client& rhs ) const;

    // accessors
    std::string& name();
    int id() const;

private:
    // function passed to std::thread constructor
    void run();

    // stream stuff
    TcpBuf buf;
    std::iostream stream;

    ChatServer &server;
    std::thread th;
    std::string nick;
    int id_;
    char command_prefix = '/';
};

// template function, must be inline
template <typename T>
std::ostream& operator<< ( Client& out, const T& data )
{
    return out.stream<< ( data );
}

#endif // CLIENT_H
