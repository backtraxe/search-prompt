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

void Trie::traverse(const TrieNode *cur, string &word, vector<string> &dict) {
    for (auto &p : cur->next) {
        word.push_back(p.first);
        if (p.second->is_end) {
            dict.push_back(word);
        }
        traverse(p.second, word, dict);
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

vector<string> Trie::prompt(const string &word) {
    vector<string> dict;
    auto cur = root;
    string prefix;
    for (auto c : word) {
        if (cur->next.count(c) == 0) {
            return dict;
        }
        cur = cur->next[c];
        prefix.push_back(c);
    }
    if (cur->is_end) {
        dict.push_back(prefix);
    }
    traverse(cur, prefix, dict);
    return dict;
}

void Trie::deleteTrie(TrieNode *cur) {
    if (cur) {
        for (auto &p : cur->next) {
            deleteTrie(p.second);
        }
        delete cur;
    }
}