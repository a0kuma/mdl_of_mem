#include <iostream>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <map>
#include <type_traits>
#include <vector>
#include <cassert>
#include <set>

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
    unordered_map<Uio, ComputationIoSocket *> sockets;
    ComputationIoSockets(Compute_IO_type compute_IO_type,
                         Computation_description computation_description)
    {
        this->io_type = compute_IO_type;
        if (computation_description == Computation_description::forward)
        {
            if (compute_IO_type == Compute_IO_type::input)
            {
                ComputationIoSocket *socket_ptr_d_i_minus_1 = new ComputationIoSocket(Uio::d_i_minus_1, compute_IO_type, computation_description);
                sockets.emplace(Uio::d_i_minus_1, socket_ptr_d_i_minus_1);
                ComputationIoSocket *socket_ptr_weight = new ComputationIoSocket(Uio::weight, compute_IO_type, computation_description);
                sockets.emplace(Uio::weight, socket_ptr_weight);
            }
            else if (compute_IO_type == Compute_IO_type::output)
            {
                ComputationIoSocket *socket_ptr_d_i = new ComputationIoSocket(Uio::d_i, compute_IO_type, computation_description);
                sockets.emplace(Uio::d_i, socket_ptr_d_i);
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
                ComputationIoSocket *socket_ptr_partial_L_over_partial_d_i = new ComputationIoSocket(Uio::partial_L_over_partial_d_i, compute_IO_type, computation_description);
                sockets.emplace(Uio::partial_L_over_partial_d_i, socket_ptr_partial_L_over_partial_d_i);
                ComputationIoSocket *socket_ptr_d_i_minus_1 = new ComputationIoSocket(Uio::d_i_minus_1, compute_IO_type, computation_description);
                sockets.emplace(Uio::d_i_minus_1, socket_ptr_d_i_minus_1);
                ComputationIoSocket *socket_ptr_weight = new ComputationIoSocket(Uio::weight, compute_IO_type, computation_description);
                sockets.emplace(Uio::weight, socket_ptr_weight);
            }
            else if (compute_IO_type == Compute_IO_type::output)
            {
                ComputationIoSocket *socket_ptr_partial_L_over_partial_d_i_minus_1 = new ComputationIoSocket(Uio::partial_L_over_partial_d_i_minus_1, compute_IO_type, computation_description);
                sockets.emplace(Uio::partial_L_over_partial_d_i_minus_1, socket_ptr_partial_L_over_partial_d_i_minus_1);
                ComputationIoSocket *socket_ptr_partial_L_over_partial_weight = new ComputationIoSocket(Uio::partial_L_over_partial_weight, compute_IO_type, computation_description);
                sockets.emplace(Uio::partial_L_over_partial_weight, socket_ptr_partial_L_over_partial_weight);
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
        this->computation_description = computation_description;
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

class Device
{
public:
    int idx;
    vector<Layer *> layers;
    vector<Partition> partitions;
    Device(int idx)
    {
        this->idx = idx;
    }
};

class Partition
{
public:
    int idx;
    vector<Layer *> layers;
    Partition(int idx)
    {
        this->idx = idx;
    }
    void add_layer(Layer *layer)
    {
        layers.push_back(layer);
    }
};

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
    vector<Layer *> layers;
    vector<MemoryBlock *> memory_blocks;
    int arrayW_int[] = {56, 16, 24, 60, 100, 40, 56, 196, 84, 24, 16, 40};
    int arrayD_int[] = {28, 8, 8, 12, 20, 20, 8, 28, 28, 12, 8, 8, 20};
    assert(sizeof(arrayW_int) / sizeof(arrayW_int[0]) == (sizeof(arrayD_int) / sizeof(arrayD_int[0])) - 1);

    for (size_t i = 0; i < sizeof(arrayW_int) / sizeof(arrayW_int[0]); i++)
    {
        layers.push_back(new Layer(i + 1));
        Layer *current_layer = layers[i];

        MemoryBlock *the_w = new MemoryBlock(i + 1, arrayW_int[i], memory_description::weight);
        current_layer->forward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::weight)->assign_memory_block(the_w);
        current_layer->backward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::weight)->assign_memory_block(the_w);
        current_layer->backward_computation.io_sockets.at(Compute_IO_type::output).sockets.at(Uio::partial_L_over_partial_weight)->assign_memory_block(the_w);
        memory_blocks.push_back(the_w);

        MemoryBlock *d_i_minus_1 = new MemoryBlock(i, arrayD_int[i], memory_description::activation);
        memory_blocks.push_back(d_i_minus_1);
        current_layer->forward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::d_i_minus_1)->assign_memory_block(d_i_minus_1);
        current_layer->backward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::d_i_minus_1)->assign_memory_block(d_i_minus_1);
        current_layer->backward_computation.io_sockets.at(Compute_IO_type::output).sockets.at(Uio::partial_L_over_partial_d_i_minus_1)->assign_memory_block(d_i_minus_1);

        MemoryBlock *d_i = new MemoryBlock(i + 1, arrayD_int[i + 1], memory_description::activation);
        memory_blocks.push_back(d_i);
        current_layer->forward_computation.io_sockets.at(Compute_IO_type::output).sockets.at(Uio::d_i)->assign_memory_block(d_i);
        current_layer->backward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::partial_L_over_partial_d_i)->assign_memory_block(d_i);
    }
    int balance[] = {2, 2, 2, 2, 2, 2};
    int devices[] = {0, 0, 0, 1, 1, 1};
    // devices array to set
    set<int> device_set(devices, devices + sizeof(devices) / sizeof(devices[0]));
    map<int, Device *> devices_array;
    // for length of device set
    for (const auto &device_idx : device_set)
    {
        devices_array[device_idx] = new Device(device_idx);
    }
    int layer_idx = 0;
    for (size_t i = 0; i < sizeof(balance) / sizeof(balance[0]); i++)
    {
        Device *current_device = devices_array[devices[i]];
        for (int j = 0; j < balance[i]; j++)
        {
            Partition *current_partition = new Partition(j);
            current_device->partitions.push_back(*current_partition);
            Layer *current_layer = layers[layer_idx];
            current_device->layers.push_back(current_layer);
            current_partition->add_layer(current_layer);
            layer_idx++;
        }
    }
}

// 正向完成 要檢查