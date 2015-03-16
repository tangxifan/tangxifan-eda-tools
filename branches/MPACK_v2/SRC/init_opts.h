/*
 * Opt_list is initialized in this file.
 * Add any option by following the format of s_optinfo
 * Attention: Think clearly before you include this file. It may cause serious problem such as empty your options!
 */
t_optinfo opt_list[] = 
{
  {"blif",0,ARG_WITHVAL,ARG_CHAR,ARG_MAND,ARG_NONDEF},
  {"mpack_blif",0,ARG_WITHVAL,ARG_CHAR,ARG_NONMAND,ARG_NONDEF},
  {"net",0,ARG_WITHVAL,ARG_CHAR,ARG_NONMAND,ARG_NONDEF},
  {"ble_arch",0,ARG_WITHVAL,ARG_CHAR,ARG_MAND,ARG_NONDEF},
  {"vpr_arch",0,ARG_WITHVAL,ARG_CHAR,ARG_NONMAND,ARG_NONDEF},
  {"stats",0,ARG_WITHVAL,ARG_CHAR,ARG_NONMAND,ARG_NONDEF},
  {"timing_analysis",0,ARG_WITHVAL,ARG_CHAR,ARG_NONMAND,ARG_NONDEF},
  {"alpha",0,ARG_WITHVAL,ARG_FLOAT,ARG_NONMAND,ARG_NONDEF},
  {"beta",0,ARG_WITHVAL,ARG_FLOAT,ARG_NONMAND,ARG_NONDEF},
  {"read_stdlib_blif",0,ARG_NONVAL,ARG_INT,ARG_NONMAND,ARG_NONDEF},
  {"pattern_member_type",0,ARG_WITHVAL,ARG_CHAR,ARG_NONMAND,ARG_NONDEF},
  //{"inner_cluster_delay",0,ARG_WITHVAL,ARG_FLOAT,ARG_NONMAND,ARG_NONDEF},
  {"inter_cluster_delay",0,ARG_WITHVAL,ARG_FLOAT,ARG_NONMAND,ARG_NONDEF},
  {"packer_algorithm",0,ARG_WITHVAL,ARG_CHAR,ARG_NONMAND,ARG_NONDEF},
  {"packer_area_attraction",0,ARG_WITHVAL,ARG_CHAR,ARG_NONMAND,ARG_NONDEF},
  {"forbid_unrelated_clustering",0,ARG_NONVAL,ARG_INT,ARG_NONMAND,ARG_NONDEF},
  {"verbose",0,ARG_NONVAL,ARG_INT,ARG_NONMAND,ARG_NONDEF},
  {"help",0,ARG_NONVAL,ARG_INT,ARG_NONMAND,ARG_NONDEF},
  // Keep this line please. It means the end of option list.
  {NULL,0,ARG_NONVAL,ARG_INT,ARG_NONMAND,ARG_NONDEF} 
}; 


