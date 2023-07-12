#include <unordered_map>
#include <vector>
#include <string>

using std::unordered_map;
using std::vector;
using std::string;

class Trie {

    // 字典树结点
    struct TrieNode {
        TrieNode(const char val) : val(val), is_end(false) {}
        char val;
        bool is_end;
        unordered_map<char, TrieNode *> next;
    };

  public:

    Trie();

    ~Trie();

    // 向字典树中添加单词。
    // word：待添加单词
    void insert(const string &word);

    // 从字典树中删除单词。
    // word：待删除单词
    void remove(const string &word);

    // 查询字典树中是否存在给定单词
    bool has(const string &word) const;

    // 根据前缀词给出提示候选词。
    // word：前缀单词
    // 返回：候选词数组
    vector<string> prompt(const string &word);

  private:

    // 在字典树中遍历以当前结点为根结点的整棵树，将树中存在的单词加入到候选数组中
    // cur：当前结点
    // word：到当前结点时的单词
    // dict：候选词数组
    void traverse(const TrieNode *cur, string &word, vector<string> &dict);

    // 删除以当前结点为根结点的整棵字典树
    void deleteTrie(TrieNode *cur);

    // 字典树根结点
    TrieNode *root;
};