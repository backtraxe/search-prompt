#include <fstream>
#include <iostream>
#include <string>

#include "Utility.hpp"

/**
 * @brief 导入 config.conf 配置文件
 *
 * @return std::unordered_map<std::string, std::string>
 */
const std::unordered_map<std::string, std::string> Utility::loadConfig() {
    std::unordered_map<std::string, std::string> params;
    std::ifstream ifs;
    ifs.open("config.conf");
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

void Utility::loadData(const char *filePath,
                       std::vector<std::pair<std::string, double>> &dict) {
    std::ifstream ifs(filePath);
    std::string line;
    int count = 0;
    auto start_t = clock();
    while (std::getline(ifs, line)) {
        // 左边界，右边界
        int idx_left, idx_right;
        double weight = 0.0;
        idx_left = line.find("\"0\":");
        if (idx_left != -1) {
            idx_right = line.find(",", idx_left);
            weight = stod(line.substr(idx_left + 5, idx_right - idx_left - 5));
        }

        idx_left = line.find("\"_k\":");
        idx_right = line.find("\"", idx_left + 7);

        dict.emplace_back(line.substr(idx_left + 7, idx_right - idx_left - 7),
                          weight);
        count++;

        if (count == 1000000)
            break;
    }
    auto end_t = clock();
    std::cout << "读取 " << count << " 条数据耗时："
              << ((double)end_t - start_t) / CLOCKS_PER_SEC << "s\n";
}