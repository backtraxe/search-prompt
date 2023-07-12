#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>

#include "Trie.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

using namespace std;
using namespace rapidjson;

int main() {
    Trie trie;

    cout << "hello world\n";

    // 读取文件
    auto start_time = clock();

    ifstream ifs("hourly_smartbox_json.2023071200", ios::binary);
    vector<char> buf(ifs.seekg(0, ios::end).tellg());
    ifs.seekg(0, ios::beg).read(&buf[0], static_cast<streamsize>(buf.size()));
    ifs.close();

    auto end_time = clock();

    cout << "读取耗时：" << ((double)end_time - start_time) / CLOCKS_PER_SEC
         << "s\n";

    string s = "{\"a\"=1}";
    Document doc;
    doc.Parse(s.c_str());

    // 插入Trie
    start_time = clock();

    int preIdx = 0;
    for (int i = 0; i < buf.size() + 1; i++) {
        // if (i == buf.size() || buf[i] == '\n') {
        //     string line(buf.begin() + preIdx, buf.begin() + i);
        //     cout << line << "\n";
        //     trie.insert(line);
        // }
        // if (i == 500) break;
    }

    end_time = clock();

    cout << "插入耗时：" << ((double)end_time - start_time) / CLOCKS_PER_SEC
         << "s\n";

    return 0;
}