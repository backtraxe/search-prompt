#pragma once

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <vector>

/**
 * @brief 线程池。
 *
 */
class ThreadPool final {

  private:
    /**
     * @brief 线程是否运行中。
     *
     */
    bool isRunning;

    /**
     * @brief 线程池大小。
     *
     */
    int size;

    /**
     * @brief 线程数组。
     *
     */
    std::vector<std::thread> threads;

    /**
     * @brief 互斥量。
     *
     */
    std::mutex mtx;

    /**
     * @brief 条件变量。
     *
     */
    std::condition_variable condVar;

    /**
     * @brief 任务队列。
     *
     */
    std::queue<std::function<void()>> taskQueue;

  public:
    ThreadPool(size_t threadNum);
    ~ThreadPool();
    ThreadPool(const ThreadPool &taskPool) = delete;
    ThreadPool operator=(const ThreadPool &taskPool) = delete;

    /**
     * @brief 向任务队列中添加任务。
     *
     * @tparam F
     * @tparam Args
     */
    template <class F, class... Args>
    auto addTask(F &&f, Args &&...args)
        -> std::future<typename std::result_of<F(Args...)>::type>;
};
