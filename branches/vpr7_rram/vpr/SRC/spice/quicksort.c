/***********************************/
/*      SPICE Modeling for VPR     */
/*       Xifan TANG, EPFL/LSI      */
/***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>

#include <util.h>

/* Subroutines */
/* Swap two elements of sort_index */
static 
void swap_int(int* int_a, int* int_b) {
  int temp = (*int_a);

  (*int_a) = (*int_b);
  (*int_b) = temp;
  
  return;
}

static 
void swap_float(float* int_a, float* int_b) {
  float temp = (*int_a);

  (*int_a) = (*int_b);
  (*int_b) = temp;
  
  return;
}

/* Partition for quicksort
 * Given a array(sort_value) , and the index (sort_index)
 * Create two arrays, A and B,  whose length is start_index and len-start_index,
 * Move all the elements that are less than sort_value[start_index] to  arrayA [0:start_index]
 * Move all the elements that are larger than sort_value[start_index] to arrayB [0:len-start_index]
 */
static 
int partition(int len, int* sort_index, 
              float* sort_value, int pivot_index) {
  int i;
  int small_len = 0;
  float pivot_val = sort_value[pivot_index];

  /* Move the pivot to the last position */
  swap_int(&sort_index[pivot_index], &sort_index[len - 1]);
  swap_float(&sort_value[pivot_index], &sort_value[len - 1]);

  /* Move all the elements smaller than sort_value[start_index] to the first partition */
  for (i = 0; i < len-1; i++) {
    if ((sort_value[i] < pivot_val) 
       ||(sort_value[i] == pivot_val)) {
      swap_int(&sort_index[i], &sort_index[small_len]);
      swap_float(&sort_value[i], &sort_value[small_len]);
      small_len++;
    } 
  }
  /* Move the pivot to the critical position */
  swap_int(&sort_index[small_len], &sort_index[len - 1]);
  swap_float(&sort_value[small_len], &sort_value[len - 1]);

  return small_len;
}

void quicksort_float_index(int len, 
                           int* sort_index,
                           float* sort_value) {
  int small_len = 0; 
  //int pivot_val = 0;
  //int pivot_index = 0;

  if ((0 == len)||(1 == len)) {
    return;
  }

  /* get partition */
  small_len = partition(len, sort_index, sort_value, 0);
  /* Check */
  //assert(pivot_val == sort_value[small_len]);
  //assert(pivot_index == sort_index[small_len]);
  /* Recursive */
  quicksort_float_index(small_len, sort_index, sort_value);
  quicksort_float_index(len - small_len - 1, sort_index + small_len + 1, sort_value+small_len + 1);

  return;
}


