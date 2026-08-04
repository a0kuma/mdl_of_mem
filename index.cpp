#include <iostream>
#include <fstream>
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
    Compute_IO_type refIO;
    Computation_description refFB;
    ComputationIoSocket(Uio uio, Compute_IO_type compute_IO_type,
                        Computation_description computation_description)
    {
        this->uio = uio;
        this->refIO = compute_IO_type;
        this->refFB = computation_description;
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

class MemorySocketCollector
{
public:
    static MemorySocketCollector *add_memory_socket_collector(MemorySocketCollector *a, MemorySocketCollector *b);
    string name;
    vector<ComputationIoSocket *> sockets;
    MemorySocketCollector(string name);
};

class Device
{
public:
    int idx;
    vector<Layer *> layers;
    vector<Partition *> partitions;
    MemorySocketCollector *sum_of_weight_of_all_layers;
    Device(int idx);
    void do_MemorySocketCollector();
    vector<Partition *> get_partitions_higher_then(Partition *partition);
    vector<Layer *> get_layers_higher_then(Layer *layer);
};

class Partition
{
public:
    int idx;
    vector<Layer *> layers;
    MemorySocketCollector *high_ly_of_P_s_bkwd_s_dl_OV_ddi;
    MemorySocketCollector *lowest_of_p_s_bw_i_diM1;
    MemorySocketCollector *lowest_of_p_s_bw_i_diM1_FROM_OTHERS;
    Device *device = nullptr;
    Partition(int idx);
    void add_layer(Layer *layer);
    // High ly of P 's bkwd 's dl / ddi
    void do_MemorySocketCollector();
    bool is_lowset_layer(Layer *layer);
    vector<Layer *> get_lower_then(Layer *layer);
};

class Layer
{
public:
    int idx;
    Computation forward_computation = Computation(Computation_description::forward);
    Computation backward_computation = Computation(Computation_description::backward);
    Device *device = nullptr;
    Partition *partition = nullptr;
    MemorySocketCollector *sum_of_in_device_ly_idx_higher_then_you = nullptr;
    MemorySocketCollector *self3io = nullptr;
    MemorySocketCollector *refwd = nullptr;
    MemorySocketCollector *ans = nullptr;
    Layer(int idx);
    void do_MemorySocketCollector();
};

MemorySocketCollector::MemorySocketCollector(string name)
{
    this->name = name;
}

MemorySocketCollector *MemorySocketCollector::add_memory_socket_collector(MemorySocketCollector *a, MemorySocketCollector *b)
{
    // vector concade name string condade new and return ptr
    MemorySocketCollector *new_collector = new MemorySocketCollector(a->name + "+" + b->name);
    new_collector->sockets.insert(new_collector->sockets.end(), a->sockets.begin(), a->sockets.end());
    new_collector->sockets.insert(new_collector->sockets.end(), b->sockets.begin(), b->sockets.end());
    return new_collector;
}

Device::Device(int idx)
{
    this->idx = idx;
    sum_of_weight_of_all_layers = new MemorySocketCollector("sum_of_weight_of_all_layers");
}

void Device::do_MemorySocketCollector()
{
    // for 0 to layer length
    for (size_t i = 0; i < layers.size(); i++)
    {
        Layer *current_layer = layers[i];
        ComputationIoSocket *weight_socket = current_layer->forward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::weight);
        sum_of_weight_of_all_layers->sockets.push_back(weight_socket);
    }
}

vector<Partition *> Device::get_partitions_higher_then(Partition *partition)
{
    vector<Partition *> higher_partitions;
    for (size_t i = 0; i < partitions.size(); i++)
    {
        if (partitions[i]->idx > partition->idx)
        {
            higher_partitions.push_back(partitions[i]);
        }
    }
    return higher_partitions;
}

vector<Layer *> Device::get_layers_higher_then(Layer *layer)
{
    vector<Layer *> higher_layers;
    for (size_t i = 0; i < layers.size(); i++)
    {
        if (layers[i]->idx > layer->idx)
        {
            higher_layers.push_back(layers[i]);
        }
    }
    return higher_layers;
}

Partition::Partition(int idx)
{
    this->idx = idx;
    this->high_ly_of_P_s_bkwd_s_dl_OV_ddi = new MemorySocketCollector("[somehow typo] the left input of the partition (for example: master input x or di)");
    this->lowest_of_p_s_bw_i_diM1 = new MemorySocketCollector("[new] will effect p all alyer and all p higher all layer");
    this->lowest_of_p_s_bw_i_diM1_FROM_OTHERS = new MemorySocketCollector("FROM_OTHERS");
}

void Partition::add_layer(Layer *layer)
{
    layers.push_back(layer);
}

// High ly of P 's bkwd 's dl / ddi
void Partition::do_MemorySocketCollector()
{
    this->high_ly_of_P_s_bkwd_s_dl_OV_ddi->sockets.push_back(
        layers[layers.size() - 1]->backward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::partial_L_over_partial_d_i)); // IMPORTANT
    this->lowest_of_p_s_bw_i_diM1->sockets.push_back(
        layers[0]->backward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::d_i_minus_1));
    for (size_t i = 0; i < this->device->get_partitions_higher_then(this).size(); i++)
    {
        this->device->get_partitions_higher_then(this)[i]->lowest_of_p_s_bw_i_diM1_FROM_OTHERS->sockets.push_back(
            layers[0]->backward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::d_i_minus_1));
    }
}

bool Partition::is_lowset_layer(Layer *layer)
{
    return layers[0] == layer;
}

vector<Layer *> Partition::get_lower_then(Layer *layer)
{
    vector<Layer *> lower_layers;
    for (size_t i = 0; i < layers.size(); i++)
    {
#ifdef EXPECT_LAST_FALSE
        if (layers[i]->idx <= layer->idx)
#else
        if (layers[i]->idx < layer->idx)
#endif
        {
            lower_layers.push_back(layers[i]);
        }
    }
    return lower_layers;
}

Layer::Layer(int idx)
{
    this->idx = idx;
    sum_of_in_device_ly_idx_higher_then_you = new MemorySocketCollector("nihongoNO_dL_over_dw");
    self3io = new MemorySocketCollector("[typo is 4 i think] self3io");
    refwd = new MemorySocketCollector("for_all_lay_lower_then_s_fwd_di");
}

void Layer::do_MemorySocketCollector()
{
    for (size_t i = 0; i < device->get_layers_higher_then(this).size(); i++)
    {
        this->sum_of_in_device_ly_idx_higher_then_you->sockets.push_back(
            device->get_layers_higher_then(this)[i]->backward_computation.io_sockets.at(Compute_IO_type::output).sockets.at(Uio::partial_L_over_partial_weight));
    }
    // if (!this->partition->is_lowset_layer(this))
    // {
    //     this->self3io->sockets.push_back(
    //         this->backward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::d_i_minus_1));
    // }
    this->self3io->sockets.push_back(
        this->backward_computation.io_sockets.at(Compute_IO_type::input).sockets.at(Uio::partial_L_over_partial_d_i));
    this->self3io->sockets.push_back(
        this->backward_computation.io_sockets.at(Compute_IO_type::output).sockets.at(Uio::partial_L_over_partial_weight));
    this->self3io->sockets.push_back(
        this->backward_computation.io_sockets.at(Compute_IO_type::output).sockets.at(Uio::partial_L_over_partial_d_i_minus_1));

    for (size_t i = 0; i < partition->get_lower_then(this).size(); i++)
    {
        this->refwd->sockets.push_back(
            partition->get_lower_then(this)[i]->forward_computation.io_sockets.at(Compute_IO_type::output).sockets.at(Uio::d_i));
    }

    //---sum---
    this->ans = MemorySocketCollector::add_memory_socket_collector(this->sum_of_in_device_ly_idx_higher_then_you, this->self3io);
    this->ans = MemorySocketCollector::add_memory_socket_collector(this->ans, this->refwd);
    this->ans = MemorySocketCollector::add_memory_socket_collector(this->ans, this->partition->high_ly_of_P_s_bkwd_s_dl_OV_ddi);
    this->ans = MemorySocketCollector::add_memory_socket_collector(this->ans, this->partition->lowest_of_p_s_bw_i_diM1);
    this->ans = MemorySocketCollector::add_memory_socket_collector(this->ans, this->partition->lowest_of_p_s_bw_i_diM1_FROM_OTHERS);
    this->ans = MemorySocketCollector::add_memory_socket_collector(this->ans, this->device->sum_of_weight_of_all_layers);
}

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
        devices_array[device_idx + 1] = new Device(device_idx + 1);
    }
    int layer_idx = 0;
    for (size_t i = 0; i < sizeof(balance) / sizeof(balance[0]); i++)
    {
        Device *current_device = devices_array[devices[i] + 1];
        Partition *current_partition = new Partition(i + 1);
        for (int j = 0; j < balance[i]; j++)
        {
            Layer *current_layer = layers[layer_idx];
            current_layer->device = current_device;
            current_layer->partition = current_partition;
            current_device->layers.push_back(current_layer);
            current_partition->add_layer(current_layer);
            current_partition->device = current_device;
            layer_idx++;
        }
        current_device->partitions.push_back(current_partition);
    }

    //--master--
    // for loop device array
    for (const auto &device_pair : devices_array)
    {
        Device *current_device = device_pair.second;
        current_device->do_MemorySocketCollector();
        for (size_t i = 0; i < current_device->partitions.size(); i++)
        {
            Partition *current_partition = current_device->partitions[i];
            current_partition->do_MemorySocketCollector();
            for (size_t j = 0; j < current_partition->layers.size(); j++)
            {
                Layer *current_layer = current_partition->layers[j];
                current_layer->do_MemorySocketCollector();
            }
        }
    }

    //--print--
    // for loop layers
    string hh = "<html><head><meta charset='utf-8'><title>report</title><script src='https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js'></script> \
    <style> \
        table, th, td {\
            border: 1px solid black;\
            border-collapse: collapse;\
        }\
    </style> \
    </head> \
    <body><table> \
    <thead><tr><th>uio</th><th>memory_idx</th><th>memory_size</th><th>memory_type</th><th>ref_io</th><th>ref_fb</th></tr></thead><tbody>";
    for (size_t i = 0; i < layers.size(); i++)
    {
        Layer *current_layer = layers[i];
        vector<ComputationIoSocket *> *tmp = &(current_layer->ans->sockets);
        hh += "<tr><td>" + to_string(current_layer->idx) + "</td><td colspan='5'>" + current_layer->ans->name + "</td></tr>";
        // for loop tmp
        for (size_t j = 0; j < tmp->size(); j++)
        {
            ComputationIoSocket *current_socket = (*tmp)[j];
            string uio_text = current_socket->uio == Uio::d_i
                                  ? "\\(d_i\\)"
                              : current_socket->uio == Uio::partial_L_over_partial_d_i
                                  ? "\\(\\frac{\\partial L}{\\partial d_i}\\)"
                              : current_socket->uio == Uio::d_i_minus_1
                                  ? "\\(d_{i-1}\\)"
                              : current_socket->uio == Uio::weight
                                  ? "\\(w_i\\)"
                              : current_socket->uio == Uio::partial_L_over_partial_d_i_minus_1
                                  ? "\\(\\frac{\\partial L}{\\partial d_{i-1}}\\)"
                                  : "\\(\\frac{\\partial L}{\\partial w_i}\\)";
            string memory_type_text = current_socket->memory_block->memory_type == memory_description::weight ? "weight" : "activation";
            hh += "<tr><td>" + uio_text + "</td><td>" + to_string(current_socket->memory_block->idx) + "</td><td>" + to_string(current_socket->memory_block->MemSize) + "</td><td>" + memory_type_text + "</td>";
            //+refIO refFB
            string refIO_text = current_socket->refIO == Compute_IO_type::input ? "input" : "output";
            string refFB_text = current_socket->refFB == Computation_description::forward ? "forward" : "backward";
            hh += "<td>" + refIO_text + "</td><td>" + refFB_text + "</td></tr>";
        }
    }
    string ht = "</tbody></table></body></html>";
    // save as yyyymmddhhmmss.html
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char filename[20];
    sprintf(filename, "%04d%02d%02d%02d%02d%02d.html", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    ofstream outfile(filename);
    outfile << hh << ht;
    outfile.close();
}
