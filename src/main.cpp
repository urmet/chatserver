#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>
#include <map>

#include "Client.hh"
#include "Greeter.hh"
#include "ChatServer.hh"
#include "util.hh"


static void cmd_test ( int sender, std::string cmd, std::map<int,std::shared_ptr<Client> >& clientlist );
static void cmd_msg ( int sender, std::string cmd, std::map<int,std::shared_ptr<Client> >& clientlist );
static void int_handler ( int );

// buu, globlaalsed muutujad, BUUU!
int poison_pill[2]; // for starting shutdown from signal handler

int main ( int argc, char **argv )
{
    if ( argc<2 ) {
        std::cout<<"usage: "<<argv[0]<<" [-d] port" << std::endl;
        return 0;
    }

    if ( std::string ( argv[1] ) == "-d" ) {
        // shift argv, leave clog alone
        argv[1] = argv[2];
    } else {
        // disable clog
        std::clog.rdbuf ( new nullbuf() );
    }

    int port_num = -1;
    try {
        port_num = std::stoi ( argv[1] );
    } catch ( std::invalid_argument ) {
        std::cout<<"port number must be a number"<<std::endl;
        return 1;
    }

    // set up socket and handler for terminating the server
    socketpair ( AF_LOCAL, SOCK_STREAM, 0, poison_pill );
    signal ( SIGINT, int_handler );

    ChatServer cs;
    cs.registerCommand ( "test", cmd_test );
    cs.registerCommand ( "msg", cmd_msg );

    // opens the listening and starts feeding incoming connections
    // to the server instance
    Greeter listen ( port_num, poison_pill[1], cs );

    // start server main loop
    // run in separate thread so we can catch signals here
    std::thread chatroom ( &ChatServer::run, std::ref ( cs ) );

    // wait here until server and listening thread stop
    chatroom.join();
    listen.join();
    return 0;
}

void int_handler ( int )
{
    write ( poison_pill[0], "s", 2 );
}

// pointless command, spams everyone without revealing the sender
static void cmd_test ( int, std::string args, std::map<int,std::shared_ptr<Client> >& clientlist )
{
    auto spam = args.empty() ? "Böö!" : args;
    for ( auto &victim : clientlist ) {
        *victim.second<<spam<<std::endl;
    }
}

// private messages
static void cmd_msg ( int id, std::string args, std::map<int,std::shared_ptr<Client> >& clientlist )
{
    auto &sender = *clientlist[id];
    auto first_space = std::string::npos;
    if(args.empty()) {
        sender<<"who do you want to write to?\n";
        return;
    }
    if((first_space = args.find_first_of(' '))==std::string::npos) {
        sender<<"pls write a msg too, plox\n";
        return;
    }
    auto target_name = args.substr(0, first_space);
    for ( auto &c : clientlist ) {
        if(c.second->name() == target_name) {
            *c.second<<sender.name()<<" whispers to you: "
                     <<args.substr(first_space+1)<<std::endl;
            return;
        }
    }
    sender<<"There's noone named like that here"<<std::endl;
}
