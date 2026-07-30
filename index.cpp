#include <iostream>
#include <string>
#include <unordered_map>
#include <type_traits>

#include "index.h"
using namespace std;

class ComputationIoSocket
{
public:
    static ComputationIoSocket d_i;
    static ComputationIoSocket partial_L_over_partial_d_i;
    static ComputationIoSocket d_i_minus_1;
    static ComputationIoSocket weight;
    static ComputationIoSocket partial_L_over_partial_d_i_minus_1;
    static ComputationIoSocket partial_L_over_partial_weight;
    static unordered_map<Computation_description,> get_by_Compute_IO_type(){

    }
};

class SocketSelector
{
    public:
  
    //enum class Fwd_in {
  static SocketSelector  d_i_minus_1, weight 
,
    //};
//enum class Fwd_out {
d_i
//};
,
//enum class Bwd_in {
partial_L_over_partial_d_i//, d_i_minus_1, weight
//};
,
//enum class Bwd_out {
partial_L_over_partial_d_i_minus_1, partial_L_over_partial_weight
;
//};
  SocketSelector(Compute_IO_type compute_IO_type,Computation_description  computation_description){
//nest if else
if (computation_description==Computation_description::forward){
    if (compute_IO_type==Compute_IO_type::input){
        //return Fwd_in::d_i_minus_1;
    }else if (compute_IO_type==Compute_IO_type::output){
        //return Fwd_out::d_i;
    }else{
        throw invalid_argument(STRING_FUCKED);
    }
}else if (computation_description==Computation_description::backward){
    if (compute_IO_type==Compute_IO_type::input){
        //return Bwd_in::partial_L_over_partial_d_i;
    }else if (compute_IO_type==Compute_IO_type::output){
        //return Bwd_out::partial_L_over_partial_d_i_minus_1;
    }else{
        throw invalid_argument(STRING_FUCKED);
    }
}else{
    throw invalid_argument(STRING_FUCKED);
}
    
};

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