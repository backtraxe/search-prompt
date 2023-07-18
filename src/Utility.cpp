#include <fstream>
#include <iostream>
#include <string>

#include "Utility.hpp"
#include "rapidjson/document.h"

/**
 * @brief 按行读取文件
 *
 * @param filepath 文件路径
 * @return std::vector<std::string>
 */
const std::vector<std::string> Utility::readFile(const char *filePath) {
    std::vector<std::string> file;
    int count = 0;
    auto start_time = clock();
    std::ifstream ifs(filePath);
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
 * @brief 导入 config.conf 配置文件
 *
 * @return std::unordered_map<std::string, std::string>
 */
const std::unordered_map<std::string, std::string> Utility::loadConfig() {
    std::unordered_map<std::string, std::string> params;
    std::ifstream ifs;
    ifs.open("../config.conf");
    if (!ifs.is_open()) {
        std::cerr << "导入配置文件失败\n";
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

/**
 * @brief 从 json 文件中提取字符串和权重
 *
 * @param json
 * @return const std::vector<std::string, double> 字符串和权重
 */
const std::vector<std::pair<std::string, double>>
Utility::json2vec(const std::vector<std::string> &json) {
    std::vector<std::pair<std::string, double>> dict;
    // json 解析器
    rapidjson::Document doc;
    for (auto &line : json) {
        doc.Parse(line.c_str());
        if (doc.HasMember("_k") && doc["_k"].IsString() &&
            doc.HasMember("_s") && doc["_s"].IsObject()) {
            auto &obj = doc["_s"];
            if (obj.HasMember("0") && obj["0"].IsDouble()) {
                dict.emplace_back(doc["_k"].GetString(), obj["0"].GetDouble());
            }
        }
    }
    return dict;
}