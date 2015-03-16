//
// Enumeration
enum e_pattern_member_type {
PATTERN_MEMBER_NORMAL,
PATTERN_MEMBER_COMBONLY,
};

enum e_packer_algorithm {
PACKER_ALGORITHM_SAT,
PACKER_ALGORITHM_SEED,
PACKER_ALGORITHM_PATTERN,
};

enum e_packer_area_attraction {
PACKER_AREA_ATTRACTION_AAPACK,
PACKER_AREA_ATTRACTION_VPACK,
};

/*Struct for configuring mpack engine*/
typedef struct s_mpack_conf t_mpack_conf;
struct s_mpack_conf
{
  char* blif;
  char* mpack_blif;
  char* net;
  char* ble_arch;
  char* vpr_arch;
  char* stats;
  /*Enhancements*/
  int timing_analysis;
  float inner_cluster_delay;
  float inter_cluster_delay;
  float alpha;
  float beta;
  int forbid_unrelated_clustering;
  /*Others*/
  int read_stdlib_blif;
  enum e_pattern_member_type pattern_member_type; 
  enum e_packer_algorithm packer_algorithm;
  enum e_packer_area_attraction packer_area_attraction;
  /*Minor options*/
  int verbose;
  int help;
};
 
/*Struct for libgates stats*/
typedef struct s_libgate_stats t_libgate_stats;
struct s_libgate_stats
{
  int total;
  int and_num;
  int xnor_num;
  int add_num;
  int carry_num;
  int inv_num;
  int buf_num;
  int zero_num;
  int one_num;
};

/*Struct for stats*/
typedef struct s_stats t_stats;
struct s_stats
{
  /* Basic Stats of netlist*/
  int libgate_num;
  /** Same stats as libgate_stats
   *  0: AND_NUM, 1:XNOR_NUM, 2:ADD_NUM,
   *  3: CARRY_NUM, 4:INV_NUM, 5:BUF_NUM,
   *  6: NAND_NUM, 7: XOR_NUM, 8: NADD_NUM,
   *  9: NCARRY_NUM 10: ZERO_NUM 11: ONE_NUM
   *  12: LUT_NUM
   */
  int stdgate_stats[13]; 
  int max_lut_size;
  int* lut_stats;
  int pi_num;
  int po_num;
  int latch_num; 

  int ble_num;
  float ble_fill_rate;
  float libgate_latch_fanout_avg;
  float avg_lt10;
};
