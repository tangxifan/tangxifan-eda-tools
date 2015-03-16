/*
 * Data structures in modeling netlists
 */
typedef struct s_typepair t_typepair;
typedef struct s_edge t_edge;
typedef struct s_lgknd t_lgknd;
typedef struct s_lgkntwk t_lgkntwk;
typedef struct s_ble_info t_ble_info;
typedef struct s_delay_info t_delay_info;

struct s_delay_info {
  float inpad_delay;
  float outpad_delay;
  float gate_delay;
  float ff_tsu;
  float ff_delay;
  float inter_cluster_delay;
  float inner_cluster_delay;
  float ff_mux_delay;
  float inter_cluster_delay_per_unit;
};

/** Type of BLE input and output conkts
 */
enum ble_inout_type
{BLE_NORMAL,
 BLE_PI,
 BLE_PO,
 BLE_LATCH};

/**Enumeration for logic node types.
 */
enum lgknd_type
{ND_AND,
 ND_XNOR,
 ND_ADD,
 ND_CARRY,
 ND_INV,
 ND_BUF,
 ND_NAND,
 ND_XOR,
 ND_NADD,
 ND_NCARRY,
 ND_ZERO,
 ND_ONE,
 ND_LUT,
 ND_LATCH,
 ND_PI,
 ND_PO};

/* To identify logic node type easily.
 * Create pairs to match name and id.
 */
struct s_typepair
{
  char* type_name;
  int type_id;
};

struct s_edge {
  float timing_weight;
  float depth_weight;
  float weight;
  float delay;
  float t_arrival;
  float t_required;
  float slack;
};

/*
 * Logic Node
 * idx: index in the array
 * type: logic node type(defined in blif)
 * input_num: number of inputs(usually it is 1/2/3) 
 * inputs: pointer list of input logic nodes
 * output_num: number of outputs(fanout)
 * outputs: pointer list of output logic nodes(fanout list)
 * input_conkts: list for input connections (only used in parser)
 * output_conkts: list for output connections (only used in parser)
 * init_val: initial value for latches,flip-flops
 * next: next pointer in a list
 */
struct s_lgknd
{
  int idx;
  enum lgknd_type type;
  int input_num;
  t_lgknd** inputs;
  int output_num;
  t_lgknd** outputs;
  t_edge* in_edges;
  t_edge** out_edges;
  // Only for parser
  //t_conkt* input_conkt;
  //t_conkt* output_conkt; 
  /* Only for latch */
  int init_val; 
  /* Timing information*/
  float delay;
  float t_arrival;
  float t_required;
  float slack;
  /* Mapping flags*/
  int mapped;
  int depth;
  int dfsed;
  int map_climb;
  int try_sat;
  int* try_pattern_types;
  int redund_output_num;
  int tmp_redund_output_num;
  int fanout_buf_num;
  int cur_edge;
  int comb_num;
  int logic_equivalent;
  int strict;
  float input_paths_affect;
  float output_paths_affect;
  float depth_source;
  
  /*BLE IDs*/
  int ble_idx;
  int ble_cell_idx;
  void* ble_info;
  /*Output blif info*/
  int output_conkt;
  int pio_conkt;
  int latch_conkt;
  char* net_name;
  char* io_name;

  /* List next pointor */
  t_lgknd* next;
};

struct s_lgkntwk
{
  t_lgknd* pi_head;
  t_lgknd* po_head;
  t_lgknd* libgate_head;
  t_lgknd* latch_head;

  int libgate_num;
  t_lgknd** libgate_ptrs;
  int pi_num;
  t_lgknd** pi_ptrs;
  int po_num;
  t_lgknd** po_ptrs;
  int latch_num;
  t_lgknd** latch_ptrs;

  float critical_delay;
  float max_slack;
};

/**
 * Store BLE packing results 
 */
struct s_ble_info
{
  int idx;

  /*Inter-BLE connections*/
  int input_num;
  int* input_conkts; // index of conkts
  int* input_conkts_type; // type of conkts
  int* input_used; // 1 used, 0 unused
  t_lgknd** input_lgknds;
  int* input_lgknds_input_idx;
 
  /*Inside-BLE connections*/
  int blend_num;
  int** blend_port_used;
  int** blend_muxes;
  int* blend_used;
  t_lgknd** blend_lgknds;
  int** blend_lgknd_input_idxes;

  /*BLE output connections*/
  int output_num;
  int* output_conkts; // Index of conkt
  int* output_conkts_type; // type of conkts
  int* output_used;
  int* output_muxes;  
  int* output_types; // Seq:0 Comb:1
  t_lgknd** output_lgknds;
  
  t_ble_info* next;
};
  
