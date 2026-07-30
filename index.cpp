#include <iostream>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <type_traits>
#include <vector>
#include <cassert>

#include "index.h"
using namespace std;

class MemoryBlock
{
public:
    int idx;
    int MemSize;
    memory_description memory_type;
    MemoryBlock(int idx, int MemSize, memory_description memory_type)
    {
        this->idx = idx;
        this->MemSize = MemSize;
        this->memory_type = memory_type;
    }
};

class ComputationIoSocket
{
public:
    Uio uio;
    MemoryBlock *memory_block = nullptr;
    ComputationIoSocket(Uio uio, Compute_IO_type compute_IO_type,
                        Computation_description computation_description)
    {
        this->uio = uio;
    }
    void assign_memory_block(MemoryBlock *memory_block)
    {
        if (uio == Uio::weight || uio == Uio::partial_L_over_partial_weight)
        {
            assert(memory_block->memory_type == memory_description::weight);
        }
        else
        {
            assert(memory_block->memory_type != memory_description::weight);
        }
        this->memory_block = memory_block;
    }
};

class ComputationIoSockets
{
public:
    Compute_IO_type io_type;
    unordered_map<Uio, ComputationIoSocket> sockets;
    ComputationIoSockets(Compute_IO_type compute_IO_type,
                         Computation_description computation_description)
    {
        this->io_type = compute_IO_type;
        if (computation_description == Computation_description::forward)
        {
            if (compute_IO_type == Compute_IO_type::input)
            {
                sockets.emplace(Uio::d_i_minus_1, ComputationIoSocket(Uio::d_i_minus_1, compute_IO_type, computation_description));
                sockets.emplace(Uio::weight, ComputationIoSocket(Uio::weight, compute_IO_type, computation_description));
            }
            else if (compute_IO_type == Compute_IO_type::output)
            {
                sockets.emplace(Uio::d_i, ComputationIoSocket(Uio::d_i, compute_IO_type, computation_description));
            }
            else
            {
                throw invalid_argument(STRING_FUCKED);
            }
        }
        else if (computation_description == Computation_description::backward)
        {
            if (compute_IO_type == Compute_IO_type::input)
            {
                sockets.emplace(Uio::partial_L_over_partial_d_i, ComputationIoSocket(Uio::partial_L_over_partial_d_i, compute_IO_type, computation_description));
                sockets.emplace(Uio::d_i_minus_1, ComputationIoSocket(Uio::d_i_minus_1, compute_IO_type, computation_description));
                sockets.emplace(Uio::weight, ComputationIoSocket(Uio::weight, compute_IO_type, computation_description));
            }
            else if (compute_IO_type == Compute_IO_type::output)
            {
                sockets.emplace(Uio::partial_L_over_partial_d_i_minus_1, ComputationIoSocket(Uio::partial_L_over_partial_d_i_minus_1, compute_IO_type, computation_description));
                sockets.emplace(Uio::partial_L_over_partial_weight, ComputationIoSocket(Uio::partial_L_over_partial_weight, compute_IO_type, computation_description));
            }
            else
            {
                throw invalid_argument(STRING_FUCKED);
            }
        }
        else
        {
            throw invalid_argument(STRING_FUCKED);
        }
    }
};

class Computation
{
public:
    Computation_description computation_description;
    unordered_map<Compute_IO_type, ComputationIoSockets> io_sockets;
    Computation(Computation_description computation_description)
    {
        switch (computation_description)
        {
        case Computation_description::forward:
            io_sockets.emplace(Compute_IO_type::input, ComputationIoSockets(Compute_IO_type::input, computation_description));
            io_sockets.emplace(Compute_IO_type::output, ComputationIoSockets(Compute_IO_type::output, computation_description));
            break;
        case Computation_description::backward:
            io_sockets.emplace(Compute_IO_type::input, ComputationIoSockets(Compute_IO_type::input, computation_description));
            io_sockets.emplace(Compute_IO_type::output, ComputationIoSockets(Compute_IO_type::output, computation_description));
            break;
        default:
            throw invalid_argument(STRING_FUCKED);
        }
    }
};

// class Device
// {
// public:
//     string name;
// };

// class Partition
// {
// public:
//     string name;
// };

class Layer
{
public:
    int idx;
    Computation forward_computation = Computation(Computation_description::forward);
    Computation backward_computation = Computation(Computation_description::backward);
    Layer(int idx)
    {
        this->idx = idx;
    }
};

int main()
{
    vector<Layer> layers;
    vector<MemoryBlock*> memory_blocks;
    int arrayW_int[] = {56, 16, 24, 60, 100, 40, 56, 196, 84, 24, 16, 40};
    int arrayD_int[] = {28, 8, 8, 12, 20, 20, 8, 28, 28, 12, 8, 8, 20};
    assert(sizeof(arrayW_int) / sizeof(arrayW_int[0]) == (sizeof(arrayD_int) / sizeof(arrayD_int[0])) - 1);

    for (size_t i = 0; i < sizeof(arrayW_int) / sizeof(arrayW_int[0]); i++)
    {
        layers.push_back(Layer(i + 1));
        // IMPORTANT : do it like this
        MemoryBlock* tmp1 = new MemoryBlock(i + 1, arrayW_int[i], memory_description::weight);
		layers.back().forward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::weight).assign_memory_block(tmp1);
		layers.back().backward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::weight).assign_memory_block(tmp1);
		layers.back().backward_computation.io_sockets.at(Compute_IO_type::output).sockets.at(Uio::partial_L_over_partial_weight).assign_memory_block(tmp1);
		memory_blocks.push_back(tmp1);
        // END OF ~

        //START OF TODO
        memory_blocks.push_back(MemoryBlock(i, arrayD_int[i], memory_description::activation));
        layers.back().forward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::d_i_minus_1).assign_memory_block(&(memory_blocks.back()));
        layers.back().backward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::d_i_minus_1).assign_memory_block(&(memory_blocks.back()));
        layers.back().backward_computation.io_sockets.at(Compute_IO_type::output).sockets.at(Uio::partial_L_over_partial_d_i_minus_1).assign_memory_block(&(memory_blocks.back()));
        memory_blocks.push_back(MemoryBlock(i + 1, arrayD_int[i + 1], memory_description::activation));
        layers.back().forward_computation.io_sockets.at(Compute_IO_type::output).sockets.at(Uio::d_i).assign_memory_block(&(memory_blocks.back()));
        layers.back().backward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::partial_L_over_partial_d_i).assign_memory_block(&(memory_blocks.back()));
    //END OF TODO
    
    }
}