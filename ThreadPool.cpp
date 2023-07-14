#include <iostream>
#include <memory>
#include <thread>

#include "ThreadPool.hpp"

ThreadPool::ThreadPool() : m_bRunning(false) {}

ThreadPool::~ThreadPool() { removeAllTask(); }

void ThreadPool::init(int threadNum) {
    if (threadNum <= 0) {
        threadNum = 5;
    }
    m_bRunning = true;
    for (int i = 0; i < threadNum; i++) {
        std::shared_ptr<std::thread> spThread;
        spThread.reset(new std::thread(std::bind(&ThreadPool::doWork, this)));
        m_threads.push_back(spThread);
    }
}

void ThreadPool::stop() {
    m_bRunning = false;
    m_cond_var.notify_all();
    for (auto &thread : m_threads) {
        if (thread->joinable()) {
            thread->join();
        }
    }
}

void ThreadPool::addTask(SocketInfo *socketInfo) {
    std::shared_ptr<SocketInfo> spSocketInfo;
    spSocketInfo.reset(socketInfo);
    {
        std::lock_guard<std::mutex> guard(std::mutex);
        m_waitingQueue.push_back(spSocketInfo);
    }
    m_cond_var.notify_one();
}

void ThreadPool::removeAllTask() {
    std::lock_guard<std::mutex> guard(std::mutex);
    for (auto &task : m_waitingQueue) {
        task.reset();
    }
    m_waitingQueue.clear();
}

void ThreadPool::doWork() {
    std::shared_ptr<SocketInfo> spSocketInfo;
    while (true) {
        {
            std::unique_lock<std::mutex> guard(m_mutex);
            while (m_waitingQueue.empty()) {
                if (!m_bRunning) {
                    break;
                }
                m_cond_var.wait(guard);
            }
            if (!m_bRunning) {
                break;
            }
            spSocketInfo = m_waitingQueue.front();
            m_waitingQueue.pop_front();
        }
        if (spSocketInfo == nullptr) {
            continue;
        }
        spSocketInfo->communicate();
        spSocketInfo.reset();
    }
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        std::cout << "thread " << std::this_thread::get_id() << " exit\n";
    }
}
