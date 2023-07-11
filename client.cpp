#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <cstring>
#include <fstream>

using std::ifstream;
using std::ios;
using std::cin;
using std::cout;
using std::string;
using std::unordered_map;

// 从 config.conf 中导入配置
unordered_map<string, string> load_config() {
    unordered_map<string, string> params;
    ifstream ifs;
    ifs.open("config.conf", ios::in);
    if (!ifs.is_open()) {
        printf("load config error\n");
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

int main() {

    cout << "客户端运行中\n";

    // 读取配置文件
    auto params = load_config();

    // 创建socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        cout << "socket error\n";
        return -1;
    }

    // 指定服务端ip和端口
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(params["ip"].c_str()); // ip
    server_addr.sin_port = htons(stoi(params["port"])); // port

    // 连接服务端
    int ret = connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (0 != ret) {
        cout << "connect error\n";
        close(clientfd);
        return -1;
    }

    // 数据交互
    char buffer[1024];
    int choice;
    while (true) {
        cout << "继续输入1，退出输出2，请输入：";
        cin >> choice;
        getchar();
        if (1 == choice) {
            // 发送
            memset(buffer, 0, sizeof(buffer));
            cout << "请输入搜索词：";
            cin.getline(buffer, 1024);
            ret = send(clientfd, buffer, strlen(buffer), 0);
            if (ret <= 0) {
                cout << "send error\n";
                break;
            } else {
                cout << "发送成功，发送内容：" << buffer << "\n";
            }

            // 接收
            memset(buffer, 0, sizeof(buffer));
            ret = recv(clientfd, buffer, sizeof(buffer), 0);
            if (ret < 0) {
                cout << "recv error\n";
                break;
            } else {
                cout << "接收成功，接收内容：" << buffer << "\n";
            }
        } else if (2 == choice) {
            break;
        }
    }

    // 关闭连接
    close(clientfd);

    return 0;
}