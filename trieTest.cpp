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
 * @return vector<char> 字符数组
 */
vector<char> read_file(const char *filename) {
    auto start_time = clock();

    // 快读
    ifstream ifs(filename, ios::binary);
    vector<char> buf(ifs.seekg(0, ios::end).tellg());
    ifs.seekg(0, ios::beg).read(&buf[0], static_cast<streamsize>(buf.size()));
    ifs.close();

    auto end_time = clock();

    cout << "读取文件耗时：" << ((double)end_time - start_time) / CLOCKS_PER_SEC
         << "s\n";

    return buf;
}

/**
 * @brief 通过字符数组中的 json 数据建立 trie
 *
 * @param trie 字典树
 * @param buf 字符数组 json 数组
 */
void init_trie(Trie &trie, vector<char> &buf) {
    auto start_time = clock();

    // json 解析器
    Document doc;
    int preIdx = 0;
    int sz = buf.size();
    int count = 0;
    for (int i = 0; i < sz + 1; i++) {
        // 按行拆分，每次解析一个 json
        if (i == sz || buf[i] == '\n') {
            string line(buf.begin() + preIdx, buf.begin() + i);
            doc.Parse(line.c_str());
            if (doc.HasMember("_k") && doc["_k"].IsString() &&
                doc.HasMember("_s") && doc["_s"].IsObject()) {
                auto &obj = doc["_s"];
                if (obj.HasMember("0") && obj["0"].IsDouble()) {
                    trie.insert(doc["_k"].GetString(), obj["0"].GetDouble());
                }
            }
            preIdx = i + 1;
            count++;
        }
    }

    auto end_time = clock();

    cout << "插入" << count << "条数据，插入耗时："
         << ((double)end_time - start_time) / CLOCKS_PER_SEC << "s\n";
}

int main() {
    auto file = read_file("hourly_smartbox_json.2023071200");
    Trie trie;
    init_trie(trie, file);

    auto s = "周杰伦";
    auto start_time = clock();
    auto vec = trie.prompt(s);
    auto end_time = clock();
    cout << "搜索" << s << "耗时："
         << ((double)end_time - start_time) / CLOCKS_PER_SEC << "s\n";
    for (const auto &v : vec) {
        printf("%s\n", v.c_str());
    }

    // 读取文件耗时：32.9934s
    // 插入7826088条数据，插入耗时：119.48s
    // 搜索周杰伦耗时：0.000697s

    return 0;
}