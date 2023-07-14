#include <fstream>
#include <iostream>
#include <string>

#include "Utility.hpp"

/**
 * @brief 按行读取文件
 *
 * @param filepath 文件路径
 * @return std::vector<std::string>
 */
std::vector<std::string> Utility::read_file(const char *filepath) {
    std::vector<std::string> file;
    int count = 0;
    auto start_time = clock();
    std::ifstream ifs(filepath);
    std::string line;
    while (std::getline(ifs, line)) {
        file.push_back(line);
    }
    auto end_time = clock();
    std::cout << "读取 " << count << " 条数据，共耗时 "
              << ((double)end_time - start_time) / CLOCKS_PER_SEC << "s\n";
    return file;
}

/**
 * @brief 导入配置
 *
 * @return std::unordered_map<std::string, std::string>
 */
std::unordered_map<std::string, std::string> Utility::load_config() {
    std::unordered_map<std::string, std::string> params;
    std::ifstream ifs;
    ifs.open("config.conf");
    if (!ifs.is_open()) {
        std::cout << "load config error\n";
        return params;
    }
    std::string line;
    while (getline(ifs, line)) {
        int idx = line.find('=');
        std::string k = line.substr(0, idx);
        std::string v = line.substr(idx + 1, line.length() - idx - 1);
        params[k] = v;
    }
    ifs.close();
    return params;
}

void load_dict(Trie &trie) {
    ifstream ifs;
    ifs.open("dict.txt", ios::in);
    if (!ifs.is_open()) {
        cout << "load dict error\n";
        return;
    }
    string line;
    while (getline(ifs, line)) {
        int idx = line.rfind(' ');
        string word = line.substr(0, idx);
        double weight = stod(line.substr(idx + 1, line.length() - idx - 1));
        trie.insert(word, weight);
    }
    ifs.close();
}