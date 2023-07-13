#include <chrono>
#include <condition_variable>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

class Task {
   public:
    virtual void doIt() { cout << "handle a task\n"; }

    virtual ~Task() { cout << "destroy a task\n"; }
};

class TaskPool final {
   public:
    TaskPool() : m_bRunning(false) {}

    ~TaskPool() { removeAllTask(); }

    TaskPool(const TaskPool &taskpool) = delete;

    TaskPool operator=(const TaskPool &taskpool) = delete;

    void init(int threadNum = 5) {
        if (threadNum <= 0) {
            threadNum = 5;
        }
        m_bRunning = true;
        for (int i = 0; i < threadNum; i++) {
            shared_ptr<thread> spThread;
            spThread.reset(new thread(bind(&TaskPool::threadFunc, this)));
            m_threads.push_back(spThread);
        }
    }

    void stop() {
        m_bRunning = false;
        m_cond_var.notify_all();
        for (auto &thread : m_threads) {
            if (thread->joinable()) {
                thread->join();
            }
        }
    }

    void addTask(Task *task) {
        shared_ptr<Task> spTask;
        spTask.reset(task);
        {
            lock_guard<mutex> guard(mutex);
            m_taskList.push_back(spTask);
            cout << "add a task\n";
        }
        m_cond_var.notify_one();
    }

    void removeAllTask() {
        lock_guard<mutex> guard(mutex);
        for (auto &task : m_taskList) {
            task.reset();
        }
        m_taskList.clear();
    }

   private:
    void threadFunc() {
        shared_ptr<Task> spTask;
        while (true) {
            {
                unique_lock<mutex> guard(m_mutex);
                while (m_taskList.empty()) {
                    if (!m_bRunning) {
                        break;
                    }
                    m_cond_var.wait(guard);
                }
                if (!m_bRunning) {
                    break;
                }
                spTask = m_taskList.front();
                m_taskList.pop_front();
            }
            if (spTask == nullptr) {
                continue;
            }
            spTask->doIt();
            spTask.reset();
        }
        cout << "thread " << this_thread::get_id() << " exit\n";
    }

    list<shared_ptr<Task>> m_taskList;
    mutex m_mutex;
    condition_variable m_cond_var;
    bool m_bRunning;
    vector<shared_ptr<thread>> m_threads;
};

int main() {
    TaskPool threadPool;
    threadPool.init();
    Task *task = nullptr;
    for (int i = 0; i < 10; i++) {
        task = new Task();
        threadPool.addTask(task);
    }
    this_thread::sleep_for(chrono::seconds(5));
    threadPool.stop();
    return 0;
}