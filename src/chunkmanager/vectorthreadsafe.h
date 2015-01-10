#pragma once

#include "semaphore.h"
#include <vector>
#include <iostream>

template<class T>
class VectorThreadSafe {
public:

    VectorThreadSafe() {
        m_needUnlock = false;
    }
    ~VectorThreadSafe() {
    }

    void push(T element) {
        m_writeMutex.lock();

        m_vector.push_back(element);

        m_readSemaphore.notify();

        m_writeMutex.unlock();
    }

    void push(T* elements, int count) {
        m_writeMutex.lock();

        for (int i = 0; i < count; ++i) {
            m_vector.push_back(elements[i]);
        }

        m_readSemaphore.notify(count);

        m_writeMutex.unlock();
    }

    T pop() {
        T elmnt{};

        bool found = false;

        while (!found) {
            m_readSemaphore.wait();

            if (m_needUnlock) {
                return {};
            }

            m_writeMutex.lock();

            if (m_vector.size() == 0) {
                m_writeMutex.unlock();
                continue;
            }
            found = true;
            elmnt = m_vector.back();
            m_vector.pop_back();

            if (m_vector.size() > 0) {
                m_readSemaphore.notify();
            }

            m_writeMutex.unlock();
        }

        return elmnt;
    }

    void reset() {
        m_writeMutex.lock();

        m_vector.clear();
        m_readSemaphore.reset();

        m_writeMutex.unlock();
    }

    void unlock() {
        m_needUnlock = true;
        m_readSemaphore.notifyAll(1000);
    }

private:
    Semaphore m_readSemaphore;
    std::mutex m_writeMutex;
    std::vector<T> m_vector;

    std::atomic<bool> m_needUnlock;
};

