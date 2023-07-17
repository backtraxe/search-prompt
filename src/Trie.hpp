#pragma once

#include <deque>
#include <queue>
#include <string>
#include <utility>

#include "ConcurrentHashMap.hpp"

using pds = std::pair<double, std::string>;

class Trie final {

  private:
    /**
     * @brief 字典树中结点的数据结构。
     *
     */
    struct TrieNode {
        TrieNode(const char v, const double w, const size_t num_shard = 128)
            : val(v), weight(w), is_end(false), next(num_shard) {}

        /**
         * @brief 当前字符。
         *
         */
        char val;

        /**
         * @brief 字符串权重，权重越大优先级越高。
         *
         */
        double weight;

        /**
         * @brief 是否是字符串结尾字符。
         *
         */
        bool is_end;

        /**
         * @brief 下个字符。
         *
         */
        ConcurrentHashMap<char, TrieNode *> next;
    };

    /**
     * @brief
     * 在字典树中遍历以当前结点为根结点的整棵树，将树中存在的单词加入到提示词数组中。
     *
     * @param cur 当前结点。
     * @param word 到当前结点时的字符串。
     * @param min_heap 按权重排序的最小堆，保存提示词。
     * @param prompt_num 提示词数量。
     */
    void traverse(const TrieNode *cur, std::string &word,
                  std::priority_queue<std::pair<double, std::string>> &min_heap,
                  const int prompt_num);

    /**
     * @brief 查找字符串最后字符所在的结点。
     *
     * @param word 字符串。
     * @return TrieNode* 字典树结点。
     */
    TrieNode *endingNode(const std::string &word) const;

    /**
     * @brief 删除以当前结点为根结点的整棵字典树。
     *
     * @param cur 当前结点。
     */
    void deleteTrie(TrieNode *cur);

    /**
     * @brief 字典树根结点
     *
     */
    TrieNode *root;

    /**
     * @brief 分片数量。
     *
     */
    const size_t num_shard;

  public:
    Trie(const size_t num_shard);

    ~Trie();

    /**
     * @brief 添加字符串。
     *
     * @param word 待添加字符串。
     * @param weight 字符串对应的权重。
     * @return true 添加成功。
     * @return false 添加失败。
     */
    bool insert(const std::string &word, const double weight) const;

    /**
     * @brief 删除字符串。
     *
     * @param word 待删除字符串。
     * @return true 删除成功。
     * @return false 删除失败。
     */
    bool remove(const std::string &word) const;

    /**
     * @brief 查询是否存在给定字符串。
     *
     * @param word 待查询字符串。
     * @return true
     * @return false
     */
    bool has(const std::string &word) const;

    /**
     * @brief 根据前缀词给出提示候选词。
     *
     * @param word 前缀单词。
     * @param prompt_num 提示词数量。
     * @return std::deque<std::pair<double, std::string>>
     * 候选词数组，根据权重降序排列。
     */
    std::deque<pds> prompt(const std::string &word, const int prompt_num);
};