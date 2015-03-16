// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon
// Packer module: Maps a logic function graph on a matrix cluster

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "matrix_packer.h"
#include "access.h"
#include "mpack.h"
#include "globals.h"

///////////////////////////////////////////////////////
// DATA MANAGEMENT
///////////////////////////////////////////////////////

// Clean the mapping structure
void free_mapping(mapping *map)
{
   map->arch=NULL;
   map->function=NULL;

   if(map->map!=NULL) free(map->map);
}

// Allocate and initialize the mapping structures using the fct and arch informations
void init_mapping(mapping *map, matrix *arch, function * fct)
{
   int i;

   map->arch=arch;
   map->function=fct;

   map->map=(int*)malloc(arch->adjm_size*sizeof(int));

   for(i=0;i<arch->adjm_size;i++)
   {
      *(map->map+i)=-1;
   }
}

///////////////////////////////////////////////////////
// SORT FUNCTIONS
///////////////////////////////////////////////////////

// Function nodes depth first search recursive callback
int modified_depth_first_search(function * fct, int **list, int **node_ticked, int node)
{
   int i;
   int size = fct->msize; 
   static int node_parsed;

   if(*list==NULL) // Init structures
   {
      *list=(int*)malloc(size*sizeof(int));
      node_parsed=0;

      *node_ticked=(int*)malloc(size*sizeof(int));
      memset(*node_ticked,0,size*sizeof(int));
   }

   if(node==-1) // If new subgraph
   {
      i=0;
      while(node == -1 && i<size)
      {
         if(*(*node_ticked+i)==0) node = i; // Choose the new head
         else i++; 
      }
   }

   *(*list+node_parsed)=node; // Store the current head
   *(*node_ticked+node)=1; // Tick it

   node_parsed++; // Update the number of node parsed

   if(node_parsed>=size) return node_parsed; // If all nodes have been parsed

   for(i=0;i<size;i++) // Search all the connected nodes to the current head
   {
      if( *(*(fct->fconm+node)+i) == 1) // Connection detected
      {
         if( *(*node_ticked+i) == 0)
         {
            modified_depth_first_search(fct,list, node_ticked, i); // If the node have never been parsed, explore from it
         }
      }
   }

   return node_parsed; // If all nodes of the current subgraph have been parsed
}

// Function nodes sorting procedure, use a depth first search
void function_node_sort (function *fct, int ** list)
{
   int *node_ticked=NULL;
   int size = fct->msize;

   while(modified_depth_first_search(fct, list, &node_ticked,-1) < size); // Explore in all subgraphs

   if(node_ticked!=NULL)
     free(node_ticked);
}

///////////////////////////////////////////////////////
// DOT GENERATION
///////////////////////////////////////////////////////

// Plot the mapping results in dot format with automatically generated names - Print the assigned nodes on the matrix structure
// Deprecated: prefer to use dot_print_mapping_with_name
void dot_print_mapping(mapping *map, char filename[])
{
   FILE * mapptr;
   int i,j;
   int matrix_size;

   int node_idx=0; 
   int x_idx=0, y_idx=0;
   int node_size=100, x_origin=50, y_origin=50;

   matrix *arch=map->arch;

   if( (mapptr = fopen(filename, "w")) == NULL)
   {
      printf("Error creating %s \n",filename);
      exit(EXIT_FAILURE);
   }

   // Head part
   fprintf(mapptr,"digraph G {\n");

   // Print the nodes
   // Input nodes
   x_idx=x_origin; y_idx=y_origin;
   for(i=0;i<arch->width*GATE_MAX_INPUT_NB;i++)
   {
      if(*(map->map+node_idx)!=-1)
      {
         fprintf(mapptr,"\tN%d [shape=box, color=darkseagreen, style=filled, label=\"Mi%d\", pos=\"%d,%d!\"];\n",node_idx,*(map->map+node_idx),y_idx,x_idx); // Print the subblock
      }
      else
      {
         fprintf(mapptr,"\tN%d [shape=box, color=green, style=filled, label=\"i%d\", pos=\"%d,%d!\"];\n",node_idx,i,y_idx,x_idx); // Print the subblock
      }
      y_idx+=node_size;
      node_idx++;
   }

   // Function nodes
   for(j=0;j<arch->depth;j++)
   {
      x_idx-=node_size; y_idx=y_origin+node_size/2;

      for(i=0;i<arch->width;i++)
      {
         if(*(map->map+node_idx)!=-1)
         {
            fprintf(mapptr,"\tN%d [shape=box, color=violet, style=filled, label=\"Mf%d\", pos=\"%d,%d!\"];\n",node_idx,*(map->map+node_idx),y_idx,x_idx); // Print the subblock
         }
         else
         {
         fprintf(mapptr,"\tN%d [shape=box, color=yellow, style=filled, label=\"f%d%d\", pos=\"%d,%d!\"];\n",node_idx,j,i,y_idx,x_idx); // Print the subblock
         }
         y_idx+=(2*node_size);
         node_idx++;
      }
   }

   //Input nodes
   x_idx-=node_size; y_idx=y_origin+node_size/2;
   for(i=0;i<arch->width*GATE_MAX_OUTPUT_NB;i++)
   {
      if(*(map->map+node_idx)!=-1)
      {
         fprintf(mapptr,"\tN%d [shape=box, color=coral, style=filled, label=\"Mo%d\", pos=\"%d,%d!\"];\n",node_idx,*(map->map+node_idx),y_idx,x_idx); // Print the subblock
      }
      else
      {
         fprintf(mapptr,"\tN%d [shape=box, color=red, style=filled, label=\"o%d\", pos=\"%d,%d!\"];\n",node_idx,i,y_idx,x_idx); // Print the subblock
      }
      y_idx+=(2*node_size);	
      node_idx++;
   }

   // Print the connections
   matrix_size= arch->width*(arch->depth+GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB);
   for(i=0;i<matrix_size;i++)
   {
      for(j=0;j<matrix_size;j++)
      {
         if(*matrix_access(arch->adjacency_matrix,i,j)==1) fprintf(mapptr,"\tN%d -> N%d;\n",i, j);
      }
   }

   fprintf(mapptr,"}\n");

   fclose(mapptr);
}

// Plot the mapping results in dot format with block names - Print the assigned nodes on the matrix structure
void dot_print_mapping_with_name(mapping *map, char filename[])
{
   FILE * mapptr;
   int i,j;
   int matrix_size;

   int node_idx=0;
   int x_idx=0, y_idx=0;
   int node_size=100, x_origin=50, y_origin=50;

   matrix *arch=map->arch;
   char *name=NULL;

   if( (mapptr = fopen(filename, "w")) == NULL)
   {
      printf("Error creating %s \n",filename);
      exit(EXIT_FAILURE);
   }

   // Head part
   fprintf(mapptr,"digraph G {\n");

   // Print the nodes
   // Input nodes
   x_idx=x_origin; y_idx=y_origin;
   for(i=0;i<arch->width*GATE_MAX_INPUT_NB;i++)
   {
      if(*(map->map+node_idx)!=-1)
      {
         name=map->function->names[*(map->map+node_idx)];
         fprintf(mapptr,"\tN%d [shape=box, color=darkseagreen, style=filled, label=\"%s\", pos=\"%d,%d!\"];\n",node_idx,name,y_idx,x_idx); // Print the subblock
      }
      else
      {
         fprintf(mapptr,"\tN%d [shape=box, color=green, style=filled, label=\"i%d\", pos=\"%d,%d!\"];\n",node_idx,i,y_idx,x_idx); // Print the subblock
      }
      y_idx+=node_size;
      node_idx++;
   }

   // Function nodes
   for(j=0;j<arch->depth;j++)
   {
      x_idx-=node_size; y_idx=y_origin+node_size/2;

      for(i=0;i<arch->width;i++)
      {
         if(*(map->map+node_idx)!=-1)
         {
            name=map->function->names[*(map->map+node_idx)];
            fprintf(mapptr,"\tN%d [shape=box, color=violet, style=filled, label=\"%s\", pos=\"%d,%d!\"];\n",node_idx,name,y_idx,x_idx); // Print the subblock
         }
         else
         {
         fprintf(mapptr,"\tN%d [shape=box, color=yellow, style=filled, label=\"f%d%d\", pos=\"%d,%d!\"];\n",node_idx,j,i,y_idx,x_idx); // Print the subblock
         }
         y_idx+=(2*node_size);
         node_idx++;
      }
   }

   //Input nodes
   x_idx-=node_size; y_idx=y_origin+node_size/2;
   for(i=0;i<arch->width*GATE_MAX_OUTPUT_NB;i++)
   {
      if(*(map->map+node_idx)!=-1)
      {
         name=map->function->names[*(map->map+node_idx)];
         fprintf(mapptr,"\tN%d [shape=box, color=coral, style=filled, label=\"%s\", pos=\"%d,%d!\"];\n",node_idx,name,y_idx,x_idx); // Print the subblock
      }
      else
      {
         fprintf(mapptr,"\tN%d [shape=box, color=red, style=filled, label=\"o%d\", pos=\"%d,%d!\"];\n",node_idx,i,y_idx,x_idx); // Print the subblock
      }
      y_idx+=(2*node_size);	
      node_idx++;
   }

   // Print the connections
   matrix_size= arch->width*(arch->depth+GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB);
   for(i=0;i<matrix_size;i++)
   {
      for(j=0;j<matrix_size;j++)
      {
         if(*matrix_access(arch->adjacency_matrix,i,j)==1) fprintf(mapptr,"\tN%d -> N%d;\n",i, j);
      }
   }

   fprintf(mapptr,"}\n");

   fclose(mapptr);
}

///////////////////////////////////////////////////////
// MATRIX PACKING 
///////////////////////////////////////////////////////

// Identifies if the node pos is connected to all the nodes contained in connected_nodes_mapped
int are_connected(int **fconm, int *connected_nodes_mapped, int nb_connected_nodes_mapped, int pos)
{
   int i,tmp;

   for(i=0 ; i<nb_connected_nodes_mapped; i++) // For all connected nodes, is there a connection between the current pos ?
   {
      tmp=*(connected_nodes_mapped+i); // Indix of the current connected node
      if(*matrix_access(fconm,tmp,pos)==0)
         return 0;
   }

   return 1;
}

// Identifies the possible positions to map the current node
int find_free_place(mapping *map, int ** pos_list, int current)
{
   int i, j, tmp;

   function *fct=map->function;
   matrix *arch=map->arch;

   int *connected_nodes=NULL;
   int connected_nodes_size=0;

   int *connected_nodes_pos_on_arch=NULL;
   int connected_nodes_pos_on_arch_size=0;

   int nb_potential_nodes=0;

   int level_current_node = *(fct->levels+current);

    // Find arch nodes on the correct level
   for(i=0;i<arch->adjm_size;i++)
   {
      if( *(arch->levels+i) == level_current_node)
      {
         *pos_list = (int*) realloc(*pos_list, (nb_potential_nodes+1) * sizeof(int));
         *(*pos_list+nb_potential_nodes)=i;
         nb_potential_nodes++;
      }
   }
   if(nb_potential_nodes==0) // No free nodes
     return 0;

   // Remove the nodes that are already occupied
   for(i=0; i<nb_potential_nodes; i++)
   {
      tmp=*(*pos_list+i); // Indix of the current
      if( *(map->map+tmp) != -1 ) *(*pos_list+i) = -1; // Node already occupied 
   }
   nb_potential_nodes=clean_table(pos_list, nb_potential_nodes);
   if(nb_potential_nodes==0)  // No free nodes
   {
     if(connected_nodes!=NULL)
        free(connected_nodes);
     return 0;
   }
     
   // Nodes connected to current, i.e., identify the predecessors of the node to map
   for(i=0; i<fct->msize; i++)
   {
     if(*matrix_access(fct->fconm,current,i)==1)
      {
         connected_nodes = (int*) realloc(connected_nodes, (connected_nodes_size+1) * sizeof(int));
         *(connected_nodes+connected_nodes_size)=i;
         connected_nodes_size++;
      };
   }
   if(connected_nodes_size == 0) // The node doesn't have any predecessors, so all the candidates are valid
     return nb_potential_nodes;

    // The node has some predecessors, so we check the position of the already mapped nodes...
   for(i=0; i< connected_nodes_size; i++)
   {
      tmp=*(connected_nodes+i); // Indix of the current node
      for(j=0; j<arch->adjm_size; j++)
      {
         if( *(map->map+j) == tmp)
         {
            connected_nodes_pos_on_arch = (int*) realloc(connected_nodes_pos_on_arch, (connected_nodes_pos_on_arch_size+1) * sizeof(int));
            *(connected_nodes_pos_on_arch+connected_nodes_pos_on_arch_size)=j;
            connected_nodes_pos_on_arch_size++;
         }
      }
   }

   if(connected_nodes_pos_on_arch_size==0)
   {
      if(connected_nodes!=NULL)
        free(connected_nodes);
      if(connected_nodes_pos_on_arch!=NULL)
        free(connected_nodes_pos_on_arch);
      return nb_potential_nodes;
   } 
   
   // ... and we check what are the potential candidates that are physically connected to the predecessors
   for(i=0; i<nb_potential_nodes; i++)
   {
      if( are_connected(arch->fconm, connected_nodes_pos_on_arch, connected_nodes_pos_on_arch_size, *(*pos_list+i)) == 0)
         *(*pos_list+i)= -1;
   }
   nb_potential_nodes=clean_table(pos_list, nb_potential_nodes);

   if(connected_nodes!=NULL)
     free(connected_nodes);
   if(connected_nodes_pos_on_arch!=NULL)
     free(connected_nodes_pos_on_arch);

   return nb_potential_nodes;
}

// Recursive mapping callback: At each occurence, position a block at the first available position respective the predecessors.
int mapping_callback (mapping *map, int *exploration_list, int current_idx)
{
   int current;

   int i,exp_pos,ret,fsize;

   int *position_list=NULL;
   int nb_position=0;
   
   int *mapping_table=map->map;

   current_idx++; // Node to map : increment at each call

   fsize= map->function->msize;

   if(current_idx>=fsize)
     return 1; // No more nodes to map

   current = *(exploration_list+current_idx); // Expressed in function node

   nb_position = find_free_place(map, &position_list, current);

   if(nb_position == 0)
     return 0; // No solutions for the node 


   for(i=0;i<nb_position;i++)
   {
      exp_pos=*(position_list+i); // Expected position
      *(mapping_table+exp_pos) = current;

      ret =  mapping_callback(map, exploration_list, current_idx);

      if (ret == 0) // Mapping of following nodes unsuccessfull, unmount the solution
         *(mapping_table+exp_pos) = -1;
      else
      {
         if(position_list!=NULL)
           free(position_list);
         return 1; // Mapping successfull
      }
   }

   if(position_list!=NULL)
     free(position_list);

   if(ret==0)
   {
      current_idx--;
      return 0; // All positions have been checked and no solutions have been found
   }

   return 1; // Mapping of the node and of the following nodes successfull
}

///////////////////////////////////////////////////////
// MAIN ROUTINE 
// Try to map the function fct on the physical matrix arch
///////////////////////////////////////////////////////
int matrix_mapping(matrix *arch, function * fct, int *matrix_content, boolean echo_dot, char *filename)
{
   int i, idx;
   int result=0;
   int *exploration_list=NULL;
   
   // Mapping setup
   mapping *map = (mapping *) malloc(sizeof(mapping));
   init_mapping(map, arch, fct);

   // PRELIMINARY CHECK: Do we have enough levels available 
   if(max_in_table(fct->levels, fct->msize) != max_in_table(arch->levels, arch->adjm_size) )
   {
      if(map != NULL) // Clean the structures
      {
         free_mapping(map);
         free(map);
      }
      return 0;
   }

   // The Mapping operation is done in 2 steps:
   // 1- The nodes to map are sorted by a depth search of the function graph
   // This step gives the sequence for the nodes to map
   function_node_sort(fct, &exploration_list);

   // 2- The nodes are iteratively assigned to the matrix, by using a recurrence function
   result = mapping_callback(map, exploration_list, -1);

   if(result) // Mapping successful
   {
      // Transfer the content of the mapping structure to the current matrix_content structure
      for(i=0; i<arch->adjm_size; i++)
      {
         idx=map->map[i];
         if(idx>=0 && idx<fct->msize)
           matrix_content[i]=fct->original_netlist_idx[idx];
         else
           matrix_content[i]=-1;
      }
      
      // Dump the mapping result
      if(echo_dot)
      {
          dot_print_mapping_with_name(map,filename);
#ifdef VERBOSE_DEBUG
          printf("DOT print mapping: %s\n", filename);
#endif
      }    
   }

   // Clean up the structures
   if(exploration_list != NULL)
     free(exploration_list);

   if(map !=NULL)
   {
      free_mapping(map);
      free(map);
   }

   return result;
}
