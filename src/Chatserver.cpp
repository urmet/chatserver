// local headers
#include "ChatServer.hh"
#include "Client.hh"

// c++ headers
#include <algorithm>
#include <iostream>
#include <map>


void ChatServer::run()
{
    bool keep_going = true;

    while ( keep_going ) {
        auto message = in_queue.pop(); // blocks until data data is availible

        switch ( message.tag ) {
        case MessageIn:
            if ( nick_unset.count ( message.id ) ) {
                checkAndSetName ( message );
                break;
            }

            // log all messages to stdout
            std::cout << "<" << message.id << ":" << clients[message.id]->name() << "> " << message.data << std::endl;

            // and then send it to everyone else
            for ( auto && client : clients ) {
                if ( client.first == message.id ) {
                    continue;    // skip self
                }
                *client.second << "<" << clients[message.id]->name() << "> " << message.data << std::endl;
            }
            break;

        case NewConnection: {
            // make the new client
            auto c = std::make_shared<Client> ( *this, message.id, counter++ );
            // start it up
            c->start();
            // shove it into the map
            nick_unset[c->id()] = c;
            // and ask annoying questions
            *c << "Please choose a name for yourself(ASCII alnum only)" << std::endl;
            break;
        }

        case ClosedConnection: {
            std::shared_ptr<Client> c;
            if ( clients.count ( message.id ) ) {
                c = clients[message.id];
                clients.erase ( message.id );
            } else {
                c = nick_unset[message.id];
                nick_unset.erase ( message.id );
            }
            // stop client thread
            c->stop();
            c->join();
            std::clog << "client " << message.id << " stopped" << std::endl;

            // and spam others, if client had a name
            if ( c->name().length() ) {
                for ( auto & it : clients ) {
                    *it.second << c->name() << " has disconnected from the chat." << std::endl;
                }
            }
            break;
        }

        case Command: {
            if ( clients.count ( message.id ) ) {
                auto &c = *clients[message.id];

                // extract command name
                auto space_offset = message.data.find_first_of ( ' ' );
                auto cmd = message.data.substr ( 0, space_offset );
                auto args = space_offset == std::string::npos ? "" : message.data.substr ( space_offset + 1 );

                if ( cmd == "quit" ) { // quit is builtin
                    std::clog << "quit command from " << message.id << std::endl;
                    c.stop();

                } else if ( cmd == "list" ) { // so is list
                    c<<"List of all commands: /quit /list ";
                    for ( auto &command : cmd_map ) {
                        c<<"/"<<command.first<<" ";
                    }
                    c<<"\n";

                } else if ( cmd_map.count ( cmd ) == 0 ) {
                    c<<"unknown command, use /list to show all commands"<<std::endl;
                } else {
                    cmd_map[cmd] ( message.id, args, clients );
                }
            } else {
                auto &c = *nick_unset[message.id];
                c<<"You can not use commands before choosing a name for yourself"<<std::endl;
            }
            break;
        }

        case ShutItAllDown:
            keep_going = false;
            for ( auto & c : clients ) {
                *c.second << "Server shutting down. Bye-bye, come again." << std::endl;
                c.second->stop(); // close client socket
                std::clog << "closed socket of client: " << c.first << std::endl;
                std::clog << "waiting to join thread of client" << std::endl;
                c.second->join();
            }
            break;

        default:
            // should be unpossible
            std::cerr << "unknown message tag from input thread: " << message.tag << std::endl;
        }
    } // while(true)

    return;
}

void ChatServer::registerCommand ( const std::string cmd, ChatCommand func )
{
    cmd_map[cmd] = func;
}

void ChatServer::checkAndSetName ( Message& m )
{
    Client &client = *nick_unset[m.id];
    // check for ASCII first
    if ( std::find_if ( m.data.begin(), m.data.end(),
        [] ( char & c ) { return !std::isalnum ( c ); } ) != m.data.end() ) {
        
        client << "Name contains other things than ASCII alnum, try again" << std::endl;
        return;
    }
    for ( const auto & c : clients ) {
        if ( c.second->name() == m.data ) {
            client << "Name already taken by someone else, try again" << std::endl;
            return;
        }
    }
    // seems alright. set the name
    client.name() = m.data;
    // and announce it to others
    for ( auto & c : clients ) {
        *c.second << m.data << " has joined the room, say hi!" << std::endl;
    }
    client<< "You can now annoy other chatters. Please behave nicely"<<std::endl;

    clients[m.id] = nick_unset[m.id];
    nick_unset.erase ( m.id );
}
