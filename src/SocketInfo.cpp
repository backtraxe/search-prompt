#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>

#include "SocketInfo.hpp"

SocketInfo::SocketInfo(int fd, struct sockaddr_in *addr, Trie *trie,
                       const int prompt_num)
    : fd(fd), addr(addr), trie(trie), prompt_num(prompt_num) {}

void SocketInfo::communicate() {
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
            std::cout << "接收成功，接收客户端 " << inet_ntoa(addr->sin_addr)
                      << " 发送的内容：" << buffer << "\n";
        }

        // 字典树查询候选词
        string tmp_str = buffer;
        auto start_time = clock();
        auto dict = trie->prompt(tmp_str, prompt_num);
        auto end_time = clock();
        std::cout << "搜索 " << tmp_str << "，耗时 "
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
        ret = send(fd, buffer, strlen(buffer), 0);
        if (ret < 0) {
            std::cout << "send error\n";
            break;
        } else {
            std::cout << "发送成功，向客户端 " << inet_ntoa(addr->sin_addr)
                      << " 发送内容：" << buffer << "\n";
        }
    }

    // 关闭连接
    close(fd);
    std::cout << "客户端 " << inet_ntoa(addr->sin_addr) << " 已断开连接\n";
}