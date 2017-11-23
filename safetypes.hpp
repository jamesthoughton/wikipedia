#ifndef SAFETYPES_H

#define SAFETYPES_H

#include <deque>
#include <unordered_set>
#include <mutex>
#include <functional>
#include <atomic>

#include "logging.hpp"

extern std::atomic<bool> searching;

template<typename T>
class SafeQueue {
  private:
    std::deque<T> queue;
    std::mutex queue_mutex;

    std::mutex cond_mutex;
    std::condition_variable cond;
    unsigned id;

  public:
    SafeQueue(unsigned id) {
        this->id = id;
        log("Starting queue " + std::to_string(id));
    };
    ~SafeQueue() {};

    void push(T item) {
        std::lock_guard<std::mutex> lk(queue_mutex);
        queue.emplace_back(item);
        cond.notify_one();
    };

    void notify_all(void) {
        cond.notify_all();
    };

    T wait_for_element() {
        T front;
        if(!searching)
            return front;
        std::unique_lock<std::mutex> lk(cond_mutex);
        cond.wait(lk, [this, &front]{
            if(!searching)
                return true;
            std::lock_guard<std::mutex> inner_lock(queue_mutex);
            if(queue.size() > 0) {
                front = queue.front();
                queue.pop_front();
                return true;
            } else {
                return false;
            }
        });
        return front;
    };

};

template<typename T>
class SafeSet {
  private:
    std::unordered_set<T> set;
    std::mutex set_mutex;
  public:
    SafeSet() {};
    ~SafeSet() {};
};

#endif // SAFETYPES_H