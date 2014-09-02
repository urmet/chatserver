#include "Greeter.hh"
#include "Client.hh"

#include <poll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <thread>
#include <cstring>
#include <ostream>


static struct pollfd make_pollfd ( int fd, short flags = POLLIN );

Greeter::Greeter ( const int port, const int just_die, ChatServer &_s ) : stop_now ( just_die ), server ( _s )
{
    // open listening port
    if ( !open_listening_port ( port ) ) {
        throw std::runtime_error ( "could not open listening port" );
    }
    in_thread = std::thread ( [this] {return input_thread();} );
}

void Greeter::input_thread()
{
    struct pollfd socks[2];
    socks[0] = make_pollfd ( listen_fd );
    socks[1] = make_pollfd ( stop_now );
    bool keep_going = true;
    while ( keep_going ) {
        // wait for something to happen
        poll ( socks, 2, -1 );
        for ( auto & socks_pf : socks ) {
            if ( ! ( socks_pf.revents & POLLIN ) ) {
                continue; // next please

                /*
                 * New incoming connection
                 */
            } else if ( socks_pf.fd == listen_fd ) {
                socks_pf.revents = 0; // reset revents right away, as we might modify the vector
                auto new_fd = accept ( listen_fd, nullptr, nullptr );
                if ( new_fd > 0 ) {
                    server.postMessage ( {NewConnection, new_fd, std::string() } );
                    std::clog<<"New connection, fd = "<<new_fd<<std::endl;
                } else {
                    std::perror ( "accept() failed" );
                    abort();
                }
                break; // vector has been modified, not safe to continue looping

                /*
                 * Normal socket ready
                 */
            } else if ( socks_pf.fd == stop_now ) {
                keep_going = false;
                for ( auto sock : socks ) {
                    shutdown ( sock.fd, SHUT_RD ); // no more incoming messages
                }
                // notify chatserver
                server.postMessage ( {ShutItAllDown, 0, std::string() } );
                break;
            } else {
                std::clog<<"WTF? shouldn't get here"<<std::endl;
                abort();
                break; // reset loop
            }
            socks_pf.revents = 0;
        }
    }
}


bool Greeter::open_listening_port ( int _portno )
{
    bool ret = true;
    int ok = 1, status;
    struct addrinfo* servinfo, hints;
    auto portno = std::to_string ( _portno );

    memset ( &hints, 0, sizeof hints );
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE | AI_V4MAPPED; // dual-stack

    if ( ( status = getaddrinfo ( nullptr, portno.c_str(), &hints, &servinfo ) ) != 0 ) {
        std::cout << "getaddrinfo error: " << gai_strerror ( status ) << std::endl;
        ret = false;
    }

    listen_fd = socket ( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol );
    if ( setsockopt ( listen_fd, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof ok ) == -1 ) {
        std::perror ( "setsockopt" );
        ret = false;
    }

    int flags = fcntl ( listen_fd, F_GETFL, 0 );
    fcntl ( listen_fd, F_SETFL, flags | O_NONBLOCK );

    if ( bind ( listen_fd, servinfo->ai_addr, servinfo->ai_addrlen ) == -1 ) {
        std::perror ( "bind() failed" );
        ret = false;
    }

    if ( listen ( listen_fd, 10 ) == -1 ) {
        std::perror( "listen() failed" );
        ret = false;
    }

    freeaddrinfo ( servinfo );

    return ret;
}

static pollfd make_pollfd ( int fd, short flags )
{
    return pollfd({fd, flags, 0});
}

void Greeter::join()
{

    if ( in_thread.joinable() ) {
        in_thread.join();
    }

}
