#include <mutex>
#include <thread>

#include "ThreadPool.hpp"

ThreadPool::ThreadPool(int threadNum) : isRunning(true), size(threadNum) {
    for (size_t i = 0; i < threadNum; i++) {
        threads.emplace_back();
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> guard(mtx);
        isRunning = false;
    }
    for (auto &thread : threads) {
        thread.join();
    }
}

template <class F, class... Args>
void ThreadPool::addTask(F &&f, Args &&...args) {
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
}
