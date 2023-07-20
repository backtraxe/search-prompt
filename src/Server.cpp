#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <functional>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "ThreadPool.hpp"
#include "TrieBase.hpp"
#include "Utility.hpp"

/**
 * @brief 线程通信的控制器。
 *
 */
struct MidNode {
    MidNode() : stopFlag(false), exitFlag(false) {}

    /**
     * @brief 互斥量。
     *
     */
    std::mutex mtx;

    /**
     * @brief 通知进程的条件变量。
     *
     */
    std::condition_variable startClock;

    /**
     * @brief 控制函数提前结束的标志。
     *
     */
    std::atomic<bool> stopFlag;

    /**
     * @brief 控制线程退出的标志。
     *
     */
    std::atomic<bool> exitFlag;
};

class Server final {
  private:
    int listenFd;
    ThreadPool threadPool;
    Trie trie;
    const int promptNum;
    const int millisecondLimit;

  public:
    Server(const char *ip, int port, int threadNum, int shardNum, int promptNum,
           int millisecondLimit)
        : threadPool(threadNum), promptNum(promptNum),
          millisecondLimit(millisecondLimit) {
        // 创建socket
        listenFd = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == listenFd) {
            std::cerr << "创建 socket 失败\n";
            exit(-1);
        }

        // 初始化服务器地址
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ip); // ip
        server_addr.sin_port = htons(port);          // port

        // 绑定地址
        int ret = bind(listenFd, (struct sockaddr *)&server_addr,
                       sizeof(server_addr));
        if (-1 == ret) {
            std::cerr << "绑定地址失败\n";
            exit(-1);
        }

        // 监听请求
        ret = listen(listenFd, SOMAXCONN);
        if (-1 == ret) {
            std::cerr << "监听请求失败\n";
            exit(-1);
        }

        // 读取数据
        // std::vector<std::pair<std::string, double>> dict;
        // Utility::loadData("hourly_smartbox_json.2023071200", dict);

        for (int i = 1; i <= 3; i++) {
            auto st_t = clock();
            std::vector<std::pair<std::string, double>> dict;
            // 导入数据
            Utility::loadData2("data" + std::to_string(i), dict);
            // 插入前排序
            sort(dict.begin(), dict.end());
            for (auto &p : dict) {
                // 插入 trie
                trie.insert(p.first, p.second);
            }
            auto ed_t = clock();
            std::cout << "准备第" << i << "份数据耗时："
                      << ((double)ed_t - st_t) / CLOCKS_PER_SEC << "s\n";
        }

        // 数据排序
        // auto start_t = clock();
        // sort(dict.begin(), dict.end());
        // auto end_t = clock();
        // std::cout << "排序数据耗时："
        //           << ((double)end_t - start_t) / CLOCKS_PER_SEC << "s\n";

        // 建立字典树
        // start_t = clock();
        // for (auto &p : dict) {
        //     trie.insert(p.first, p.second);
        // }
        // end_t = clock();
        // std::cout << "建立Trie耗时："
        //           << ((double)end_t - start_t) / CLOCKS_PER_SEC << "s\n";

        std::cout << "服务器运行中\n";
    }

    ~Server() { close(listenFd); }

    /**
     * @brief
     *
     */
    void listenRequest() {
        while (true) {
            // 建立连接
            struct sockaddr_in client_addr;
            socklen_t socklen = sizeof(client_addr);
            int clientfd =
                accept(listenFd, (struct sockaddr *)&client_addr, &socklen);
            if (-1 == clientfd) {
                std::cerr << "连接失败\n";
                continue;
            } else {
                // std::cout << "客户端 " << inet_ntoa(client_addr.sin_addr)
                //           << " 已连接\n";
                threadPool.addTask(communicate, clientfd, client_addr, trie,
                                   promptNum, millisecondLimit);
                // 加入线程池
                // 使用两个线程配合工作，一个完成用户请求，另一个负责记时
                // MidNode *node = new MidNode();
                // threadPool.addTask(communicate, clientfd, client_addr, trie,
                //                    promptNum, node);
                // threadPool.addTask(countDown, millisecondLimit, node);
            }
        }
    }

    /**
     * @brief
     *
     * @param fd
     * @param addr
     * @param trie
     * @param promptNum
     */
    static void communicate(const int fd, const struct sockaddr_in &addr,
                            Trie &trie, const int promptNum,
                            int millisecondLimit) {
        // 数据交互
        char buffer[2048];
        int ret;
        do {
            // while (true) {
            // 从客户端接收数据
            memset(buffer, 0, sizeof(buffer));
            ret = recv(fd, buffer, sizeof(buffer), 0);
            if (ret <= 0) {
                std::cerr << "接收失败\n";
                break;
            } else {
                // std::cout << "接收成功，接收客户端 " <<
                // inet_ntoa(addr.sin_addr)
                //           << " 发送的内容：" << buffer << "\n";
            }

            // 字典树查询候选词
            std::string tmp_str = buffer;

            // node->stopFlag = false;
            // node->exitFlag = false;
            std::deque<pds> dict;
            // 其他线程进行查询，并设置超时时间
            // 通知另一个线程开始计时
            // std::unique_lock<std::mutex> lock(node->mtx);
            // node->startClock.notify_one();
            // lock.unlock();

            // 搜索trie
            // auto st_t = std::chrono::system_clock::now();
            trie.prompt(tmp_str, promptNum, dict, millisecondLimit);
            // auto ed_t = std::chrono::system_clock::now();

            // std::chrono::duration<double, std::milli> inv_t = ed_t - st_t;
            // std::cout << "搜索 " << tmp_str << "，耗时 "
            //           <<
            //           std::chrono::duration_cast<std::chrono::milliseconds>(
            //                  inv_t)
            //                  .count()
            //           << "ms\n";

            tmp_str = "";
            for (auto &p : dict) {
                tmp_str +=
                    "{" + p.second + ", " + std::to_string(-p.first) + "} ";
            }
            // 返回空格代表返回为空，防止客户端recv阻塞
            if (tmp_str.empty()) {
                tmp_str = " ";
            }

            // 向客户端发送数据
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, tmp_str.c_str());
            ret = send(fd, buffer, strlen(buffer), 0);
            if (ret < 0) {
                std::cerr << "发送失败\n";
                break;
            } else {
                // std::cout << "发送成功，向客户端 " <<
                // inet_ntoa(addr.sin_addr)
                //           << " 发送内容：" << buffer << "\n";
            }
            // }
        } while (false);

        // 关闭连接
        close(fd);
        // node->startClock.notify_one();
        // node->exitFlag = true;
        // delete node;

        // std::cout << "客户端 " << inet_ntoa(addr.sin_addr) << "
        // 已断开连接\n";
    }

    static void countDown(const int milliseconds, MidNode *node) {
        node->exitFlag = false;
        while (true) {
            // 互斥锁
            std::unique_lock<std::mutex> lock(node->mtx);
            // 阻塞，等待另一个线程的通知
            node->startClock.wait(lock);
            lock.unlock();
            // 休眠指定时间
            std::this_thread::sleep_for(
                std::chrono::milliseconds(milliseconds));
            // 到时间后更改标志位，让另一个线程结束查找，立即返回
            node->stopFlag = true;
            if (node->exitFlag) {
                break;
            }
        }
    }
};

int main() {
    auto params = Utility::loadConfig();
    Server server(params["ip"].c_str(), stoi(params["port"]),
                  stoi(params["thread_num"]), stoi(params["shard_num"]),
                  stoi(params["prompt_num"]),
                  stoi(params["millisecond_limit"]));
    server.listenRequest();

    return 0;
}