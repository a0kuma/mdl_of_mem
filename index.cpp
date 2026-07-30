#include <iostream>
#include <string>
#include <unordered_map>
#include <type_traits>

#include "index.h"
using namespace std;


class Computation
{
public:
Computation_description computation_description;
unordered_map<Compute_IO_type, ComputationIoSocket[]> sockets;
Computation(Computation_description computation_description){
int socket_count = -1;
        switch (computation_description)
        {
            forward:
            socket_count= static_cast<int>(Fwd_in::Count)
            backward:
            default:
                throw invalid_argument(STRING_FUCKED);
        }
}
    
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