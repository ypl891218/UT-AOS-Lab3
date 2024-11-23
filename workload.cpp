#include <iostream>
#include <cstring>
#include <time.h>
using namespace std;

#pragma pack(1)
struct FixedSizeStruct {
    char data[4096];
};
#pragma pack()

FixedSizeStruct objectArr[100000];

int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cerr << "Please provide the access mode.\n";
        return 1;
    }

    int mode = atoi(argv[1]);

    if (mode == 1) {
        for (int i = 0; i < 100000; ++i) {
            for (int j = 0; j < 4096; ++j) {
                objectArr[i].data[j] = 'a' + i % 26;
            }
        }
    } else if (mode == 2) {
        for (int i = 0; i < 100000; i+=2) {
            for (int j = 0; j < 4096; ++j) {
                objectArr[i].data[j] = 'a' + i % 26;
            }
        }
        for (int i = 1; i < 100000; i+=2) {
            for (int j = 0; j < 4096; ++j) {
                objectArr[i].data[j] = 'a' + i % 26;
            }
        }
    } else if (mode == 3) {
        srand(static_cast<unsigned>(time(0)));
        for (int i = 0; i < 100000; ++i) {
            int idx = rand() % 100000;
            for (int j = 0; j < 4096; ++j) {
                objectArr[idx].data[j] = 'a' + i % 26;
            }
        }
    } else if (mode == 4) {
        srand(static_cast<unsigned>(time(0)));
        for (int i = 0; i < 50000; ++i) {
            int idx = rand() % 100000;
            for (int j = 0; j < 4096; ++j) {
                objectArr[idx].data[j] = 'a' + i % 26;
            }
        }
    } else {
        std::cerr << "Unsupported mode\n";
        return 1; 
    }

    cout << mode << endl;
    cout << sizeof(FixedSizeStruct) << endl;
    return 0;
}
