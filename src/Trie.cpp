#include <algorithm>

#include "Trie.hpp"

Trie::Trie(const size_t shardNum) { root = new TrieNode('0', 0, shardNum); }

Trie::~Trie() { deleteTrie(root); }

/**
 * @brief 添加字符串。
 *
 * @param word 待添加字符串。
 * @param weight 字符串对应的权重。
 * @return true 添加成功。
 * @return false 添加失败。
 */
bool Trie::insert(const std::string &word, const double weight) const {
    auto cur = root;
    for (char c : word) {
        auto ntx = cur->next.get(c);
        if (!ntx) {
            ntx = new TrieNode(c, 0.0);
            cur->next.put(c, ntx);
        }
        cur = ntx;
    }
    if (cur->is_end) {
        return false;
    }
    cur->is_end = true;
    cur->weight = weight;
    return true;
}

bool Trie::remove(const std::string &word) const {
    auto cur = endingNode(word);
    if (cur) {
        cur->is_end = false;
        return true;
    }
    return false;
}

/**
 * @brief 查询是否存在给定字符串。
 *
 * @param word 待查询字符串。
 * @return true
 * @return false
 */
bool Trie::has(const std::string &word) const {
    auto cur = endingNode(word);
    return cur && cur->is_end;
}

/**
 * @brief 根据前缀词给出提示候选词。
 *
 * @param word 前缀单词。
 * @param prompt_num 提示词数量。
 * @return deque<pair<double, string>> 候选词数组，根据权重降序排列。
 */
std::deque<pds> Trie::prompt(const std::string &word,
                             const int prompt_num) const {
    std::deque<pds> dict;
    auto cur = endingNode(word);
    if (!cur) {
        // 不存在该前缀，返回空数组
        return dict;
    }
    // 权重变为负数，将大顶堆转为小顶堆
    std::priority_queue<pds> min_heap;
    if (cur->is_end) {
        // 添加当前字符串
        min_heap.emplace(-cur->weight, word);
    }
    // 遍历 trie
    std::string prefix = word;
    traverse(cur, prefix, min_heap, prompt_num);
    // 按权重降序排列
    while (!min_heap.empty()) {
        dict.emplace_front(min_heap.top());
        min_heap.pop();
    }
    return dict;
}

/**
 * @brief
 * 在字典树中遍历以当前结点为根结点的整棵树，将树中存在的单词加入到提示词数组中。
 *
 * @param cur 当前结点。
 * @param word 到当前结点时的字符串。
 * @param min_heap 按权重排序的最小堆，保存提示词。
 * @param prompt_num 提示词数量。
 */
void Trie::traverse(const TrieNode *cur, std::string &word,
                    std::priority_queue<pds> &min_heap,
                    const int prompt_num) const {
    // 遍历数组
    for (auto &x : cur->next.getData()) {
        // 遍历哈希表
        for (auto &y : x.getData()) {
            word.push_back(y.first);
            if (y.second->is_end) {
                // 负权重（最小堆），字符串
                min_heap.emplace(-y.second->weight, word);
                // 保持数量，弹出权重最小的字符串
                if ((int)min_heap.size() > prompt_num) {
                    min_heap.pop();
                }
            }
            traverse(y.second, word, min_heap, prompt_num);
            word.pop_back();
        }
    }
}

/**
 * @brief 查找字符串最后字符所在的结点。
 *
 * @param word 字符串。
 * @return TrieNode* 字典树结点。
 */
Trie::TrieNode *Trie::endingNode(const std::string &word) const {
    auto cur = root;
    for (auto &c : word) {
        auto ntx = cur->next.get(c);
        if (!ntx) {
            return nullptr;
        }
        cur = ntx;
    }
    return cur;
}

/**
 * @brief 删除以当前结点为根结点的整棵字典树。
 *
 * @param cur 当前结点。
 */
void Trie::deleteTrie(TrieNode *cur) {
    if (cur) {
        // 遍历数组
        for (auto &x : cur->next.getData()) {
            // 遍历哈希表
            for (auto &y : x.getData()) {
                deleteTrie(y.second);
            }
        }
        delete cur;
    }
}

void Trie::build(std::vector<std::pair<std::string, double>> &dict) {
    sort(dict.begin(), dict.end());
    for (auto &p : dict) {
        insert(p.first, p.second);
    }
}