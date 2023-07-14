#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Trie.hpp"

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
    static std::vector<std::string> read_file(const char *filepath);

    /**
     * @brief 导入配置
     *
     * @return std::unordered_map<std::string, std::string>
     */
    static std::unordered_map<std::string, std::string> load_config();

    /**
     * @brief 从 dict.txt 中导入词语
     *
     * @param trie
     */
    static void load_dict(Trie &trie);
};