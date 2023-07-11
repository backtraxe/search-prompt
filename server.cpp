#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <fstream>
#include <deque>
#include <mutex>
#include <condition_variable>

#include "Trie.h"

using std::ifstream;
using std::ios;
using std::cout;
using std::thread;
using std::deque;
using std::mutex;
using std::lock_guard;
using std::unique_lock;
using std::condition_variable;

// socket的详细信息
struct SocketData {
    SocketData(int fd, sockaddr_in addr) : fd(fd), addr(addr) {}

    int fd;
    struct sockaddr_in addr;
};

// 从 config.conf 中导入配置
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

// 从 dict.txt 中导入词语
void load_dict(Trie &trie) {
    ifstream ifs;
    ifs.open("dict.txt", ios::in);
    if (!ifs.is_open()) {
        cout << "load dict error\n";
        return;
    }
    string line;
    while (getline(ifs, line)) {
        trie.insert(line);
    }
    ifs.close();
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
//             vector<string> dict = trie->prompt(tmp_str);

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
//         cout << "客户端 " << inet_ntoa(client_addr.sin_addr) << " 已断开连接\n";
//     }
// }

void worker(int clientfd, struct sockaddr_in *client_addr, Trie *trie) {
    // 数据交互
    char buffer[1024];
    int ret;
    while (true) {
        // 接收
        memset(buffer, 0, sizeof(buffer));
        ret = recv(clientfd, buffer, sizeof(buffer), 0);
        if (ret <= 0) {
            cout << "recv error\n";
            break;
        } else {
            cout << "接收成功，接收客户端 " << inet_ntoa(client_addr->sin_addr) << " 发送的内容：" << buffer << "\n";
        }

        // 字典树查询候选词
        string tmp_str = buffer;
        vector<string> dict = trie->prompt(tmp_str);

        // 发送
        memset(buffer, 0, sizeof(buffer));
        tmp_str = "";
        for (auto word : dict) {
            tmp_str += word;
            tmp_str.push_back(',');
        }
        if (!tmp_str.empty()) {
            tmp_str.pop_back();
        } else {
            tmp_str = " ";
        }
        strncpy(buffer, tmp_str.c_str(), tmp_str.length() + 1);
        ret = send(clientfd, buffer, strlen(buffer), 0);
        if (ret < 0) {
            cout << "send error\n";
            break;
        } else {
            cout << "发送成功，向客户端 " << inet_ntoa(client_addr->sin_addr) << " 发送内容：" << buffer << "\n";
        }
    }

    // 关闭连接
    close(clientfd);
    cout << "客户端 " << inet_ntoa(client_addr->sin_addr) << " 已断开连接\n";
}

int main() {

    cout << "服务器运行中\n";

    // 读取配置文件
    auto params = load_config();

    // 创建socket
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == serverfd) {
        cout << "socket error\n";
        return -1;
    }

    // 指定服务端ip和端口
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(params["ip"].c_str()); // ip
    server_addr.sin_port = htons(stoi(params["port"])); // port

    // 绑定地址
    int ret = bind(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (0 != ret) {
        cout << "bind error\n";
        return -1;
    }

    // 监听请求
    ret = listen(serverfd, 5);
    if(0 != ret) {
        cout << "listen error\n";
        close(serverfd);
        return -1;
    }

    // 字典树初始化
    Trie trie;
    load_dict(trie);

    // 连接池初始化
    // int MAX_THREAD = stoi(params["max_thread"]);
    // thread threads[MAX_THREAD]; // 线程池
    // mutex mtx; // 互斥信号量
    // condition_variable cond_var; // 条件变量
    // deque<SocketData> wait_queue; // 等待队列
    // for (int i = 0; i < MAX_THREAD; i++) {
    //     threads[i] = thread(worker, &mtx, &cond_var, &wait_queue, &trie);
    // }

    while (true) {
        // 建立连接
        int socklen = sizeof(struct sockaddr_in);
        struct sockaddr_in client_addr;
        int clientfd = accept(serverfd, (struct sockaddr*)&client_addr, (socklen_t *)&socklen);
        if (-1 == clientfd) {
            cout << "accept error\n";
            continue;
        } else {
            cout << "客户端 " << inet_ntoa(client_addr.sin_addr) << " 已连接\n";
            // 加锁，然后添加到任务队列
            // lock_guard<mutex> lock(mtx);
            // SocketData socket(clientfd, client_addr);
            // wait_queue.push_back(socket);
            // cond_var.notify_one();
        }

        thread t(worker, clientfd, &client_addr, &trie);
        t.detach();
    }

    // 关闭连接
    close(serverfd);

    return 0;
}