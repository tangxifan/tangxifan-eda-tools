#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"
#include "Options.h"
#include "read_opts.h"

int read_options(IN int argc,
                 IN char **argv)
{
  int iarg;
  char* curarg = NULL;
  
  printf("Processing Options...\n");

  /*Start from argv[1], the 1st argv is programme name*/
  for (iarg = 1; iarg < argc;)
  {
    curarg = argv[iarg]; 
    /*Process the option start with hyphone*/
    if (0 == strncmp("-",curarg,1))    
    {
       curarg += 1; /*Eliminate the '-'*/ 
       process_arg_opt(argv,&iarg,curarg); 
       iarg += 1; /*Move on to the next argument*/
    }
    else
    {iarg++;}

  }

  if (DEBUG)
  {show_opt_list();}

  printf("Process Options Completed!\n");

  return 1;
}

/**
 * Process the argument by comparing 
 * Store the options in struct
 */
int process_arg_opt(IN char** argv,
                    INOUT int* iarg,
                    IN char* curarg)
{
  t_optinfo* cur = opt_list;
        

  while(cur->name)
  {
    if (DEBUG)
    {printf("DEBUG: curname=%s\n",cur->name);}

    /*Process Match Arguments*/
    if (0 == strcmp(curarg,cur->name))
    {
      /*A value is stored in next argument*/
      if (WITHVAL == cur->with_val)
      {
        *(iarg) += 1;
        //cur->val = my_strdup(*(argv+(*iarg)));    
        cur->val = my_strdup((argv[*iarg]));    
        /*Mark the option has been defined in argv*/
        cur->opt_def = NONDEF; 
        return 1;
      }
      /*Do not need next argument, return*/
      else if (NONVAL == cur->with_val)
      {
        cur->val = "on";
        /*Mark the option has been defined in argv*/
        cur->opt_def = NONDEF; 
        return 1;
      }
      else
      {
        printf("Error Unknown type of Option With_Val!Abort.\n");
        exit(1);
      }
    }
    cur++;
  }
  
  printf("Warning: Unknown Option(%s) detected!Ignore it.\n",curarg);

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

/**
 * Read the integer in option list 
 * Do simple check and convert it from char to 
 * integer
 */
int process_int_arg(IN char* arg,
                    OUT int* val)
{
  /*Check if the pointor of arg is NULL */
  if (NULL == arg)
  {
    printf("Error: Arg is NULL when processing integer!\n");
    exit(1);
  } 

  *val = atoi(arg);

  return 1;
}

/**
 * Read the float in option list 
 * Do simple check and convert it from char to 
 * float
 */
int process_float_arg(IN char* arg,
                      OUT float* val)
{
  /*Check if the pointor of arg is NULL */
  if (NULL == arg)
  {
    printf("Error: Arg is NULL when processing float!\n");
    exit(1);
  }

  *val = atof(arg);

  return 1;
}
