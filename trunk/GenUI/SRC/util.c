/**
 * Filename : util.c
 * Author : Xifan TANG, EPFL
 * Description : Define most useful functions for general purpose
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "util.h"

void* 
my_malloc(size_t size)
{
  void* ret;
   
  if ((ret = malloc(size)) == NULL)
  {
    printf("Error: Unable to malloc memeory.Aborting!\n");
    exit(1);
  }
  return (ret);
}

/*Allocate new char array for a string and Copy it to the new char array*/
char* 
my_strdup(const char* str)
{
  int len;
  char* des;

  len = 1 + strlen(str);
  des = (char*)my_malloc(len*sizeof(char));
  memcpy(des,str,len);

  return des;
}

