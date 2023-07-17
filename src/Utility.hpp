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
     * @brief 按行读取文件
     *
     * @param filepath 文件路径
     * @return std::vector<std::string>
     */
    static std::vector<std::string> readFile(const char *filepath);

    /**
     * @brief 导入配置文件
     *
     * @return std::unordered_map<std::string, std::string>
     */
    static std::unordered_map<std::string, std::string> loadConfig();
};