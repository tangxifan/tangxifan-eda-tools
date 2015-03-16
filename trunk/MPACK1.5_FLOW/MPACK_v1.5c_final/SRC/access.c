// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon
// Accessibility and debug tools library

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "access.h"

///////////////////////////////////////////////////////
// MATRIX ACCESSIBILITY FUNCTIONS
///////////////////////////////////////////////////////

// Direct access to a given case with a 2D table
int* matrix_access(int ** matrix, int depth_indix, int width_indix)
{
   return (*(matrix+depth_indix)+width_indix);
}

// Add an entry (line and column) in 2D table at position idx
int matrix_add_line_column(int *** matrix, int msize, int idx)
{
   int i,j;

   // Update the size
   msize++;

   // Reallocate the memory
   *matrix=(int **) realloc(*matrix, msize * sizeof(int*));
   // Init the new pointer
   *(*matrix+msize-1)=NULL;

   for(i=0; i<msize; i++)
      *(*matrix+i)=(int*) realloc(*(*matrix+i), msize * sizeof(int));

   for(i=0; i<msize; i++)
   {
      *matrix_access(*matrix, msize-1, i) = -1;
      *matrix_access(*matrix, i, msize-1) = -1;
   }

   // Move the datas after the new indix
   for(i=0; i<msize-1; i++) // For columns
      for(j=msize-2; j>=idx; j--)
         *matrix_access(*matrix, i, j+1) = *matrix_access(*matrix, i, j);

   for(i=msize-2; i>=idx; i--) // For lines
      for(j=0; j<msize; j++)
         *matrix_access(*matrix, i+1, j) = *matrix_access(*matrix, i, j);

   // Init the new node
   for(i=0;i<msize;i++)
   {
      *matrix_access(*matrix, i, idx) = 0; // column
      *matrix_access(*matrix, idx, i) = 0; // line
   }

   return msize;
}

// Remove an entry (line and column) in 2D table at position idx
int matrix_remove_line_column(int *** matrix, int msize, int idx)
{
   int i,j;

   // Move the datas after the new indix
   for(i=0; i<msize; i++) // For columns
      for(j=idx; j<msize-1; j++)
         *matrix_access(*matrix, i, j) = *matrix_access(*matrix, i, j+1);

   for(i=idx; i<msize-1; i++) // For lines
      for(j=0; j<msize; j++)
         *matrix_access(*matrix, i, j) = *matrix_access(*matrix, i+1, j);

   // Reallocate the memory
   for(i=0; i<msize; i++)
      *(*matrix+i)=(int*) realloc(*(*matrix+i), (msize-1) * sizeof(int));
   free(*(*matrix+msize-1));
   *matrix=(int **) realloc(*matrix, (msize-1) * sizeof(int*));

   // Update the size
   msize--;

   return msize;
}

// Delete a full connectivity matrix
int ** delete_full_connectivity_matrix(int ** fconm, int size)
{
   int i;

   if(fconm==NULL)
     return NULL;

   for(i=0; i<size; i++)
      if(*(fconm+i) != NULL)
         free(*(fconm+i));

   free(fconm);
   return NULL;
}

// Allocate and fill a full connectivity matrix from an adjacency matrix
int ** alloc_and_fill_full_connectivity_matrix(int ** fadjm, int size)
{
   int i,j;
   int ** fconm = NULL;

   fconm=(int**)malloc(  size * sizeof(int*) );
   for(i=0;i<size;i++)
   {
      *(fconm+i)=(int*)malloc(size*sizeof(int));
      for(j=0; j<size; j++)
         *matrix_access(fconm,i,j)=0;
   }

   for(i=0;i<size;i++)
   {
      for(j=0;j<size;j++)
      {
         if(*matrix_access(fadjm,i,j)==1) // For an entry in the adjacency matrix, fill both directions
         {
            *matrix_access(fconm,i,j)=1;
            *matrix_access(fconm,j,i)=1;
         }
      }
   }

   return fconm;
}

///////////////////////////////////////////////////////
// TABLE ACCESSIBILITY FUNCTIONS
///////////////////////////////////////////////////////

// Add an entry in a 1D table
void table_add_case(int **tab, int size, int idx)
{
   int i;

   *tab=(int *)realloc(*tab, size*sizeof(int));
   *(*tab+(size-1))=-1;

   for(i=size-2; i>=idx; i--)
      *(*tab+i+1) = *(*tab+i);

   *(*tab+idx)=0;
}

// Remove an entry in a 1D table
int table_remove_case(int **tab, int size, int idx)
{
   int i;

   for(i=idx; i<size-1; i++)
      *(*tab+i)=*(*tab+i+1);

   size--;

   *tab=(int *)realloc(*tab, size*sizeof(int));

   return size;
}

// Add an entry in a pointer of char table
void table_add_pchar(char ***tab, int size, int idx)
{
   int i;

   *tab=(char **)realloc(*tab, size*sizeof(char*));
   *(*tab+(size-1))=NULL;

   for(i=size-2; i>=idx; i--)
      *(*tab+i+1) = *(*tab+i);

   *(*tab+idx)=NULL;
}

// Remove an entry in a pointer of char table
int table_remove_pchar(char ***tab, int size, int idx)
{
   int i;

   free(*(*tab+idx));

   for(i=idx; i<size-1; i++)
      *(*tab+i)=*(*tab+i+1);

   size--;

   *tab=(char **)realloc(*tab, size*sizeof(char*));

   return size;
}

// Clean and realloc unused case in a table
int clean_table(int **tab, int size)
{
   int i;
   int new_size=0;
   
   for(i=0; i<size; i++)
   {
      if(*(*tab+i) != -1)
      {
         *(*tab+new_size) = *(*tab+i);
         new_size++;
      }
   }

   *tab=(int*)realloc(*tab,new_size*sizeof(int));

   return new_size;
}

// Give the maximum integer value of a 1D table
int max_in_table(int * tab, int size)
{
   int i;
   int max = *tab; // Init max with the value of first block

   for(i=0; i<size; i++)
   {
      if(max< *(tab+i) ) max = *(tab+i);
   }

   return max;
}

///////////////////////////////////////////////////////
// DEBUG FUNCTIONS
///////////////////////////////////////////////////////

// Print a 2-D matrix
void print_matrix(int ** matrix, int nb_line, int nb_column)
{
   int i,j;

   printf("Print Matrix : %p, nb_line=%d, nb_column=%d\n", matrix, nb_line, nb_column);

   if(matrix==NULL) return;

   printf("     ");
   for(j=0; j<nb_column; j++)
   {
      printf("%2d  ",j);
   }
   printf("\n\n");

   for(i=0; i<nb_line; i++)
   {
      printf("%2d   ",i);
      for(j=0; j<nb_column; j++)
      {
         printf("%2d  ",*(*(matrix+i)+j));
      }
      printf("\n");
   }
}

// Print a 1-D table
void print_table(int * table, int size)
{
   int i;

   printf("Print Table : %p, size=%d\n", table, size);

   if (table==NULL) return;

   for(i=0; i<size; i++)
   {
      printf("%2d  ",i);
   }
   printf("\n");

   for(i=0; i<size; i++)
   {
      printf("%2d  ",*(table+i));
   }
   printf("\n");
}

// Print a 1-D char table
void print_char_table(char ** table, int size)
{
   int i;

   printf("Print Table : %p, size=%d\n", table, size);

   if (table==NULL) return;

   for(i=0; i<size; i++)
   {
      printf("%2d\t%s\t  ",i, *(table+i));
   }
   printf("\n");
}
