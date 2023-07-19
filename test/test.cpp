#include <chrono>
#include <iostream>
#include <sys/time.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

using namespace std;

int addTwo(int a, int b) { return a + b; }

struct Person {
    string name;
    int age;
};

int main() {
    // struct timeval t1, t2;
    // gettimeofday(&t1, NULL);
    // auto st = clock();
    auto st = chrono::system_clock::now();
    { this_thread::sleep_for(chrono::milliseconds(100)); }
    // sleep(3);
    auto et = chrono::system_clock::now();

    // auto et = clock();
    // gettimeofday(&t2, NULL);

    // cout << ((double)et - st) / CLOCKS_PER_SEC * 1000 << endl;
    // cout << ((t2.tv_sec - t1.tv_sec) * 1000 +
    //          (double)(t2.tv_usec - t1.tv_usec) / 1000)
    //      << endl;
    auto inv = et - st;
    cout << inv.count() << endl;

    return 0;
}