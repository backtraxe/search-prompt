#include "Trie.hpp"

Trie::Trie() { root = new TrieNode('0', 0.0); }

Trie::~Trie() { deleteTrie(root); }

/**
 * @brief 添加字符串。
 *
 * @param word 待添加字符串。
 * @param weight 字符串对应的权重。
 * @return true 添加成功。
 * @return false 添加失败。
 */
bool Trie::insert(const string &word, const double weight) const {
    auto cur = root;
    for (char c : word) {
        if (cur->next.count(c) == 0) {
            cur->next[c] = new TrieNode(c, 0.0);
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

void Trie::traverse(const TrieNode *cur, string &word,
                    priority_queue<pair<double, string>> &min_heap,
                    const int prompt_num) {
    for (auto &p : cur->next) {
        word.push_back(p.first);
        if (p.second->is_end) {
            // 负权重（最小堆）、字符串
            min_heap.emplace(-p.second->weight, word);
            if ((int)min_heap.size() > prompt_num) {
                min_heap.pop();
            }
        }
        traverse(p.second, word, min_heap, prompt_num);
        word.pop_back();
    }
}

bool Trie::remove(const string &word) const {
    auto cur = root;
    for (auto c : word) {
        if (!cur->next[c]) {
            return false;
        }
        cur = cur->next[c];
    }
    cur->is_end = false;
    return true;
}

bool Trie::has(const string &word) const {
    auto cur = root;
    for (auto c : word) {
        if (!cur->next[c]) {
            return false;
        }
        cur = cur->next[c];
    }
    return cur->is_end;
}

/**
 * @brief
 *
 * @param word
 * @param prompt_num
 * @return deque<string>
 */
deque<string> Trie::prompt(const string &word, const int prompt_num) {
    // 权重变为负数，将大顶堆转为小顶堆
    priority_queue<pair<double, string>> min_heap;
    deque<string> ret;
    auto cur = root;
    string prefix;
    for (auto c : word) {
        // 不是前缀，返回空
        if (cur->next.count(c) == 0) {
            return ret;
        }
        cur = cur->next[c];
        prefix.push_back(c);
    }
    if (cur->is_end) {
        min_heap.emplace(-cur->weight, word);
    }
    traverse(cur, prefix, min_heap, prompt_num);
    // 按权重降序排列
    while (!min_heap.empty()) {
        ret.emplace_front(min_heap.top().second);
        min_heap.pop();
    }
    return ret;
}

/**
 * @brief
 *
 * @param cur
 */
void Trie::deleteTrie(TrieNode *cur) {
    if (cur) {
        for (auto &p : cur->next) {
            deleteTrie(p.second);
        }
        delete cur;
    }
}