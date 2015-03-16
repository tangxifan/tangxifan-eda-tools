#include <stdio.h>
#include <string.h>
#include <assert.h> 
#include "util.h"

/*****Subroutines*****/
int read_options(IN int argc,
                 IN char **argv);

int main(int argc,char **argv)
{
  
  read_options(argc,argv);
  
  return 1;
}
