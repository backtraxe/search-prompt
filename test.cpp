#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

int main() {
  ifstream ifs("hourly_smartbox_json.2023071200");
  string line;
  int count = 0;
  while (getline(ifs, line)) {
    count++;
  }
  cout << count << "\n";
  ifs.close();

  return 0;
}