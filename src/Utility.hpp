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
    static const std::vector<std::string> readFile(const char *filepath);

    /**
     * @brief 导入配置文件
     *
     * @return std::unordered_map<std::string, std::string>
     */
    static const std::unordered_map<std::string, std::string> loadConfig();

    /**
     * @brief 从 json 文件中提取字符串和权重
     *
     * @param json
     * @return const std::vector<std::pair<std::string, double>> 字符串和权重
     */
    static const std::vector<std::pair<std::string, double>>
    json2vec(const std::vector<std::string> &json);
};