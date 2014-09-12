#ifndef CHATSERVER_H
#define CHATSERVER_H

#include "MessageQueue.hh"

#include <map>
#include <string>
#include <memory>

class Client;

// one fugly typedef
typedef void ( *ChatCommand ) ( const int sender, std::string command, std::map<int,std::shared_ptr<Client> >& clientlist );

enum MessageTag {
    MessageIn,
    NewConnection,
    ClosedConnection,
    Command,
    ShutItAllDown // notification from main thread
};

struct Message {
    MessageTag tag;
    int id;
    std::string data;
};


class ChatServer
{
public:
    // super simple constructor
    ChatServer() : counter ( 0 ) {};

    void run();
    void postMessage ( Message&& m) {in_queue.push(std::forward<Message>(m));};
    void registerCommand ( std::string name, ChatCommand funk );

private:
    std::map<int, std::shared_ptr<Client>> clients;
    std::map<int, std::shared_ptr<Client>> prelogin;
    std::map<std::string, ChatCommand> cmd_map;
    MessageQueue< Message > in_queue;
    void checkAndSetName ( Message& );

    // keeps track of total number of connections
    int counter;
};

#endif
