// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon

// Matrix architecture management library: part of the mapper


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "mpack.h"
#include "access.h"
#include "matrix_architecture.h"

///////////////////////////////////////////////////////
// TOPOLOGIES
///////////////////////////////////////////////////////

void create_omega_interconnect(int ** matrix, int width, int depth)
{
   int i,j;
   int base_idx;

   // Connect the input layer
   for(i=0;i<(width*GATE_MAX_INPUT_NB);i++)
   {
      *matrix_access(matrix,i,(i/GATE_MAX_INPUT_NB)+width*GATE_MAX_INPUT_NB)=1; // Each input node is connected to the associated logic net 
   }

   base_idx=width*GATE_MAX_INPUT_NB;
   for(i=0;i<depth-1;i++) // For each interconnect level
   {
      *matrix_access(matrix,base_idx,base_idx+width)=1; // Left direct connection

      for(j=0; j<width-1;j++) // Connections to the right
      {
         *matrix_access(matrix,base_idx+j,base_idx+j+width+1)=1;
      }

      for(j=1; j<width;j++) // Connections to the left
      {
         *matrix_access(matrix,base_idx+j,base_idx+j+width-1)=1;
      }

      *matrix_access(matrix,base_idx+(width-1),base_idx+2*width-1)=1; // Right direct connection

      base_idx+=width; // Increment for the next layer
   }

   // Connect the output layer
   for(i=width*(GATE_MAX_INPUT_NB+(depth-1));i<width*(GATE_MAX_INPUT_NB+depth);i++)
   {
      *matrix_access(matrix,i,i+width)=1; // Each input node is connected to the associated logic net 
   }
}

void terminal_generation_omega_interconnect(int ** matrix, int width, int depth)
{
/// Needs to be adapted to support multiple output cells  
   int i,j;
   int base_idx;

   // Input layer has no predecessors
   for(i=0;i<(width*GATE_MAX_INPUT_NB);i++)
   {
     *matrix_access(matrix,i,0)= i;
     *matrix_access(matrix,i,1)= -1;
     *matrix_access(matrix,i,2)= -1;
   }

   for(i=0;i<(width);i++) // First layer has all the inputs nodes as predecessors
   {
     *matrix_access(matrix,(width*GATE_MAX_INPUT_NB)+i,0)= (width*GATE_MAX_INPUT_NB)+i;
     *matrix_access(matrix,(width*GATE_MAX_INPUT_NB)+i,1)= (i*GATE_MAX_INPUT_NB);
     *matrix_access(matrix,(width*GATE_MAX_INPUT_NB)+i,2)= (i*GATE_MAX_INPUT_NB)+1;
   }   

   base_idx=width*(GATE_MAX_INPUT_NB+1);
   for(i=0;i<depth-1;i++) // For each interconnect level starting at the second one
   {
     for(j=0; j<width;j++) // Update the node id
     {
        *matrix_access(matrix,base_idx+j,0)=base_idx+j;
     }
     
     *matrix_access(matrix,base_idx,1)=base_idx-width; // Left direct connection
 
     for(j=0; j<width-1;j++) // Connections to the left
     {
        *matrix_access(matrix,base_idx+j,2)=base_idx+j-width+1;
     }
 
     for(j=1; j<width;j++) // Connections to the right
     {
        *matrix_access(matrix,base_idx+j,1)=base_idx+j-width-1;
     }

       *matrix_access(matrix,base_idx+(width-1),2)=base_idx-1; // Right direct connection

      base_idx+=width; // Increment for the next layer
   }

    // Connect the output layer
    for(i=base_idx;i<width*(GATE_MAX_INPUT_NB+depth+1);i++)
    {
      *matrix_access(matrix,i,0)= i;
      *matrix_access(matrix,i,1)= i-width;
      *matrix_access(matrix,i,2)= -1;
    }
}

///////////////////////////////////////////////////////
// DATA MANAGEMENT
///////////////////////////////////////////////////////

void init_architecture(matrix *arch)
{
   arch->topo=NO;
   arch->width=0;
   arch->depth=0;
   arch->adjacency_matrix=NULL;
   arch->levels=NULL;
   arch->terminal_interconnect_matrix=NULL;
}

void free_architecture(matrix *arch)
{
   int i,matrix_size;

   if(arch==NULL) return;
 
   matrix_size= arch->width*(arch->depth+GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB);

   for(i=0;i<matrix_size;i++)
   {
      if(*(arch->adjacency_matrix+i)) free(*(arch->adjacency_matrix+i));
   }

   delete_full_connectivity_matrix(arch->fconm, arch->adjm_size);
   
   for(i=0;i<arch->adjm_size;i++)
   {
      if(*(arch->terminal_interconnect_matrix+i)) free(*(arch->terminal_interconnect_matrix+i));
   }

   free(arch->adjacency_matrix);
   free(arch->terminal_interconnect_matrix);
   free(arch->levels);
}


///////////////////////////////////////////////////////
// DOT GENERATION
///////////////////////////////////////////////////////

// Dump the matrix architecture structure in DOT file format
void dot_print_architecture(matrix *arch)
{
   FILE * dotptr;
   int i,j;
   int node_idx=0;
   int matrix_size;

   int x_idx;
   int y_idx;
   int node_size=100;
   int x_origin=50;
   int y_origin=50;

   if( (dotptr = fopen("./WORK/output_arch.dot", "w")) == NULL)
   {
      printf("Error creating %s \n","output_arch.dot");
      exit(EXIT_FAILURE);
   }

   // Head part
   fprintf(dotptr,"digraph G {\n");

   // Print the nodes
   // Input nodes
   x_idx=x_origin; y_idx=y_origin;
   for(i=0;i<arch->width*GATE_MAX_INPUT_NB;i++)
   {
      fprintf(dotptr,"\tN%d [shape=box, color=green, style=filled, label=\"i%d\", pos=\"%d,%d!\"];\n",node_idx,i,y_idx,x_idx); // Print the subblock
      y_idx+=node_size;
      node_idx++;
   }

   // Function nodes
   for(j=0;j<arch->depth;j++)
   {
      x_idx-=node_size; y_idx=y_origin+node_size/2;

      for(i=0;i<arch->width;i++)
      {
         fprintf(dotptr,"\tN%d [shape=box, color=yellow, style=filled, label=\"f%d%d\", pos=\"%d,%d!\"];\n",node_idx,j,i,y_idx,x_idx); // Print the subblock
         y_idx+=(2*node_size);
         node_idx++;
      }
   }

   //Input nodes
   x_idx-=node_size; y_idx=y_origin+node_size/2;
   for(i=0;i<arch->width*GATE_MAX_OUTPUT_NB;i++)
   {
      fprintf(dotptr,"\tN%d [shape=box, color=red, style=filled, label=\"o%d\", pos=\"%d,%d!\"];\n",node_idx,i,y_idx,x_idx); // Print the subblock
      y_idx+=(2*node_size);	
      node_idx++;
   }

   // Print the connections
   matrix_size= arch->width*(arch->depth+GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB);
   for(i=0;i<matrix_size;i++)
   {
      for(j=0;j<matrix_size;j++)
      {
         if(*matrix_access(arch->adjacency_matrix,i,j)==1) fprintf(dotptr,"\tN%d -> N%d;\n",i, j);
      }
   }

   fprintf(dotptr,"}\n");

   fclose(dotptr);
}

///////////////////////////////////////////////////////
// UTILITY FUNCTIONS
///////////////////////////////////////////////////////

int number_of_outputs(matrix *arch)
{
   return GATE_MAX_OUTPUT_NB*arch->width;
}

int number_of_inputs(matrix *arch)
{
   return GATE_MAX_INPUT_NB*arch->width;
}

int number_of_nodes(matrix *arch)
{
   return arch->depth*arch->width;
}

int number_of_nets(matrix * arch)
{
   return (GATE_MAX_INPUT_NB*arch->width*(arch->depth+1));
}



///////////////////////////////////////////////////////
// MAIN ROUTINE
///////////////////////////////////////////////////////

void create_architecture(matrix *arch, enum interconnect_topology topo, int width, int depth)
{
   int i, j, idx, level;
   int matrix_size;

   printf("\nARCHITECTURE - topo:%d - width:%d - depth:%d\n",topo,width,depth);

   arch->topo=topo;
   arch->width=width;
   arch->depth=depth;
   matrix_size= width*(depth+GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB);
   arch->adjm_size=matrix_size;
   
    // Allocate all the cells
   arch->adjacency_matrix=(int**)malloc(matrix_size*sizeof(int*));
   for(i=0;i<matrix_size;i++)
   {
      *(arch->adjacency_matrix+i)=(int*)malloc(matrix_size*sizeof(int));
      memset(*(arch->adjacency_matrix+i),0,matrix_size*sizeof(int));
   }
   
   // Allocate all the terminal  
    arch->terminal_interconnect_matrix=(int**) malloc(matrix_size*sizeof(int*));
    for(i=0;i<matrix_size;i++)
    {
       *(arch->terminal_interconnect_matrix+i)=(int*) malloc((GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB)*sizeof(int));
       memset(*(arch->terminal_interconnect_matrix+i),0,(GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB)*sizeof(int));
    }
 
   // Levels
   arch->levels=(int*)malloc(matrix_size*sizeof(int));

   idx=0; // Init indices
   level=0;

   for(i=0; i<width*GATE_MAX_INPUT_NB;i++) // Fill the input level
   {
      *(arch->levels+idx)=level;
      idx++;
   }

   for(i=0; i<depth;i++) // Fill the logic node levels
   {
      level++;
      for(j=0;j<width;j++)
      {
         *(arch->levels+idx)=level;
         idx++;
      }
   }

   level++;
   for(i=0; i< width*GATE_MAX_OUTPUT_NB; i++) // Fill the output level
   {
      *(arch->levels+idx)=level;
      idx++;
   }

   // Create the interconnect topologies
   switch(topo)
   {
      case OMEGA:
         create_omega_interconnect(arch->adjacency_matrix,width,depth);
         terminal_generation_omega_interconnect(arch->terminal_interconnect_matrix, width, depth);
         break;
//       case BANYAN:
//          break;
      default:
         break;
   }

   // Create the full connectivity matrix from the adjacency matrix 
   arch->fconm=alloc_and_fill_full_connectivity_matrix(arch->adjacency_matrix, matrix_size);

#ifdef VERBOSE_DEBUG
    printf("\nARCHITECTURE - adjacency_matrix\n");
    print_matrix(arch->adjacency_matrix, matrix_size, matrix_size);
    printf("\nARCHITECTURE - full_connectivity_matrix\n");
    print_matrix(arch->fconm, matrix_size, matrix_size);
    printf("\nARCHITECTURE - levels\n");
    print_table(arch->levels, matrix_size);
    printf("\nARCHITECTURE - terminal interconnect\n");
    print_matrix(arch->terminal_interconnect_matrix, matrix_size, GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB);
#endif


#ifdef DEBUG
   dot_print_architecture(arch);
#endif
}
