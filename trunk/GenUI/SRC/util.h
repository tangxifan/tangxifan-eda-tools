/**
 * Filename : util.h
 * Author : Xifan TANG, EPFL
 * Description : Define most useful types for general purpose
 */

/*DEBUG MODE*/
#define DEBUG 1

#define IN
#define OUT
#define INOUT

/*Boolean enum, C does not support*/
typedef enum e_bool bool;
enum e_bool
{TRUE,FALSE};

/*****Subroutines*****/
void* my_malloc(size_t size);

char* my_strdup(const char* str);


