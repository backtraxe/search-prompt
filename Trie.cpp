#include "Trie.h"

Trie::Trie() {
    root = new TrieNode('0');
}

Trie::~Trie() {
    deleteTrie(root);
}

void Trie::insert(const string &word) {
    auto cur = root;
    for (char c : word) {
        if (cur->next.count(c) == 0) {
            cur->next[c] = new TrieNode(c);
        }
        cur = cur->next[c];
    }
    cur->is_end = true;
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

void Trie::remove(const string &word) {
    auto cur = root;
    for (auto c : word) {
        if (!cur->next[c]) {
            return;
        }
        cur = cur->next[c];
    }
    cur->is_end = false;
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