/**
 * Filename : Options.h
 * Author : Xifan TANG, EPFL
 * Description : Header file contains structs and enumeration 
 *               types for option reading purpose.
 */

/*Determine whether it is a mandatory option*/
enum opt_manda
{MAND,NONMAND};

/*The option has been appeared in the command line */
enum opt_default
{DEF,NONDEF};

/*Determine whether the option contains a value*/
enum opt_with_val
{NONVAL,WITHVAL};

/*Determine the date type of value*/
enum opt_val_type
{INT,FLOAT,CHAR,DOUBLE};

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

/*Add any option by following the format of s_optinfo*/
t_optinfo opt_list[] = 
{
  {"net",0,WITHVAL,CHAR,MAND,DEF},
  {"rpd",0,WITHVAL,CHAR,MAND,DEF},
  {"height",0,WITHVAL,INT,NONMAND,DEF},
  {"width",0,WITHVAL,INT,NONMAND,DEF},
  {"ratio",0,WITHVAL,FLOAT,NONMAND,DEF},
  {"fixed_place",0,NONVAL,INT,NONMAND,DEF},
  {NULL,0,NONVAL,INT,NONMAND,DEF}
}; 





