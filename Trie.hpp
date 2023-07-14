#include <deque>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

using std::deque;
using std::pair;
using std::priority_queue;
using std::string;
using std::unordered_map;

class Trie {
    /**
     * @brief 字典树中结点的数据结构。
     *
     */
    struct TrieNode {
        TrieNode(const char v, const double w)
            : val(v), weight(w), is_end(false) {}
        char val;
        double weight; // 权重
        bool is_end;
        unordered_map<char, TrieNode *> next;
    };

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
    bool insert(const string &word, const double weight) const;

    /**
     * @brief 删除字符串。
     *
     * @param word 待删除字符串。
     * @return true 删除成功。
     * @return false 删除失败。
     */
    bool remove(const string &word) const;

    /**
     * @brief 查询是否存在给定字符串。
     *
     * @param word 待查询字符串。
     * @return true
     * @return false
     */
    bool has(const string &word) const;

    // 根据前缀词给出提示候选词。
    // word：前缀单词
    // 返回：候选词数组
    deque<string> prompt(const string &word, const int prompt_num);

  private:
    // 在字典树中遍历以当前结点为根结点的整棵树，将树中存在的单词加入到候选数组中
    // cur：当前结点
    // word：到当前结点时的单词
    // dict：候选词数组
    void traverse(const TrieNode *cur, string &word,
                  priority_queue<pair<double, string>> &min_heap,
                  const int prompt_num);

    // 删除以当前结点为根结点的整棵字典树
    void deleteTrie(TrieNode *cur);

    // 字典树根结点
    TrieNode *root;
};