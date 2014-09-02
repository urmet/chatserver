#include "TcpBuf.hh"
#include "Client.hh"
#include "ChatServer.hh"

#include <string>
#include <iostream>

void Client::start()
{
    th = std::thread ( &Client::run, this );
}

void Client::run()
{
    std::string buf;
    while ( stream.good() ) {
        std::getline ( stream, buf );
        if ( stream.eof() ) {
            break;
        }
        if ( buf[0] == command_prefix ) {
            server.postMessage ( {Command, id_, buf.substr ( 1 ) } );
        } else {
            server.postMessage ( {MessageIn, id_, buf} );
        }
    }
    std::clog<<"input loop ended"<<std::endl;
    server.postMessage ( {ClosedConnection, id_, ""} );
}

bool Client::operator== ( const Client& rhs ) const
{
    return id() == rhs.id();
}

std::string& Client::name()
{
    return nick;
}

void Client::stop()
{
    buf.stop(); // closes socket and causes getline loop to break out.
}

void Client::join()
{
    if ( th.joinable() ) {
        th.join();
    }
    return;
}

int Client::id() const
{
    return id_;
}
