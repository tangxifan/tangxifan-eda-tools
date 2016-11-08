/* global parameters for dumping synthesizable verilog */
extern char* verilog_top_postfix;
extern char* bitstream_verilog_file_postfix;
extern char* top_testbench_verilog_file_postfix;
extern char* logic_block_verilog_file_name;
extern char* luts_verilog_file_name;
extern char* routing_verilog_file_name;
extern char* muxes_verilog_file_name;
extern char* wires_verilog_file_name;
extern char* decoders_verilog_file_name;
extern char* verilog_mux_basis_posfix;
extern char* verilog_mux_special_basis_posfix;

extern t_spice_model* sram_verilog_model;
extern enum e_sram_orgz sram_verilog_orgz_type;

/* Number of configuration bits of each switch block */
extern int** num_conf_bits_sb;
/* Number of configuration bits of each Connection Box CHANX */
extern int** num_conf_bits_cbx;
/* Number of configuration bits of each Connection Box CHANY */
extern int** num_conf_bits_cby;

/* Input and Output Pad spice model. should be set as global */
extern t_spice_model* inpad_verilog_model;
extern t_spice_model* outpad_verilog_model;
extern t_spice_model* iopad_verilog_model;
/* Linked-list that stores all the configuration bits */
extern t_llist* conf_bits_head;

/* Prefix of global input, output and inout of a I/O pad */
extern char* gio_input_prefix;
extern char* gio_output_prefix;
extern char* gio_inout_prefix;

extern int verilog_default_signal_init_value;

/* Enumeration */
enum e_pin2pin_interc_type {
 INPUT2INPUT_INTERC, OUTPUT2OUTPUT_INTERC
};

