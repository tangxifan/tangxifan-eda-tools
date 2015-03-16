/**
 * BLE Architecture description structs
 */
/**
 * Enum of BLE port type
 */
enum ble_port_type
{BLE_IN,
BLE_OUT,
BLE_ND,
};


/**
 * Port of a BLE or BLE cell 
 * idx: index of port. For a ble cell, it follows the port name. a(index=0), b(index=1)
 * name: port name in BLE architecture description file
 * cand_num: number of candidates
 * candtoks: strings store the name of candidates
 * cands: candidates for the inputs (NULL if this is a input of BLE)
 * type: type of this BLE port. It can be an input or output or a part of BLE node
 * blend_idx: the index of BLE node which this port belongs to. For an input or output, leave it as -1
 * is_comb: if this is a combination output, valid only for BLE output
 */
typedef struct s_bleport t_bleport;
struct s_bleport
{
  int idx;
  char* name;
  int cand_num;
  char** candtoks;
  t_bleport** cands;

  enum ble_port_type type;
  int blend_idx;
  int is_comb;
  int depth;
  
  /*Inverter solutions*/
  int inv_capable;
};

/**
 * BLE node: a logic cell inside BLE
 * idx: index of cell in BLE architecture file
 * name: "cell"+idx, e.g. cell0
 * input_num: number of input
 * inputs: input attributes
 */
typedef struct s_blend t_blend;
struct s_blend
{
  int idx; 
  char* name;
  int input_num;
  t_bleport* inputs;

  int sat2_capable;
  int sat3_capable;
  int pattern_type;
  int curin;
  float priority;
  int depth;
};

/**
 * BLE Arch Struct
 * input_num: number of inpit
 * inputs: inputs attributes
 * output_num: number of output 
 * outputs: outputs attributes
 * blend_num : number of BLE nodes
 * blends: BLE nodes attributes
 */
typedef struct s_ble_arch t_ble_arch;
struct s_ble_arch
{
  char* name;

  int input_num;
  t_bleport* inputs;
  
  int output_num;
  t_bleport* outputs; 

  int blend_num;
  t_blend* blends;

  float area;
  float rmetal;
  float cmetal;
  float cell_delay;
  float inner_cluster_delay;
  int curnd;
}; 
