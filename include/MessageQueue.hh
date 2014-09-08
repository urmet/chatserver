#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>

// threadsafe FIFO queue
template <typename T>
class MessageQueue
{
public:
    void push ( T&& data ) {
        std::lock_guard<std::mutex> lock ( mtx );
        q.push(data);
        cv.notify_one();
        return;
    }
    T pop() {
        std::unique_lock<std::mutex> lock ( mtx );
        cv.wait ( lock, [this] {return !q.empty();} );
        T ret = q.front();
        q.pop();
        return ret;
    }
private:
    std::queue<T> q;
    std::mutex mtx;
    std::condition_variable cv;
};

#endif
