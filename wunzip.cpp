//wunzip.cpp, by Yingwu Zhu
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << string(argv[0]+2) << " file1 [file2 ...]\n";
    exit(1);
  }
  for (int i = 1; i < argc; i++) {
    ifstream fin(argv[i], ifstream::in);
    if  (!fin.is_open()) {
      cout << string(argv[0]+2) << ": cannot open file\n";
      exit(1);
    }
    int count = 0;
    char c;
    while (fin.read((char*)&count, sizeof(int))) {
      fin.read(&c, 1);
      while (count--)
        cout << c;
    }
    fin.close();
  }
  return 0;
}