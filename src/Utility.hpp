#pragma once

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief 工具类
 *
 */
class Utility final {
  public:
    /**
     * @brief 导入配置文件
     *
     * @return std::unordered_map<std::string, std::string>
     */
    static const std::unordered_map<std::string, std::string> loadConfig();

    /**
     * @brief 读取数据
     *
     * @param filePath
     * @param dict
     */
    static void loadData(const char *filePath,
                         std::vector<std::pair<std::string, double>> &dict);
};