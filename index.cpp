#include <iostream>
#include <string>
#include "index.h"
using namespace std;

struct Computation
{
    ComputationIO<II>** input;
    ComputationIO<OO>** output;
};

template <typename T>
class ComputationIO
{
public:
T c;
MemoryBlock *mb;
    ComputationIO(MemoryBlock *mb, T c)//compute_IO_type compute_IO_t,)
    {
this->c = c;
this->mb = mb;
    };
    get_io()
}

class Forward
{
public:
    Computation compute;
    Forward()
    {
compute.input = ComputationIO();
compute.output = nullptr;
    };
};

class Backward
{
public:
    Computation compute;
    Backward()
    {

    };
};

class Device
{
public:
    string name;
};

class Partition
{
public:
    string name;
};

class Layer
{
public:
    string name;
};

class MemoryBlock
{
public:
    string name;
    int MemSize;
    memory_description memory_type;
};

int main()
{
    cout << "Hello, World!" << endl;
}