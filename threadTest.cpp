#include <arpa/inet.h>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "Trie.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;

class Task {
public:
  Task(int fd, struct sockaddr_in *addr) : clientfd(fd), client_addr(addr) {}

  void doIt(Trie &trie, const int prompt_num) {
    // 数据交互
    char buffer[1024];
    int ret;
    while (true) {
      // 从客户端接收数据
      memset(buffer, 0, sizeof(buffer));
      ret = recv(clientfd, buffer, sizeof(buffer), 0);
      if (ret <= 0) {
        cout << "recv error\n";
        break;
      } else {
        cout << "接收成功，接收客户端 " << inet_ntoa(client_addr->sin_addr)
             << " 发送的内容：" << buffer << "\n";
      }

      // 字典树查询候选词
      string tmp_str = buffer;
      auto start_time = clock();
      auto dict = trie.prompt(tmp_str, prompt_num);
      auto end_time = clock();
      cout << "搜索 " << tmp_str << "，耗时 "
           << ((double)end_time - start_time) / CLOCKS_PER_SEC << "s\n";
      tmp_str = "";
      for (auto &word : dict) {
        tmp_str += word;
        tmp_str.push_back(',');
      }
      if (!tmp_str.empty()) {
        tmp_str.pop_back();
      } else {
        tmp_str = " ";
      }

      // 向客户端发送数据
      memset(buffer, 0, sizeof(buffer));
      strncpy(buffer, tmp_str.c_str(), tmp_str.length() + 1);
      ret = send(clientfd, buffer, strlen(buffer), 0);
      if (ret < 0) {
        cout << "send error\n";
        break;
      } else {
        cout << "发送成功，向客户端 " << inet_ntoa(client_addr->sin_addr)
             << " 发送内容：" << buffer << "\n";
      }
    }

    // 关闭连接
    close(clientfd);
    cout << "客户端 " << inet_ntoa(client_addr->sin_addr) << " 已断开连接\n";
  }

private:
  int clientfd;
  struct sockaddr_in *client_addr;
};

/**
 * @brief
 *
 */
class ThreadPool final {
public:
  ThreadPool() : m_bRunning(false) {}

  ~ThreadPool() { removeAllTask(); }

  ThreadPool(const ThreadPool &taskpool) = delete;

  ThreadPool operator=(const ThreadPool &taskpool) = delete;

  void init(int threadNum) {
    if (threadNum <= 0) {
      threadNum = 5;
    }
    m_bRunning = true;
    for (int i = 0; i < threadNum; i++) {
      shared_ptr<thread> spThread;
      spThread.reset(new thread(bind(&ThreadPool::doWork, this)));
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
  void doWork(Trie &trie, const int prompt_num) {
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
      spTask->doIt(trie, prompt_num);
      spTask.reset();
    }
    {
      unique_lock<mutex> guard(m_mutex);
      cout << "thread " << this_thread::get_id() << " exit\n";
    }
  }

  list<shared_ptr<Task>> m_taskList;
  mutex m_mutex;
  condition_variable m_cond_var;
  bool m_bRunning;
  vector<shared_ptr<thread>> m_threads;
};

int main() {
  ThreadPool threadPool;
  threadPool.init(5);
  Task *task = nullptr;
  for (int i = 0; i < 10; i++) {
    task = new Task(1, nullptr);
    threadPool.addTask(task);
  }
  this_thread::sleep_for(chrono::seconds(5));
  threadPool.stop();
  return 0;
}