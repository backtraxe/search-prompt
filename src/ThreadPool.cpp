#include <thread>

#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t threadNum) : isRunning(true), size(threadNum) {
    for (size_t i = 0; i < threadNum; i++) {
        threads.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->mtx);
                    this->condVar.wait(lock, [this] {
                        return !this->isRunning || !this->taskQueue.empty();
                    });
                    if (!this->isRunning && this->taskQueue.empty()) {
                        return;
                    }
                    task = std::move(this->taskQueue.front());
                    this->taskQueue.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mtx);
        isRunning = false;
    }
    condVar.notify_all();
    for (auto &thread : threads) {
        thread.join();
    }
}