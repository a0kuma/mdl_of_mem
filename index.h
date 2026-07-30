
enum class memory_description {weight, activation};
class MemoryBlock;
class ComputationIO;

enum class fwd_in {d_i_minus_1, weight};
enum class fwd_out {d_i};

enum class bwd_in {partial_L_over_partial_d_i, d_i_minus_1, weight};
enum class bwd_out {partial_L_over_partial_d_i_minus_1, partial_L_over_partial_weight};
enum class compute_IO_type {input, output};

struct II{
    fwd_in fwd_in_type;
    bwd_in bwd_in_type;
};
struct OO{
    fwd_out fwd_out_type;
    bwd_out bwd_out_type;
};