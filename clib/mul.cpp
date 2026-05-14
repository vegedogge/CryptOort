#include "iostream"
#include "math.h"
using namespace std;

extern "C"{
    int mul(int a, int b)
    {
        int c=a*b;
        cout<<"Multiplication completed!"<<endl;

        return c;
    }
}

