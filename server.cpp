#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>

#include "Trie.h"

using std::ifstream;
using std::ios;

// 从 dict.txt 中导入词语
void load_dict(Trie &trie) {
    ifstream ifs;
    ifs.open("dict.txt", ios::in);
    if (!ifs.is_open()) {
        printf("open file error");
    }
    string line;
    while (getline(ifs, line)) {
        trie.insert(line);
    }
    // 关闭文件
    ifs.close();
}

int main() {

    // 创建socket
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == serverfd) {
        printf("socket error\n");
        return -1;
    }

    // 指定服务端ip和端口
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("10.91.215.144"); // ip
    server_addr.sin_port = htons(6789); // port

    // 绑定地址
    int ret = bind(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (0 != ret) {
        printf("bind error\n");
        return -1;
    }

    // 监听请求
    ret = listen(serverfd, 5);
    if(0 != ret) {
        printf("listen error\n");
        close(serverfd);
        return -1;
    }

    // 字典树初始化
    Trie trie;
    load_dict(trie);

    // 相互通信
    char buffer[1024];
    while (true) {
        int socklen = sizeof(struct sockaddr_in);
        struct sockaddr_in client_addr;
        int clientfd = accept(serverfd, (struct sockaddr*)&client_addr, (socklen_t *)&socklen);
        if (-1 == clientfd) {
            printf("accept error\n");
        } else {
            printf("客户端 %s 已连接\n", inet_ntoa(client_addr.sin_addr));
        }

        while (true) {
            // 接收
            memset(buffer, 0, sizeof(buffer));
            ret = recv(clientfd, buffer, sizeof(buffer), 0);
            if (ret <= 0) {
                printf("recv error\n");
            } else {
                printf("接收内容：%s\n", buffer);
            }

            // 字典树查询候选词
            string tmp_str = buffer;
            vector<string> dict = trie.prompt(tmp_str);

            // 发送
            memset(buffer, 0, sizeof(buffer));
            tmp_str = "";
            for (auto word : dict) {
                tmp_str.push_back(',');
                tmp_str += word;
            }
            std::cout << tmp_str << '\n';
            strncpy(buffer, tmp_str.c_str(), tmp_str.length() + 1);
            ret = send(clientfd, buffer, strlen(buffer), 0);
            if (ret <= 0) {
                printf("send error\n");
            } else {
                printf("发送成功，发送内容：%s\n", buffer);
            }
        }

        printf("客户端 %s 已断开连接\n", inet_ntoa(client_addr.sin_addr));
        close(clientfd);
    }

    // 关闭连接
    close(serverfd);

    return 0;
}