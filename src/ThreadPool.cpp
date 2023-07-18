#include <mutex>
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

template <class F, class... Args>
auto ThreadPool::addTask(F &&f, Args &&...args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (!isRunning) {
            throw std::runtime_error("线程池已停止运行");
        }
        taskQueue.emplace([task]() { (*task)(); });
    }
    condVar.notify_one();
    return res;
}
