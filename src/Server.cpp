#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <functional>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "ThreadPool.hpp"
#include "TrieBase.hpp"
#include "Utility.hpp"

class Server final {
  private:
    int listenFd;
    ThreadPool threadPool;
    Trie trie;
    const int promptNum;

  public:
    Server(const char *ip, int port, int threadNum, int shardNum, int promptNum)
        : threadPool(threadNum), promptNum(promptNum) {
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
        clock_t start_t, end_t;
        {
            std::vector<std::pair<std::string, double>> dict;
            {
                // 设置语句块删除临时对象
                auto file =
                    Utility::readFile("hourly_smartbox_json.2023071200");
                dict = Utility::json2vec(file);
            }
            // 建立字典树
            start_t = clock();
            for (auto &p : dict) {
                trie.insert(p.first, p.second);
            }
            end_t = clock();
        }
        std::cout << "建立Trie耗时："
                  << ((double)end_t - start_t) / CLOCKS_PER_SEC << "s\n";

        std::cout << "服务器运行中\n";
    }

    ~Server() { close(listenFd); }

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
                std::cout << "客户端 " << inet_ntoa(client_addr.sin_addr)
                          << " 已连接\n";
                // 加入线程池
                threadPool.addTask(communicate, clientfd, client_addr, trie,
                                   promptNum);
            }
        }
    }

    static void communicate(const int fd, const struct sockaddr_in &addr,
                            Trie &trie, const int promptNum) {
        // 数据交互
        char buffer[1024];
        int ret;
        while (true) {
            // 从客户端接收数据
            memset(buffer, 0, sizeof(buffer));
            ret = recv(fd, buffer, sizeof(buffer), 0);
            if (ret <= 0) {
                std::cout << "recv error\n";
                break;
            } else {
                std::cout << "接收成功，接收客户端 " << inet_ntoa(addr.sin_addr)
                          << " 发送的内容：" << buffer << "\n";
            }

            // 字典树查询候选词
            std::string tmp_str = buffer;
            auto start_time = clock();
            auto dict = trie.prompt(tmp_str, promptNum);
            auto end_time = clock();
            std::cout << "搜索 " << tmp_str << "，耗时 "
                      << ((double)end_time - start_time) / CLOCKS_PER_SEC
                      << "s\n";
            tmp_str = "";
            for (auto &p : dict) {
                tmp_str +=
                    "{" + p.second + ", " + std::to_string(p.first) + "} ";
            }
            if (tmp_str.empty()) {
                tmp_str = " ";
            }

            // 向客户端发送数据
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, tmp_str.c_str(), tmp_str.length() + 1);
            ret = send(fd, buffer, strlen(buffer), 0);
            if (ret < 0) {
                std::cout << "send error\n";
                break;
            } else {
                std::cout << "发送成功，向客户端 " << inet_ntoa(addr.sin_addr)
                          << " 发送内容：" << buffer << "\n";
            }
        }

        // 关闭连接
        close(fd);
        std::cout << "客户端 " << inet_ntoa(addr.sin_addr) << " 已断开连接\n";
    }
};

int main() {
    auto params = Utility::loadConfig();
    Server server(params["ip"].c_str(), stoi(params["port"]),
                  stoi(params["thread_num"]), stoi(params["shard_num"]),
                  stoi(params["prompt_num"]));
    server.listenRequest();

    return 0;
}