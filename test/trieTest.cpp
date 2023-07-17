#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>

#include "Trie.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;
using namespace rapidjson;

/**
 * @brief 读取 json 数据
 *
 * @param filename
 * @return vector<string>
 */
vector<string> read_file(const char *filename) {
    vector<string> file;

    auto start_time = clock();

    ifstream ifs(filename);
    string line;
    int count = 0;
    while (getline(ifs, line)) {
        file.push_back(line);
        count++;
    }
    ifs.close();

    auto end_time = clock();

    cout << "读取 " << count << " 条数据，共耗时 "
         << ((double)end_time - start_time) / CLOCKS_PER_SEC << "s\n";

    return file;
}

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

void sortString(vector<string> &file) {
    auto start_time = clock();
    sort(file.begin(), file.end());
    auto end_time = clock();
    cout << "排序耗时 " << ((double)end_time - start_time) / CLOCKS_PER_SEC
         << "s\n";
}

int main() {
    auto file = read_file("hourly_smartbox_json.2023071200");
    sortString(file);
    Trie trie(128);
    init_trie(trie, file);

    auto s = "周杰伦";
    auto start_time = clock();
    auto vec = trie.prompt(s, 10);
    auto end_time = clock();
    cout << "搜索 " << s << "，耗时 "
         << ((double)end_time - start_time) / CLOCKS_PER_SEC << "s\n";
    for (const auto &p : vec) {
        printf("%s %f\n", p.second.c_str(), p.first);
    }

    s = "a";
    start_time = clock();
    vec = trie.prompt(s, 10);
    end_time = clock();
    cout << "搜索 " << s << "，耗时 "
         << ((double)end_time - start_time) / CLOCKS_PER_SEC << "s\n";
    for (const auto &p : vec) {
        printf("%s %f\n", p.second.c_str(), p.first);
    }

    // 读取 26065370 条数据，共耗时 150.158s
    // 排序耗时 73.1876s
    // 插入 26065370 条数据，共耗时 307.8s
    // 搜索 周杰伦，耗时 0.008142s
    // 搜索 a，耗时 0.034122s

    return 0;
}