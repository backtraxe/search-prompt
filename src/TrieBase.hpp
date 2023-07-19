#pragma once

#include <deque>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

using pds = std::pair<double, std::string>;

class Trie final {
  private:
    struct TrieNode {
        double weight;
        bool is_end;
        std::unordered_map<char, TrieNode *> next;
        TrieNode(double w = 0.0) : weight(w), is_end(false) {}
    };

    TrieNode *root;

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
                  std::priority_queue<pds> &min_heap, const int prompt_num,
                  std::atomic<bool> &flag);

    /**
     * @brief 查找字符串最后字符所在的结点。
     *
     * @param word 字符串。
     * @return TrieNode* 字典树结点。
     */
    Trie::TrieNode *endingNode(const std::string &word) const;

  public:
    Trie();

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
     * @brief 根据前缀词给出提示候选词。
     *
     * @param word 前缀单词。
     * @param prompt_num 提示词数量。
     * @return deque<pair<double, string>> 候选词数组，根据权重降序排列。
     */
    void prompt(const std::string &word, const int prompt_num,
                std::deque<pds> &dict, std::atomic<bool> &flag);
};