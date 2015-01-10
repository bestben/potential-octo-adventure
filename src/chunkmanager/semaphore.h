#pragma once

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    Semaphore(int count = 0) : m_count(count) {}

    ~Semaphore() {
        m_cv.notify_all();
    }

    void notify(int count = 1) {
        std::unique_lock<std::mutex> lck(m_mtx);
        m_count += count;
        m_cv.notify_one();
    }

    void notifyAll(int count = 1) {
        std::unique_lock<std::mutex> lck(m_mtx);
        m_count += count;
        m_cv.notify_all();
    }

    void wait() {
        std::unique_lock<std::mutex> lck(m_mtx);

        while(m_count == 0){
            m_cv.wait(lck);
        }
        m_count--;
    }

    void reset() {
        std::unique_lock<std::mutex> lck(m_mtx);
        m_count = 0;
    }

private:
    std::mutex m_mtx;
    std::condition_variable m_cv;
    int m_count;
};
