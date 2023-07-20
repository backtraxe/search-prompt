#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/time.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

using namespace std;

int addTwo(int a, int b) { return a + b; }

struct Person {
    string name;
    int age;
};

int main() {

    // ifstream ifs("../hourly_smartbox_json.2023071200");

    // ofstream ofs[6]{ofstream("../data1"), ofstream("../data2"),
    //                 ofstream("../data3"), ofstream("../data4"),
    //                 ofstream("../data5"), ofstream("../data6")};
    // int idx = 0;

    // std::string line;
    // int count = 0;
    // auto start_t = clock();
    // while (std::getline(ifs, line)) {
    //     // 左边界，右边界
    //     int idx_left, idx_right;
    //     double weight = 0.0;
    //     idx_left = line.find("\"0\":");
    //     if (idx_left != -1) {
    //         idx_right = line.find(",", idx_left);
    //         weight = stod(line.substr(idx_left + 5, idx_right - idx_left -
    //         5));
    //     }

    //     idx_left = line.find("\"_k\":");
    //     idx_right = line.find("\"", idx_left + 7);

    //     // dict.emplace_back(line.substr(idx_left + 7, idx_right - idx_left -
    //     // 7),
    //     //                   weight);
    //     count++;

    //     if (count % 5000000 == 0)
    //         idx++;

    //     ofs[idx] << line.substr(idx_left + 7, idx_right - idx_left - 7)
    //              << "####" << weight << "\n";
    // }

    // ifs.close();
    // for (auto &ofs : ofs) {
    //     ofs.close();
    // }

    string s1 = "3.14";
    string s2 = "0";

    stod(s1);
    stod(s2);

    return 0;
}