#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <cstddef> // std::size_t definition
#include <iostream>

// threadsafe FIFO queue
template <typename T>
class MessageQueue
{
public:
    struct node {
        node ( T elem ) : next ( nullptr ), data ( std::move ( elem ) ) {}
        node* next;
        T data;
    };

    MessageQueue ( const MessageQueue& ) = delete;
    MessageQueue () : head ( nullptr ),tail ( nullptr ) {
        // T might not have a default constructor
        // allocate raw memory because of that
        storage = static_cast<node*> ( operator new ( sizeof ( node ) ) );
    }

    ~MessageQueue() {
        while ( tail ) {
            node* old = tail;
            tail = old->next;
            delete old;
        }
    }

    void push ( T&& data ) {
        std::lock_guard<std::mutex> lock ( mtx );
        // teeme uue node
        // kui head == nullptr, siis on uus asi nii head kui tail
        // muul juhul head = n; oldhead->next = n;
        // if queue is empty, placement new copy construct
        // the thing inplace
        node *n;
        if ( head == nullptr ) {
            new ( storage ) node ( std::move ( data ) );
            n = storage;
        } else {
            n = new node ( std::move ( data ) );
        }
        auto oldhead = head;
        head = n;
        if ( oldhead == nullptr ) {
            std::clog<<"MsgQ "<<this<<" was empty"<<std::endl;
            tail = n;
        } else {
            std::clog<<"MsgQ "<<this<<" was not empty"<<std::endl;
            oldhead->next = head;
        }
        cv.notify_all();
        return;
    }

    T pop() {
        std::unique_lock<std::mutex> lock ( mtx );
        cv.wait ( lock, [this] {return !empty();} );
        T ret ( std::move ( tail->data ) );
        node* n = tail->next;

        if ( n == nullptr ) {
            tail->~node();
        } else {
            delete tail;
        }
        tail = n;
        if ( tail == nullptr ) {
            head = nullptr;
        }
        return ret;
    }

private:
    // actually a node, but without constructing
    node *storage;
    node  *head, *tail;
    std::mutex mtx;
    std::condition_variable cv;
    bool empty() {
        return tail == nullptr;
    }
};

#endif
