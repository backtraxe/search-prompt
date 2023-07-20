#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

#include "Client.hpp"
#include "Utility.hpp"

Client::Client(const char *ip, int port) {
    // 创建socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd) {
        std::cerr << "创建 socket 失败\n";
        exit(-1);
    }

    // 指定服务端 ip 和端口
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip); // ip
    server_addr.sin_port = htons(port);          // port

    // 连接服务端
    int ret = connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (-1 == ret) {
        std::cerr << "连接服务器失败\n";
        exit(-1);
    }

    // std::cout << "客户端运行中\n";
}

Client::~Client() {
    // 关闭连接
    close(fd);
}

void Client::communicate() {
    // 数据交互
    int bufSize = 2048;
    char buffer[bufSize];
    int choice, ret;
    while (true) {
        std::cout << "继续输入1，退出输出2，请输入：";
        std::cin >> choice;
        getchar();
        if (1 == choice) {
            // 发送消息
            memset(buffer, 0, sizeof(buffer));
            std::cout << "请输入搜索词：";
            std::cin.getline(buffer, bufSize);
            ret = send(fd, buffer, strlen(buffer), 0);
            if (ret <= 0) {
                std::cerr << "发送失败\n";
                continue;
            } else {
                std::cout << "发送成功，发送内容：" << buffer << "\n";
            }
            // 接收消息
            memset(buffer, 0, sizeof(buffer));
            ret = recv(fd, buffer, sizeof(buffer), 0);
            if (ret < 0) {
                std::cerr << "接收失败\n";
                continue;
            } else {
                std::cout << "接收成功，接收内容：" << buffer << "\n";
            }
        } else if (2 == choice) {
            break;
        }
    }
}

bool Client::doRequest(const std::string &query) {
    char buffer[2048] = {0};
    // 发送消息
    strcpy(buffer, query.c_str());
    int ret = send(fd, buffer, strlen(buffer), 0);
    if (ret < 0) {
        return false;
    }
    memset(buffer, 0, sizeof(buffer));
    ret = recv(fd, buffer, sizeof(buffer), 0);
    if (ret < 0) {
        return false;
    }
    return true;
}

// int main() {
//     // 读取配置文件
//     auto params = Utility::loadConfig();

//     Client client(params["ip"].c_str(), stoi(params["port"]));
//     client.communicate();
//     std::cout << client.doRequest("周杰伦");

//     return 0;
// }