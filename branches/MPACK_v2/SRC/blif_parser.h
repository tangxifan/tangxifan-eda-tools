/**
 * Type enumeration for blif marcos
 */
enum e_blif_type
{
  BLIF_NONE,
  BLIF_LIBGATE,
  BLIF_LATCH,
  BLIF_GATE,
};


/*
 * Connection(Conkt) wires
 * idx: index in the array
 * name: name of the connection in blif
 * input_num: number of inputs(it should be 1 or 0!)
 * inputs: list of input logic nodes(Usually there is only one element)
 * output_num: number of outputs
 * outputs: list of output logic nodes
 * next: pointer to next connections in the list 
 */
typedef struct s_conkt t_conkt;
struct s_conkt
{
  int idx;
  char* name;
  int input_num;
  enum e_blif_type inputs_types;
  int inputs;
  int output_num;
  enum e_blif_type* outputs_types;
  int* outputs;
  t_conkt* next;
  /* Flags to identify PI and PO, 1 as true*/
  int is_pi;
  int pi_idx;
  int is_po;
  int po_idx;
  //counter to help assign outputs
  int iout; 
};



/**
 * Types for a blif parser
 */
typedef struct s_blif_ios t_blif_ios;
struct s_blif_ios
{
  int idx;
  char* port; // port name
  char* name; // mapping name
  /*conkt pointor, easy to traversal*/
  t_conkt* conkt;
};


/* Attributes for a blif gate(started with .name)*/
typedef struct s_blif_gate t_blif_gate;
struct s_blif_gate
{
  int idx;
  int input_num;
  t_blif_ios* inputs;
  /*Single Output!*/
  t_blif_ios output; 
  int is_buf;
  /* Single-output-cover*/ 
  //t_blif_ios* covers;
};

typedef struct s_blif_libgate t_blif_libgate;
struct s_blif_libgate
{
  int idx;
  char* name;
  int input_num;
  t_blif_ios* inputs;
  /*Single Output!*/
  t_blif_ios output;
};

typedef struct s_blif_latch t_blif_latch;
struct s_blif_latch
{
  int idx;
  t_blif_ios input;
  t_blif_ios output;
  char* type;
  char* clk;
  int   init_val;
};

/*
 * Architecture summary for a blif file
 * Contain all structs;
 */
typedef struct s_blif_arch t_blif_arch;
struct s_blif_arch
{
  char* model;

  int input_num;
  t_blif_ios* inputs;

  int output_num;
  t_blif_ios* outputs; 

  int clk_num;
  t_blif_ios* clk_names;

  int gate_num;
  t_blif_gate* gates;

  int libgate_num;
  t_blif_libgate* libgates;

  int latch_num;
  t_blif_latch* latches;

  t_conkt* nets;
};


