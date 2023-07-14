#include <iostream>

#include "Utility.hpp"
using namespace std;

int main() {
    auto file = Utility::read_file("dict.txt");
    for (auto &line : file) {
        cout << line << endl;
    }

    return 0;
}