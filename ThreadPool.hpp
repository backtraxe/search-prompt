#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include "SocketInfo.hpp"

/**
 * @brief
 *
 */
class ThreadPool final {
  public:
    ThreadPool();
    ~ThreadPool();
    ThreadPool(const ThreadPool &taskpool) = delete;
    ThreadPool operator=(const ThreadPool &taskpool) = delete;

    /**
     * @brief
     *
     * @param threadNum
     */
    void init(int threadNum);

    /**
     * @brief
     *
     */
    void stop();

    /**
     * @brief
     *
     * @param socketInfo
     */
    void addTask(SocketInfo *socketInfo);

    /**
     * @brief
     *
     */
    void doWork();

  private:
    /**
     * @brief
     *
     */
    void removeAllTask();

    /**
     * @brief 是否运行中
     *
     */
    bool m_bRunning;

    /**
     * @brief 线程池
     *
     */
    std::vector<std::shared_ptr<std::thread>> m_threads;

    /**
     * @brief
     *
     */
    std::mutex m_mutex;

    /**
     * @brief
     *
     */
    std::condition_variable m_cond_var;

    /**
     * @brief
     *
     */
    std::deque<std::shared_ptr<SocketInfo>> m_waitingQueue;
};
