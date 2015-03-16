// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon
// Mapper module: Logic function management - graph representation - function formatting

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "function.h"
#include "access.h"
#include "globals.h"
#include "mpack.h"

///////////////////////////////////////////////////////
// DATA MANAGEMENT
///////////////////////////////////////////////////////

// Initialize the function data structures
void init_function (function * fct)
{
   fct->nb_nodes=0;
   fct->nb_input=0;
   fct->nb_output=0;
   fct->nb_buf=0;
   fct->msize=0;
   fct->levels=NULL;
   fct->original_netlist_idx=NULL;
   fct->adjm=NULL;
   fct->fconm=NULL;
   fct->names=NULL;
}

// Clean the function data structures
void free_function (function * fct)
{
   int i;

   if(fct==NULL) return;

   if(fct->original_netlist_idx!=NULL)
      free(fct->original_netlist_idx);

   if(fct->levels!=NULL)
      free(fct->levels);

   if(fct->adjm!=NULL)
   {
      for(i=0;i<fct->msize;i++)
         if(*(fct->adjm+i)!=NULL) free(*(fct->adjm+i));
      free(fct->adjm);
   }

   if(fct->fconm!=NULL)
   {
      for(i=0;i<fct->msize;i++)
         if(*(fct->fconm+i)!=NULL) free(*(fct->fconm+i));
      free(fct->fconm);
   }

   if(fct->names!=NULL)
   {
      for(i=0; i<fct->msize; i++)
         free(fct->names[i]);
      free(fct->names);
   }
}

// Copy two function data structures
void copy_function (function * fctd, function * fcts)
{
   int i;

   fctd->nb_nodes=fcts->nb_nodes;
   fctd->nb_input=fcts->nb_input;
   fctd->nb_output=fcts->nb_output;
   fctd->msize=fcts->msize;

   if(fcts->levels!=NULL)
   {
      fctd->levels=(int*)malloc(fcts->msize*sizeof(int));
      memcpy(fctd->levels, fcts->levels, fcts->msize*sizeof(int));
   }

   if(fcts->original_netlist_idx!=NULL)
   {
      fctd->original_netlist_idx=(int*)malloc(fcts->msize*sizeof(int));
      memcpy(fctd->original_netlist_idx, fcts->original_netlist_idx, fcts->msize*sizeof(int));
   }

   if(fcts->adjm!=NULL)
   {
      fctd->adjm=(int**)malloc(fcts->msize*sizeof(int*));

      for(i=0; i<fcts->msize; i++)
      {
         *(fctd->adjm+i)=(int*)malloc(fcts->msize*sizeof(int));
         memcpy(*(fctd->adjm+i), *(fcts->adjm+i), fcts->msize*sizeof(int));
      }
   }

   if(fcts->fconm!=NULL)
   {
      fctd->fconm=(int**)malloc(fcts->msize*sizeof(int*));

      for(i=0; i<fcts->msize; i++)
      {
         *(fctd->fconm+i)=(int*)malloc(fcts->msize*sizeof(int));
         memcpy(*(fctd->fconm+i), *(fcts->fconm+i), fcts->msize*sizeof(int));
      }
   }

   if(fcts->names!=NULL)
   {
      fctd->names=(char**)malloc(fcts->msize*sizeof(char*));

      for(i=0; i<fcts->msize; i++)
      {
         *(fctd->names+i)=(char*)malloc((strlen(*(fcts->names+i))+1)*sizeof(int));
         strcpy(*(fctd->names+i),*(fcts->names+i));
      }
   }
}

///////////////////////////////////////////////////////
// PRIMITIVES
///////////////////////////////////////////////////////

// Recursive procedure for automatic logic depth identification
// From a seed, browse the function graph and assign level values to the encountered nodes
int filling_routine(function *fct, int *node_ticked, int idx, int level)
{
   int i;

   fct->levels[idx]=level; // Assign the given level to the current seed idx
   node_ticked[idx]=1; // Mark the current seed as already browsed

   for(i=0; i<fct->msize; i++) // For all the nodes
   {
      if(*matrix_access(fct->fconm, idx, i) == 1) // The node connected to the current seed (either downstream or upstream)
      {
         if(*matrix_access(fct->adjm, idx, i) ==1) // Downstream connection
         {
            if(node_ticked[i]==0 || fct->levels[i]<=fct->levels[idx])
               filling_routine(fct, node_ticked, i, fct->levels[idx]+1); // Recursive downstream, i.e., level+1, call
         }
         else // Upstream connection
            if(node_ticked[i]==0 || fct->levels[i]>=fct->levels[idx])
               filling_routine(fct, node_ticked, i, fct->levels[idx]-1); // Recursive upstream, i.e., level-1, call
      }
   }

   return 0;
}

// Browse the function and assign the logic levels (logic depth)
void autofill_level (function *fct)
{
   int *node_ticked=NULL;
   int sum=0, min=0,i=0;

   // Init level structures
  fct->levels=(int*)malloc(fct->msize*sizeof(int));
  for(i=0; i<fct->msize; i++)
    *(fct->levels+i)=-1;

   // Init autofilling structures
   node_ticked=(int*)malloc(fct->msize*sizeof(int));
   for(i=0; i<fct->msize; i++)
     *(node_ticked+i)=0;

   // For all the elements in the function (including nodes, inputs, outputs and buffers)
   // This loops is here to handle the multi-independant function case
   do
   {
      // Detect the next seed for the level attribution
      i=0;
      while(i<fct->msize && *(node_ticked+i)==1) i++; // First unticked node

      filling_routine(fct, node_ticked, i,0); // Start the filling with a level 0

      // Check if all nodes have been ticked
      sum=0;
      for(i=0; i<fct->msize; i++)
         sum+=*(node_ticked+i);

   }while(sum<fct->msize);

   // Here, the levels might be either positive or negative
   // Correction of the levels
   // Compensation of negative indix
   min=fct->levels[0];
   for(i=0; i<fct->msize; i++) // Identify the minimal indix
   {
         if(fct->levels[i]<min)
           min=fct->levels[i];
   }

   for(i=0; i<fct->msize; i++) // Correct the different levels
   {
         fct->levels[i]=fct->levels[i]-min;
   }

   // Correct the position of input and outputs
   input_level_correction(fct);
   output_level_correction(fct);

   sort_by_level(fct);

   if(node_ticked!=NULL)
     free(node_ticked);
}

// Update the different elements count by parsing the current function graph
void update_node_count(function *fct)
{
   int i;

   int olevel = max_in_table(fct->levels,fct->msize);

   fct->nb_input=0;
   fct->nb_output=0;
   fct->nb_nodes=0;
   fct->nb_buf=0;

   for(i=0; i<fct->msize; i++)
   {
      if( *(fct->levels +i) == 0) fct->nb_input++;
      else
      {
         if( *(fct->levels +i) == olevel) fct->nb_output++;
         else
         {
            fct->nb_nodes++;
            if(strcmp(*(fct->names+i),"buf")==0) fct->nb_buf++;
         }
      }
   }
}

// Add a node (a buffer) to the function fct at a given level
// If register_buf is 1, the buffer will be added to the blocks
int add_node(function *fct, int level, int register_buf)
{
   int i;
   int idx=0;

   // Find the idx to add the node
   for(i=0; i<fct->msize; i++) // Final node on the wanted level
   {
      if(*(fct->levels+i) <= level) // Identify the last node on the given level
        idx=i;
   }
   idx++;

   // Add an entry to the adjacency matrix
   fct->msize = matrix_add_line_column(&fct->adjm, fct->msize, idx);
   
   // Add an entry to the level matrix
   table_add_case(&(fct->levels), fct->msize, idx);
   *(fct->levels+idx)= level;
   
   // Add an entry to the original netlist indix matrix
   table_add_case(&(fct->original_netlist_idx), fct->msize, idx);
   if(register_buf==1) // If we add a buffer, add it to the buffer count
   {   
       *(fct->original_netlist_idx+idx)= buffer_counting_current;
       buffer_counting_current++;
   }
   else
     *(fct->original_netlist_idx+idx)=-2;
       
   // Add an entry to the name matrix
   table_add_pchar(&(fct->names), fct->msize, idx);
   *(fct->names+idx)=malloc(sizeof("buf"));
   strcpy(*(fct->names+idx), "buf");
 
   // Update the count variables of the function
   update_node_count(fct);

   return idx;
}

///////////////////////////////////////////////////////
// DOT GENERATION 
///////////////////////////////////////////////////////

// Plot the function in dot format with automatically generated names
// Deprecated: prefer to use dot_print_function_with_name
void dot_print_function(function *fct, char name[])
{
   FILE * dotptr;
   int i,j;
   int input, output;
   int node_idx=0, input_idx=0, output_idx=0;

   if( (dotptr = fopen(name, "w")) == NULL)
   {
      printf("Error creating %s \n",name);
      exit(EXIT_FAILURE);
   }

   // Head part
   fprintf(dotptr,"digraph G {\n");

   // Print the nodes
   for(i=0; i<fct->msize;i++) // For all the nodes
   {
      output=1; input=1;
      for(j=0;j<fct->msize;j++) // Check the connection
      {
         if( *matrix_access(fct->adjm, i, j)== 1) output=0;
         if( *matrix_access(fct->adjm, j, i)== 1) input=0;
      }

      if(input==1)
      {
         fprintf(dotptr,"\tN%d [shape=box, color=green, style=filled, label=\"i%d\"];\n", i, input_idx);
         input_idx++; 
      }

      if(output==1)
      {
         fprintf(dotptr,"\tN%d [shape=box, color=red, style=filled, label=\"o%d\"];\n",i,output_idx);
         output_idx++;
      }

      if(output==0 && input ==0)
      {
         fprintf(dotptr,"\tN%d [shape=box, color=yellow, style=filled, label=\"f%d\"];\n",i,node_idx);
         node_idx++;
      }
   }

   // Print the connections
   for(i=0;i<fct->msize;i++)
   {
      for(j=0;j<fct->msize;j++)
      {
         if(*matrix_access(fct->adjm,i,j)==1) fprintf(dotptr,"\tN%d -> N%d;\n",i, j);
      }
   }

   fprintf(dotptr,"}\n");

   fclose(dotptr);
}

// Plot the function in dot format with block names
void dot_print_function_with_name(function *fct, char name[])
{
   FILE * dotptr;
   int i,j;
   int input, output;

   if( (dotptr = fopen(name, "w")) == NULL)
   {
      printf("Error creating %s \n",name);
      exit(EXIT_FAILURE);
   }

   // Head part
   fprintf(dotptr,"digraph G {\n");

   // Print the nodes
   for(i=0; i<fct->msize;i++) // For all the nodes
   {
      output=1; input=1;
      for(j=0;j<fct->msize;j++) // Check the connection
      {
         if( *matrix_access(fct->adjm, i, j)== 1) output=0;
         if( *matrix_access(fct->adjm, j, i)== 1) input=0;
      }

      if(input==1)
      {
         fprintf(dotptr,"\tN%d [shape=box, color=green, style=filled, label=\"%s\"];\n", i, fct->names[i]);

      }

      if(output==1)
      {
         fprintf(dotptr,"\tN%d [shape=box, color=red, style=filled, label=\"%s\"];\n",i,fct->names[i]);

      }

      if(output==0 && input ==0)
      {
         fprintf(dotptr,"\tN%d [shape=box, color=yellow, style=filled, label=\"%s\"];\n",i,fct->names[i]);

      }
   }

   // Print the connections
   for(i=0;i<fct->msize;i++)
   {
      for(j=0;j<fct->msize;j++)
      {
         if(*matrix_access(fct->adjm,i,j)==1) fprintf(dotptr,"\tN%d -> N%d;\n",i, j);
      }
   }

   fprintf(dotptr,"}\n");

   fclose(dotptr);
}

///////////////////////////////////////////////////////
// FUNCTION FORMATING
///////////////////////////////////////////////////////

// Put all the input nodes to the same initial level 0
void input_level_correction(function *fct)
{
   int i,j,input;

   // Inputs are characterized by no connections to them
   // The colum related to its number is then filled by 0

   for(i=0;i<fct->msize;i++) // For all nodes
   {
      input=1;
      for(j=0;j<fct->msize;j++) // Check the connection
      {
         if( *matrix_access(fct->adjm, j, i)== 1)
         {
            input=0; // it exists a connection, it is not an input
         }
      }

      if(input==1) // Check the level of the input
      {
         if( *(fct->levels+i) !=0) // The input is not on the correct level
         {
            *(fct->levels+i)=0;
         }
      }
   }
}

// Put all the output nodes to the same final level
void output_level_correction(function *fct)
{
   int i,j,output, add_layer;

   int output_level = max_in_table(fct->levels, fct->msize);

   // Outputs are characterized by no connections from them
   // The line related to its number is then filled by the maximum level value

   // Detect if another output layer is required
   add_layer=0;
   for(i=0;i<fct->msize;i++) // For all nodes
   {
      output=1;
      for(j=0;j<fct->msize;j++) // Check the connection
      {
         if( *matrix_access(fct->adjm, i, j)== 1)
            output=0; // An output is detected
      }

      if(output==0 && *(fct->levels+i) == output_level)
         add_layer=1;
   }

   output_level=output_level+add_layer;

   for(i=0;i<fct->msize;i++) // For all nodes
   {
      output=1;
      for(j=0;j<fct->msize;j++) // Check the connection
      {
         if( *matrix_access(fct->adjm, i, j)== 1)
            output=0; // An output is detected
      }

      if(output==1) // Check the level of the input
      {
         if( *(fct->levels+i) != output_level) // The input is not on the correct level
            *(fct->levels+i)= output_level;
      }
   }
}

// Delete the combinational loops on a single node internally to the matrix
// Combinational loops can be handled at the clustering level
void feedback_correction(function *fct)
{
   int i;
   int idx_in, idx_out;

   fct->fconm = delete_full_connectivity_matrix(fct->fconm, fct->msize);

   // Feedback is characterized by a 1 in the diagonal of the adjacency matrix
   // Feedbacks are not handled by the matrix structure, should be handled externaly by adding an input and an output 

   for(i=0;i<fct->msize;i++) // For all nodes
   {
      if( *matrix_access(fct->adjm, i, i)== 1) // Feedback detected
      {
         printf("WARNING: Feedback correction : correction of node %d\n",i);

         // First delete the feedback connection
         *matrix_access(fct->adjm, i, i)=0;

         // Then add an input and an output
         idx_in=add_node(fct, 0, 0); // The added nodes do not count as real buffers (unregistered buf)
         idx_out=add_node(fct, max_in_table(fct->levels, fct->msize), 0);

         // Do the connection
         *matrix_access(fct->adjm, idx_in, i+1) =1;
         *matrix_access(fct->adjm, i+1, idx_out) =1;
      }
   }

   if(fct->msize != (fct->nb_input + fct->nb_nodes + fct->nb_output) )
   {
      printf("ERROR feedback correction : wrong size\n");
      exit(EXIT_FAILURE);
   }

   fct->fconm=alloc_and_fill_full_connectivity_matrix(fct->adjm, fct->msize);
}

// Check if a connection exist within a layer
// Check if a connection exist upstream
void check_inter_level_connections(function * fct)
{
   int i, j;
   int level_i, level_j;

   for(i=0; i<fct->msize; i++)
   {
      for(j=0; j<fct->msize; j++)
      {
         if ( *matrix_access(fct->adjm, i, j) == 1) // We have a connection
         {
            // Check the levels
            level_i = *(fct->levels+i);
            level_j = *(fct->levels+j);
            if(level_i==level_j) // Connections among the same level
            {
               fprintf(stderr, "ERROR : Connections on the same level\n");
               exit(EXIT_FAILURE);
            }

            if(level_i>level_j)
            {
               fprintf(stderr, "ERROR : Upstream connections detected\n");
               exit(EXIT_FAILURE);
            }
         }
      }
   }
}

// Check if an output node has a unique node connected to it
// If not, add it to handle multiple outputs
void unique_output_connection_correction(function * fct)
{
   int i, j, idx;
   int level_output;

   int nb_conn=0;

   level_output=max_in_table(fct->levels, fct->msize);

   for(i=0; i<fct->msize; i++)
   {
      if( *(fct->levels+i) == level_output ) // We are parsing an output node
      {
         nb_conn=0;
         for(j=0; j<fct->msize; j++)
         {
            if ( *matrix_access(fct->adjm, i, j) == 1) nb_conn++; // We have a connection
         }
      }

      if(nb_conn>1)
      {
         printf("WARNING : Bad number of connections to an output\n");

         nb_conn=0;
         for(j=0; j<fct->msize; j++)
         {
            if ( *matrix_access(fct->adjm, i, j) == 1) nb_conn++;

            if(nb_conn > 1)
            {
               // Add a node on the next layer
               idx=add_node(fct, level_output, 0); // A new output node is added --> not a real buffer

               *matrix_access(fct->adjm, i, j) = 0;
               *matrix_access(fct->adjm, i, idx) = 1;
            }
         }
      }
   }
}

// Check if an input node is driving only a single node
void unique_input_connection_correction(function * fct)
{
   int i, j, idx;
   int input, conn;

   fct->fconm = delete_full_connectivity_matrix(fct->fconm, fct->msize);

   for(i=0; i<fct->msize;i++) // For all the nodes
   {
      input=1; conn=0;
      for(j=0;j<fct->msize;j++) // Find the inputs and count the number of connections comming from it
      {
         if( *matrix_access(fct->adjm, j, i)== 1) input=0;
         if( *matrix_access(fct->adjm, i, j)== 1) conn++;
      }

      if(input==1)
      {
         while(conn>=2)
         {
            // Add a node on the first layer
            idx=add_node(fct, 0, 0); // A new logical input does not count as a real buffer
            fct->names[idx]=(char*) realloc(fct->names[idx], (strlen(fct->names[i])+1)*sizeof(char)); strcpy(fct->names[idx],fct->names[i]);
            
            fct->original_netlist_idx[idx]=fct->original_netlist_idx[i];

            j=0;
            while(j<fct->msize && *matrix_access(fct->adjm, i, j)!=1) j++;


            *matrix_access(fct->adjm, i, j) = 0;
            *matrix_access(fct->adjm, idx, j) = 1;

            conn--;
         }
      }
   }

   fct->fconm=alloc_and_fill_full_connectivity_matrix(fct->adjm, fct->msize);

//    printf("UNIQUE INPUT CORRECTION : exit\n");
}

// Adapt the fan-out of a gate to the capacity of the matrix
void delete_multiple_connections_to_output(function * fct)
{
   int i, j, idx;
   int nb_output;

   fct->fconm = delete_full_connectivity_matrix(fct->fconm, fct->msize);

   for(i=0; i<fct->msize; i++)
   {
      //First, detect the number of output of the node
      nb_output=0;
      for(j=0; j<fct->msize; j++)
      {
         if ( *matrix_access(fct->adjm, i, j) == 1) nb_output++; // We have a connection
      }
      
      // The gate is connected to more than XX (here, 2 = GATE_MAX_INPUT_NB) other gates, the matrix cannot accomodate this fan-out
      /// TO DO: Adapt the conditions to a parameterizable number of inputs/outputs
      if(nb_output>GATE_MAX_INPUT_NB) // Split the outputs to multiple buffers to decrease the fan-out
      {
         // Add a node on the next layer
         idx=add_node(fct, *(fct->levels+i)+1, 1);

         // Modify the connections - the first connection is unchanged - all are then connected to the new node
         nb_output=0;
         for(j=0; j<fct->msize; j++)
         {
            if ( *matrix_access(fct->adjm, i, j) == 1)
            {
               nb_output++;
               if(nb_output>1)
               {
                  *matrix_access(fct->adjm, i, j) = 0;
                  *matrix_access(fct->adjm, idx, j) = 1;
               }
            } 
         }

         *matrix_access(fct->adjm, i, idx) = 1;

         output_level_correction(fct); // Re-update the levels of outputs
      }
   }

   fct->fconm=alloc_and_fill_full_connectivity_matrix(fct->adjm, fct->msize);
}

// Sort the nodes of the functions in the different data structures according to the current level of the nodes
void sort_by_level(function * fct)
{
   int i,j,k;
   int tmpi;
   char * tmpc;

   int change=0;

   fct->fconm = delete_full_connectivity_matrix(fct->fconm, fct->msize);

   do
   {
      change=0;
      for(i=1; i<fct->msize; i++)
      {
         if(fct->levels[i]<fct->levels[i-1]) // If we need to move the node
         {
            change=1;

            j=0;
            while(fct->levels[j]<=fct->levels[i] && j<fct->msize-1) j++;

            // Exchange nodes i and j
            // In levels
            tmpi = fct->levels[i];
            fct->levels[i]=fct->levels[j];
            fct->levels[j]=tmpi;

            // In original netlist
            tmpi = fct->original_netlist_idx[i];
            fct->original_netlist_idx[i]=fct->original_netlist_idx[j];
            fct->original_netlist_idx[j]=tmpi;

            // In adjm
            for(k=0; k<fct->msize; k++)
            {
               tmpi=*matrix_access(fct->adjm,k,i);
               *matrix_access(fct->adjm,k,i)=*matrix_access(fct->adjm,k,j);
               *matrix_access(fct->adjm,k,j)=tmpi;
            }

            for(k=0; k<fct->msize; k++)
            {
               tmpi=*matrix_access(fct->adjm,i,k);
               *matrix_access(fct->adjm,i,k)=*matrix_access(fct->adjm,j,k);
               *matrix_access(fct->adjm,j,k)=tmpi;
            }

            // In Names
            tmpc = fct->names[i];
            fct->names[i]=fct->names[j];
            fct->names[j]=tmpc;
         }
      }
   }while(change!=0);

   fct->fconm=alloc_and_fill_full_connectivity_matrix(fct->adjm, fct->msize);
}

// If a connection goes accross one layer, add a buffer on the inner layers to accomodate the matrix organization
void jump_correction(function * fct)
{
   int i, j, z;
   int level_i, level_j, jump;
   int idx, previous_idx;

   fct->fconm = delete_full_connectivity_matrix(fct->fconm, fct->msize);

   // For each connection, check if the level jump is only equal to 1
   for(i=0; i<fct->msize; i++)
   {
      for(j=0; j<fct->msize; j++)
      {
         if ( *matrix_access(fct->adjm, i, j) == 1)
         {
            level_i=*(fct->levels+i);
            level_j=*(fct->levels+j);

            jump = abs(level_j - level_i);

            if(jump == 0) // Should never happen if check_inter_level_connections has been applied before
            {
               fprintf(stderr, "Jump corr : %d conn to %d - jump=%d\n", i, j, jump);
               fprintf(stderr, "ERROR : incorrect jump\n");
               exit(EXIT_FAILURE);
            }

            if(jump>1)
            {
               *matrix_access(fct->adjm, i, j) = 0;

               previous_idx=i;
               for(z=1; z<jump; z++)
               {
                  // Add a node on the next layer
                  idx=add_node(fct, level_i+z, 1);
                  *matrix_access(fct->adjm, previous_idx, idx) = 1;
                  previous_idx=idx;
               }

               *matrix_access(fct->adjm, idx, j+jump-1) = 1;
            }
         }
      }
   }

   fct->fconm=alloc_and_fill_full_connectivity_matrix(fct->adjm, fct->msize);
}

// Extend the output lines (by adding buffers) up to the physical output of the matrix
void extended_output_level_correction(function * fct, int wlevel)
{
   int i;

   int level_output, wlevel_output;
   int level_size;

   int idx;

   int tmpi;
   char * tmp;

   fct->fconm = delete_full_connectivity_matrix(fct->fconm, fct->msize);

   level_output = max_in_table(fct->levels, fct->msize);

   wlevel_output=wlevel+1;

   while(level_output<wlevel_output) // If the outpout node is not on the final level
   {
      // For each output nodes, add a buffer to extend the levels
      level_size=fct->msize;
      for(i=0; i<level_size; i++)
      {
         if( *(fct->levels+i) == level_output )
         {
            idx=add_node(fct, level_output+1, 1);

            *matrix_access(fct->adjm, i, idx) = 1;

            tmpi=fct->original_netlist_idx[idx];
            fct->original_netlist_idx[idx]=fct->original_netlist_idx[i];
            fct->original_netlist_idx[i]=tmpi;

            tmp=fct->names[idx];
            fct->names[idx]=fct->names[i];
            fct->names[i]=tmp;
         }
      }

      level_output = max_in_table(fct->levels, fct->msize);
   }

   fct->fconm=alloc_and_fill_full_connectivity_matrix(fct->adjm, fct->msize);
}

// Synthetic test function for debug purpose
void fill_test_fct1 (function * fct)
{
   int i,j;
  
   fct->nb_input=1;
   fct->nb_nodes=1;
   fct->nb_output=2;
   fct->msize=fct->nb_input+fct->nb_nodes+fct->nb_output;
   
   fct->adjm=(int**)malloc(fct->msize*sizeof(int*));
   for(i=0; i<fct->msize; i++) *(fct->adjm+i)=(int*) malloc(fct->msize*sizeof(int));

   for(i=0; i<fct->msize; i++)
     for(j=0; j<fct->msize; j++)
         *matrix_access(fct->adjm,i,j)=0;

   *matrix_access(fct->adjm,0,1)=1;
   *matrix_access(fct->adjm,1,2)=1;
   *matrix_access(fct->adjm,1,3)=1;
//    *matrix_access(fct->adjm,1,4)=1;   

   fct->fconm=alloc_and_fill_full_connectivity_matrix(fct->adjm,fct->msize);
}

///////////////////////////////////////////////////////
// ARCHITECTURE TO FUNCTION FORMATING
///////////////////////////////////////////////////////

// Adapt the function structure to the physical architecture
void function_formatting(function *fct, int max_depth)
{
 // Apply the formatting sequence
 input_level_correction(fct);
 unique_input_connection_correction(fct);
 feedback_correction(fct);
 check_inter_level_connections(fct);
 sort_by_level(fct);
   
 jump_correction(fct); 
 unique_output_connection_correction(fct);
 delete_multiple_connections_to_output(fct);
 extended_output_level_correction(fct, max_depth);
  
 output_level_correction(fct);
 update_node_count(fct);

#ifdef VERBOSE_DEBUG
 printf("\nFUNCTION FORMATTING : Adjacency matrix\n");
 print_matrix(fct->adjm, fct->msize, fct->msize);
 printf("\nFUNCTION FORMATTING : Original netlist\n");
 print_table(fct->original_netlist_idx, fct->msize);
 printf("\nFUNCTION FORMATTING : Names\n");
 print_char_table(fct->names, fct->msize);
 printf("\nFUNCTION FORMATTING : Levels\n");
 print_table(fct->levels, fct->msize);
#endif
}


