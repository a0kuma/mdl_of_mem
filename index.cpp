#include <iostream>
#include <string>
#include "index.h"
using namespace std;

struct Forward{
    Activation* d_i;
    Activation* d_i_Minus_1;
};

struct Backward{
    Activation* partial_L_over_partial_d_i;
    Activation* partial_L_over_partial_d_i_Minus_1;
};

class Device{
    public:
        string name;
};

class Partition{
    public:
        string name;
};

class Layer{
    public:
        string name;
};

class weight{
    public:
        string name;
        int MemSize;
        Forward forward;
};

class Activation{
    public:
        string name;
        int MemSize;
};



int main() {
    cout << "Hello, World!" << endl;
}