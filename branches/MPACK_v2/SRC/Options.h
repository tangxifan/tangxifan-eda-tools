/**
 * Filename : Options.h
 * Author : Xifan TANG, EPFL
 * Description : Header file contains structs and enumeration 
 *               types for option reading purpose.
 */

/*Determine whether it is a mandatory option*/
enum opt_manda
{ARG_MAND,ARG_NONMAND};

/*
 * The option has been appeared in the command line
 * In the s_optinfo, fill it with DEF. 
 */
enum opt_default
{ARG_DEF,ARG_NONDEF};

/*Determine whether the option contains a value*/
enum opt_with_val
{ARG_NONVAL,ARG_WITHVAL};

/*Determine the date type of value*/
enum opt_val_type
{ARG_INT,ARG_FLOAT,ARG_CHAR,ARG_DOUBLE};

/*Basic struct stores option information*/
typedef struct s_optinfo t_optinfo;
struct s_optinfo
{
  char* name; /*The name of option*/
  char* val;  /*The value*/
  enum opt_with_val with_val;
  enum opt_val_type val_type; 
  enum opt_manda mandatory;
  enum opt_default opt_def;
};

/*
 * Item offsets in Opt_list
 */
enum opt_offset
{OP_BLIF,
 OP_MPACKBLIF,
 OP_NET,
 OP_BLEARCH,
 OP_VPRARCH,
 OP_STATS,
 OP_TIMING_ANALYSIS,
 OP_ALPHA,
 OP_BETA,
 OP_READ_STDLIB_BLIF,
 OP_PATTERN_MEMBER_TYPE,
 //OP_INNER_CLUSTER_DELAY,
 OP_INTER_CLUSTER_DELAY,
 OP_PACKER_ALGORITHM,
 OP_PACKER_AREA_ATTRACTION,
 OP_FORBID_UNRELATED_CLUSTERING,
 OP_VERBOSE,
 OP_HELP,
 OP_END};
