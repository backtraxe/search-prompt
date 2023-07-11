#include <iostream>

#include "Trie.h"

int main() {




    Trie trie;
    string s = "周杰伦";
    trie.insert(s);
    s = "周杰一";
    trie.insert(s);
    vector<string> dict = trie.prompt("周杰伦");
    for (auto x : dict) {
        std::cout << x << std::endl;
    }


    return 0;
}