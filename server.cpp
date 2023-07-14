#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <functional>

#include "Trie.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using rapidjson::Document;
using std::condition_variable;
using std::cout;
using std::deque;
using std::ifstream;
using std::ios;
using std::lock_guard;
using std::mutex;
using std::shared_ptr;
using std::streamsize;
using std::thread;
using std::unique_lock;
using std::vector;

/**
 * @brief
 *
 */
class Task {
public:
  Task(int fd, struct sockaddr_in *addr, Trie *trie, int prompt_num)
      : clientfd(fd), client_addr(addr), trie(trie), prompt_num(prompt_num) {}

  void doIt() {
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
      auto dict = trie->prompt(tmp_str, prompt_num);
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
  Trie *trie;
  int prompt_num;
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
      spThread.reset(new thread(std::bind(&ThreadPool::doWork, this)));
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
  void doWork() {
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
    {
      unique_lock<mutex> guard(m_mutex);
      cout << "thread " << std::this_thread::get_id() << " exit\n";
    }
  }

  deque<shared_ptr<Task>> m_taskList;
  mutex m_mutex;
  condition_variable m_cond_var;
  bool m_bRunning;
  vector<shared_ptr<thread>> m_threads;
};

/**
 * @brief 从 config.conf 中导入配置
 *
 * @return unordered_map<string, string>
 */
unordered_map<string, string> load_config() {
  unordered_map<string, string> params;
  ifstream ifs;
  ifs.open("config.conf", ios::in);
  if (!ifs.is_open()) {
    cout << "load config error\n";
    return params;
  }
  string line;
  while (getline(ifs, line)) {
    int idx = line.find('=');
    string k = line.substr(0, idx);
    string v = line.substr(idx + 1, line.length() - idx - 1);
    params[k] = v;
  }
  ifs.close();
  return params;
}

// 读取配置文件
auto params = load_config();
// mutex mtx;                   // 互斥信号量
// condition_variable cond_var; // 条件变量

/**
 * @brief 从 dict.txt 中导入词语
 *
 * @param trie
 */
void load_dict(Trie &trie) {
  ifstream ifs;
  ifs.open("dict.txt", ios::in);
  if (!ifs.is_open()) {
    cout << "load dict error\n";
    return;
  }
  string line;
  while (getline(ifs, line)) {
    int idx = line.rfind(' ');
    string word = line.substr(0, idx);
    double weight = stod(line.substr(idx + 1, line.length() - idx - 1));
    trie.insert(word, weight);
  }
  ifs.close();
}

/**
 * @brief 读取 json 数据
 *
 * @param filename
 * @return vector<string>
 */
vector<string> read_file(const char *filename) {
  vector<string> file;

  auto start_time = clock();

  ifstream ifs(filename);
  string line;
  int count = 0;
  while (getline(ifs, line)) {
    file.push_back(line);
    count++;
  }
  ifs.close();

  auto end_time = clock();

  cout << "读取 " << count << " 条数据，共耗时 "
       << ((double)end_time - start_time) / CLOCKS_PER_SEC << "s\n";

  return file;
}

/**
 * @brief 通过字符数组中的 json 数据建立 trie
 *
 * @param trie 字典树
 * @param buf 字符数组 json 数组
 */
void init_trie(Trie &trie, vector<string> &file) {
  auto start_time = clock();

  // json 解析器
  Document doc;
  int count = 0;
  for (auto &line : file) {
    doc.Parse(line.c_str());
    if (doc.HasMember("_k") && doc["_k"].IsString() && doc.HasMember("_s") &&
        doc["_s"].IsObject()) {
      auto &obj = doc["_s"];
      if (obj.HasMember("0") && obj["0"].IsDouble()) {
        trie.insert(doc["_k"].GetString(), obj["0"].GetDouble());
      }
    }
    count++;
  }

  auto end_time = clock();

  cout << "插入 " << count << " 条数据，共耗时 "
       << ((double)end_time - start_time) / CLOCKS_PER_SEC << "s\n";
}

// 每个线程的任务
// void worker(mutex *mtx,
//             condition_variable *cond_var,
//             deque<SocketData> *wait_queue,
//             Trie *trie) {

//     char buffer[1024];
//     int ret;

//     while (true) {
//         unique_lock<mutex> lock(*mtx);
//         while (wait_queue->empty()) {
//             cond_var->wait(lock);
//         }
//         SocketData socket = wait_queue->front();
//         wait_queue->pop_front();

//         int clientfd = socket.fd;
//         struct sockaddr_in client_addr = socket.addr;

//         // 数据交互
//         while (true) {
//             // 接收
//             memset(buffer, 0, sizeof(buffer));
//             ret = recv(clientfd, buffer, sizeof(buffer), 0);
//             if (ret <= 0) {
//                 cout << "recv error\n";
//                 break;
//             } else {
//                 cout << "接收成功，接收内容：" << buffer << "\n";
//             }

//             // 字典树查询候选词
//             string tmp_str = buffer;
//             auto dict = trie->prompt(tmp_str, stoi(params["prompt_num"]));

//             // 发送
//             memset(buffer, 0, sizeof(buffer));
//             tmp_str = "";
//             for (auto word : dict) {
//                 tmp_str += word;
//                 tmp_str.push_back(',');
//             }
//             if (!tmp_str.empty()) {
//                 tmp_str.pop_back();
//             } else {
//                 tmp_str = " ";
//             }
//             strncpy(buffer, tmp_str.c_str(), tmp_str.length() + 1);
//             ret = send(clientfd, buffer, strlen(buffer), 0);
//             if (ret < 0) {
//                 cout << "send error\n";
//                 break;
//             } else {
//                 cout << "发送成功，发送内容：" << buffer << "\n";
//             }
//         }

//         // 关闭连接
//         close(clientfd);
//         cout << "客户端 " << inet_ntoa(client_addr.sin_addr) << "
//         已断开连接\n";
//     }
// }

// void worker(int clientfd, struct sockaddr_in *client_addr, Trie *trie) {
//   // 数据交互
//   char buffer[1024];
//   int ret;
//   while (true) {
//     // 从客户端接收数据
//     memset(buffer, 0, sizeof(buffer));
//     ret = recv(clientfd, buffer, sizeof(buffer), 0);
//     if (ret <= 0) {
//       cout << "recv error\n";
//       break;
//     } else {
//       cout << "接收成功，接收客户端 " << inet_ntoa(client_addr->sin_addr)
//            << " 发送的内容：" << buffer << "\n";
//     }

//     // 字典树查询候选词
//     string tmp_str = buffer;
//     auto start_time = clock();
//     auto dict = trie->prompt(tmp_str, stoi(params["prompt_num"]));
//     auto end_time = clock();
//     cout << "搜索 " << tmp_str << "，耗时 "
//          << ((double)end_time - start_time) / CLOCKS_PER_SEC << "s\n";
//     tmp_str = "";
//     for (auto &word : dict) {
//       tmp_str += word;
//       tmp_str.push_back(',');
//     }
//     if (!tmp_str.empty()) {
//       tmp_str.pop_back();
//     } else {
//       tmp_str = " ";
//     }

//     // 向客户端发送数据
//     memset(buffer, 0, sizeof(buffer));
//     strncpy(buffer, tmp_str.c_str(), tmp_str.length() + 1);
//     ret = send(clientfd, buffer, strlen(buffer), 0);
//     if (ret < 0) {
//       cout << "send error\n";
//       break;
//     } else {
//       cout << "发送成功，向客户端 " << inet_ntoa(client_addr->sin_addr)
//            << " 发送内容：" << buffer << "\n";
//     }
//   }

//   // 关闭连接
//   close(clientfd);
//   cout << "客户端 " << inet_ntoa(client_addr->sin_addr) << " 已断开连接\n";
// }

int main() {
  cout << "服务器运行中\n";

  // 字典树初始化
  Trie trie;
  auto file = read_file("hourly_smartbox_json.2023071200");
  // load_dict(trie);
  init_trie(trie, file);

  // 线程池初始化
  ThreadPool threadPool;
  threadPool.init(stoi(params["thread_num"]));
  Task *task = nullptr;

  // 创建socket
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == listenfd) {
    cout << "create socket error\n";
    return -1;
  }

  // 初始化服务器地址
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(params["ip"].c_str()); // ip
  server_addr.sin_port = htons(stoi(params["port"]));            // port

  // 绑定地址
  int ret =
      bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (-1 == ret) {
    cout << "bind error\n";
    return -1;
  }

  // 监听请求
  ret = listen(listenfd, SOMAXCONN);
  if (-1 == ret) {
    cout << "listen error\n";
    return -1;
  }

  while (true) {
    // 建立连接
    struct sockaddr_in client_addr;
    socklen_t socklen = sizeof(client_addr);
    int clientfd = accept(listenfd, (struct sockaddr *)&client_addr, &socklen);
    if (-1 == clientfd) {
      cout << "accept error\n";
      continue;
    } else {
      cout << "客户端 " << inet_ntoa(client_addr.sin_addr) << " 已连接\n";
      // 加入线程池
      task =
          new Task(clientfd, &client_addr, &trie, stoi(params["prompt_num"]));
      threadPool.addTask(task);
    }

    // thread t(worker, clientfd, &client_addr, &trie);
    // t.detach();
  }

  // 关闭监听
  close(listenfd);
  threadPool.stop();

  return 0;
}