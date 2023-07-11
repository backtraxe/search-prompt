#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <string>

int main() {

    // 创建socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        printf("socket error\n");
        return -1;
    }

    // 指定服务端ip和端口
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(""); // ip
    server_addr.sin_port = htons(6789); // port

    // 连接服务端
    int ret = connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (0 != ret) {
        printf("connect error\n");
        close(clientfd);
        return -1;
    }

    // 数据交互
    char buffer[1024];
    while (true) {
        printf("继续输入1，退出输出2，请输入：");
        int choice;
        scanf("%d", &choice);
        if (1 == choice) {
            // 发送
            memset(buffer, 0, sizeof(buffer));
            std::cin.getline(buffer, 1024);
            ret = send(clientfd, buffer, strlen(buffer), 0);
            if (ret <= 0) {
                printf("send error\n");
            } else {
                printf("发送成功，发送内容：%s\n", buffer);
            }

            // 接收
            memset(buffer, 0, sizeof(buffer));
            ret = recv(clientfd, buffer, sizeof(buffer), 0);
            if (ret <= 0) {
                printf("");
            }

        } else if (2 == choice) {
            break;
        }
    }

    // 关闭连接
    close(clientfd);

    return 0;
}