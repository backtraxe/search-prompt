#include <iostream>

#include "../src/ThreadPool.hpp"

using namespace std;

int addTwo(int a, int b) { return a + b; }

int main() {
    ThreadPool threadPool(5);
    // threadPool.addTask(addTwo, 1, 2);

    std::vector<std::future<int>> results;
    for (int i = 1; i <= 10; i++) {
        results.emplace_back(threadPool.addTask([i] {
            cout << i << endl;
            return i;
        }));

        // threadPool.addTask([i] { cout << i << endl; });
    }

    return 0;
}