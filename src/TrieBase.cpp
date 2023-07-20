#include "TrieBase.hpp"

#include <chrono>
#include <iostream>

Trie::Trie() { root = new TrieNode(); }

Trie::~Trie() {}

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
        if (cur->next.count(c) == 0) {
            cur->next[c] = new TrieNode(0.0);
        }
        cur = cur->next[c];
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
 * @brief 根据前缀词给出提示候选词。
 *
 * @param word 前缀单词。
 * @param prompt_num 提示词数量。
 * @return std::deque<pds> 候选词数组，根据权重降序排列。
 */
void Trie::prompt(const std::string &word, const int prompt_num,
                  std::deque<pds> &dict, std::atomic<bool> &flag) {
    auto st_t = clock();
    auto cur = endingNode(word);
    if (!cur) {
        // 不存在该前缀，返回空数组
        return;
    }
    // 权重变为负数，将大顶堆转为小顶堆
    std::priority_queue<pds> min_heap;
    if (cur->is_end) {
        // 添加当前字符串
        min_heap.emplace(-cur->weight, word);
    }
    // 遍历 trie
    std::string prefix = word;
    traverse(cur, prefix, min_heap, prompt_num, flag);
    // 按权重降序排列
    while (!min_heap.empty()) {
        dict.emplace_front(min_heap.top());
        min_heap.pop();
    }
    auto ed_t = clock();
    std::cout << ((double)ed_t - st_t) / CLOCKS_PER_SEC * 1000 << "ms\n";
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
                    std::priority_queue<pds> &min_heap, const int prompt_num,
                    std::atomic<bool> &flag) {
    if (flag) {
        return;
    }
    // 遍历
    for (auto &p : cur->next) {
        if (flag) {
            return;
        }
        word.push_back(p.first);
        if (p.second->is_end) {
            // 负权重（最小堆）、字符串
            min_heap.emplace(-p.second->weight, word);
            if ((int)min_heap.size() > prompt_num) {
                min_heap.pop();
            }
        }
        traverse(p.second, word, min_heap, prompt_num, flag);
        word.pop_back();
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
        if (cur->next.count(c) == 0) {
            cur->next[c] = new TrieNode(0.0);
        }
        cur = cur->next[c];
    }
    return cur;
}

void Trie::prompt(const std::string &word, const int prompt_num,
                  std::deque<pds> &dict, const int millisecondLimit) {
    // 记录开始时间
    auto st_t = std::chrono::steady_clock::now();

    // 查找前缀
    auto cur = endingNode(word);
    if (!cur) {
        // 不存在该前缀，返回空数组
        return;
    }

    // 保存权重更大的字符串
    std::priority_queue<pds> min_heap;
    if (cur->is_end) {
        // 权重变为负数，将大顶堆转为小顶堆
        min_heap.emplace(-cur->weight, word);
    }

    // 遍历 trie
    std::string prefix = word;
    bool exitFlag = false;
    traverse(cur, prefix, min_heap, prompt_num, st_t, 0ULL, millisecondLimit,
             exitFlag);
    // 按权重降序排列
    while (!min_heap.empty()) {
        dict.emplace_front(min_heap.top());
        min_heap.pop();
    }
    // auto ed_t = std::chrono::steady_clock::now();
    // std::cout << (ed_t - st_t).count() / 1000000.0 << "ms\n";
}

void Trie::traverse(const TrieNode *cur, std::string &word,
                    std::priority_queue<pds> &min_heap, const int prompt_num,
                    const std::chrono::steady_clock::time_point &st_t,
                    unsigned long long counter, const int millisecondLimit,
                    bool &exitFlag) {
    if (exitFlag) {
        return;
    }
    // 遍历
    for (auto &p : cur->next) {
        word.push_back(p.first);
        counter++;
        if (counter % 200ULL == 0) {
            // 判断当前时间
            auto ed_t = std::chrono::steady_clock::now();
            if ((ed_t - st_t).count() / 1000000.0 >= millisecondLimit) {
                exitFlag = true;
                return;
            }
        }
        if (p.second->is_end) {
            // 负权重（最小堆）、字符串
            min_heap.emplace(-p.second->weight, word);
            if ((int)min_heap.size() > prompt_num) {
                min_heap.pop();
            }
        }
        traverse(p.second, word, min_heap, prompt_num, st_t, counter,
                 millisecondLimit, exitFlag);
        if (exitFlag) {
            return;
        }
        word.pop_back();
    }
}