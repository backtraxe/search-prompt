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
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

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

class Server final {};

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
        if (doc.HasMember("_k") && doc["_k"].IsString() &&
            doc.HasMember("_s") && doc["_s"].IsObject()) {
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
        int clientfd =
            accept(listenfd, (struct sockaddr *)&client_addr, &socklen);
        if (-1 == clientfd) {
            cout << "accept error\n";
            continue;
        } else {
            cout << "客户端 " << inet_ntoa(client_addr.sin_addr) << " 已连接\n";
            // 加入线程池
            task = new Task(clientfd, &client_addr, &trie,
                            stoi(params["prompt_num"]));
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