//g++ -shared -o libtest_sort.so -fPIC test_sort.cpp//编译成动态链接库
#include <iostream>
#include <algorithm>

extern "C" {
    void sort_array(double* arr, int size) {
        std::sort(arr, arr + size);
    }
}
