#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"
#include "Options.h"
#include "init_opts.h"
#include "read_opts.h"

/*****Subroutines declaration*****/
int check_opt_list();
extern void print_usage();

/*****Subroutines*****/

void print_usage();

t_optinfo* read_options(IN int argc,
                        IN char **argv)
{
  int iarg;
  char* curarg = NULL;
  
  printf("Processing Options...\n");

  /*Start from argv[1], the 1st argv is programme name*/
  //if (argc<6)
  //{
  //  printf("Error:Must input mandatory arguments!\n");
  //  print_usage();
  //  exit(1);
  //}
  
  for (iarg = 1; iarg < argc;)
  {
    curarg = argv[iarg]; 
    /*Process the option start with hyphone*/
    if (0 == strncmp("-",curarg,1))    
    {
       curarg += 1; /*Eliminate the '-'*/ 
       process_arg_opt(argv,&iarg,curarg,argc); 
       iarg += 1; /*Move on to the next argument*/
    }
    else
    {iarg++;}

  }

  if (DEBUG)
  {show_opt_list();}
  
  check_opt_list();

  printf("Process Options Completed!\n");

  return opt_list;
}

/**
 * Check the option list
 * 1. Mandatory options are filled
 */
int check_opt_list()
{
  /*Define pointor for opt_list*/
  t_optinfo* cur = opt_list;
  
  while(cur->name)
  {
    if ((ARG_MAND == cur->mandatory)&&(ARG_NONDEF == cur->opt_def))
    {
      printf("Error: Mandatory Option(%s) should be filled!\n",cur->name); 
      print_usage();
      exit(1);
    }
    cur++;
  }

  return 1;
}

/**
 * Read the integer in option list 
 * Do simple check and convert it from char to 
 * integer
 */

/**
 * Process the argument by comparing 
 * Store the options in struct
 */
int process_arg_opt(IN char** argv,
                    INOUT int* iarg,
                    IN char* curarg,
                    IN int argc)
{
  t_optinfo* cur = opt_list;
        

  while(cur->name)
  {
    if (DEBUG)
    {printf("DEBUG: curname=%s\n",cur->name);}

    /*Process Match Arguments*/
    if (0 == strcmp(curarg,cur->name))
    {
      /*Check if this option has been defined in previous arguments*/
      if (ARG_DEF == cur->opt_def)
      {
        printf("Warning: Option(%s) redefined in arguments! It is overwritten!\n",cur->name);
      }
      /*A value is stored in next argument*/
      if (ARG_WITHVAL == cur->with_val)
      {
        *(iarg) += 1;
        // Check iarg vaild in the arguments
        if (*(iarg) < argc)
        {
          cur->val = my_strdup((argv[*iarg]));    
          /*Mark the option has been defined in argv*/
          cur->opt_def = ARG_DEF; 
          return 1;
        }
        printf("Error: Expected argument after %s!\n",cur->name); 
        print_usage();
        exit(1);
      }
      /*Do not need next argument, return*/
      else if (ARG_NONVAL == cur->with_val)
      {
        cur->val = "on";
        /*Mark the option has been defined in argv*/
        cur->opt_def = ARG_DEF; 
        return 1;
      }
      else
      {
        printf("Error:Unknown type of Option With_Val!Abort.\n");
        exit(1);
      }
    }
    cur++;
  }
  
  printf("Warning: Unknown Argument(%s) detected!Ignore it.\n",curarg);

  return 0;
}


/**
 * Show the options in opt_list after process.
 * Only for debug use
 */
int show_opt_list()
{
  /*Define pointor for opt_list*/
  t_optinfo* cur = opt_list;

  printf("DEBUG: Show opt_list\n");
  
  while(cur->name)
  {
    printf("DEBUG: Name=%s,Value=%s,DefInArg=%d.\n",cur->name,cur->val,cur->opt_def);
    cur++;
  }
  printf("DEBUG: END Show opt_list\n");

  return 1;
}


