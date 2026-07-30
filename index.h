#define STRING_FUCKED "u r fucked"
enum class memory_description {weight, activation};

// enum class Fwd_in {d_i_minus_1, weight};
// enum class Fwd_out {d_i};

// enum class Bwd_in {partial_L_over_partial_d_i, d_i_minus_1, weight};
// enum class Bwd_out {partial_L_over_partial_d_i_minus_1, partial_L_over_partial_weight};

enum class Uio {d_i, partial_L_over_partial_d_i, d_i_minus_1, weight, partial_L_over_partial_d_i_minus_1, partial_L_over_partial_weight};
enum class Compute_IO_type {input, output};
enum class Computation_description {forward, backward};

class MemoryBlock;
class ComputationIoSocket;
class ComputationIoSockets;
class Computation;
class Device;
class Layer;
class Partition;
class MemorySocketCollector