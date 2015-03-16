// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon

// Clustering of the gates into MClusters

// This source file will greedily fill the clusters with gates. The legitimity test will be done by the matrix packer.
// Most of the selection algorithm and speed up tricks are adapted from original VPack

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mpack.h"
#include "globals.h"
#include "cluster.h"
#include "output_clustering.h"
#include "function.h"
#include "matrix_architecture.h"
#include "access.h"
#include "matrix_packer.h"

// Marks gains that aren't valid
// Ensure no gain can ever be this negative!
#define NO_CLUSTER -1
#define NEVER_CLUSTER -2
#define NOT_VALID -10000

enum e_gain_update {GAIN, NO_GAIN};
enum e_gain_type {SHARED_PINS, INPUT_REDUCTION};
enum e_feasibility {FEASIBLE, INFEASIBLE};
enum e_removal_policy {REMOVE_CLUSTERED, LEAVE_CLUSTERED};

// Linked list structure. Stores one integer (iblk).
struct s_ilink {int iblk; struct s_ilink *next;};

// 1/MARKED_FRAC is the fraction of nets or blocks that must be marked in order for the brute force (go through the whole data structure linearly) gain update code to be used. 
// This is done for speed only; make MARKED_FRAC whatever number speeds the code up most.   
#define MARKED_FRAC 2   

// [0..num_nets-1].  How many pins of each net are contained in the currently open cluster? 
static int *num_pins_of_net_in_cluster;

// [0..num_nets-1].  Is the driver for this net in the open cluster?
static boolean *net_output_in_cluster;

// [0..num_blocks-1].  Which cluster (numbered from 0 to num_cluster - 1) is this block in? If not assigned to a cluster yet, it is NO_CLUSTER.
static int *cluster_of_block;

// [0..num_blocks-1].  Reduction in number of pins to connect that would result from each block being added to the currently open cluster.
// gain[iblk] is NOT_VALID if it hasn't been computed for iblk yet.
static int *gain;

// [0..num_marked_nets] and [0..num_marked_blocks] respectively.  List the indices of the nets and blocks that have had their num_pins_of_net_in_cluster and gain entries altered.                             */
static int *marked_nets, *marked_blocks;
static int num_marked_nets, num_marked_blocks;

// To speed up some linear searches, store the index of the first clusterable (LUT, LATCH, etc.) block.
static int first_clusterable_block = -1;

// Keeps a linked list of the unclustered blocks to speed up looking for unclustered blocks with a certain number of *external* inputs. [0..cell_size].
// Unclustered_list_head[i] points to the head of the list of gates with i inputs to be hooked up via external interconnect.
static struct s_ilink *unclustered_list_head;
static struct s_ilink *memory_pool; // Declared here to be freeed easily.

// Does the block that drives the output of this net also appear as a receiver (input) pin of the net?  [0..num_nets-1].
// This is used in the gain routines to avoid double counting the connections from the current cluster to other blocks (hence yielding better clusterings).
// The only time a block should connect to the same net twice is when one connection is an output and the other is an input, so this should take care of all multiple connections.
static boolean *net_output_feeds_driving_block_input;

// Store the global matrix architecture targeted by the packer
static matrix arch;

// Store the gate mapping on the different matrices (+support for rollback)
static int **matrix_contents, **matrix_contents_tmp;



// Echo back the clustering data structures (pre_mapping) to file XX.echo
void echo_clustering_info (char *echo_file)
{
 int i, j; 
 FILE *fp;
 
 fp = fopen (echo_file, "w");

 fprintf(fp, "Num cluster : %d\n",num_blocks);

 fprintf(fp,"\n\n\nBlocks\t\t\tBlock Type Legend:\n");
 fprintf(fp,"\t\t\tINPAD = %d\tOUTPAD = %d\n", INPAD, OUTPAD);
 fprintf(fp,"\t\t\tGATE = %d\t\tLATCH = %d\n", GATE, LATCH);
 fprintf(fp,"\t\t\tEMPTY = %d\t\n\n", EMPTY);
 
 fprintf(fp,"\nBlock\tName\t\tType\t#Nets\tCluster\tOutput\tInputs\t");
 fprintf(fp,"Clock\n\n");

 for (i=0;i<num_blocks;i++) { 
    fprintf(fp,"\n%d\t%s\t",i, block[i].name);
    if (strlen(block[i].name) < 8)
       fprintf(fp,"\t"); // Name field is 16 chars wide
    fprintf(fp,"%d\t%d\t%d", block[i].type, block[i].num_nets,cluster_of_block[i]);

    for(j=block[i].num_in;j<block[i].num_out+block[i].num_in;j++)
       fprintf(fp,"\to:%d",block[i].nets[j]);
    for(j=0;j<block[i].num_in;j++)
       fprintf(fp,"\ti:%d",block[i].nets[j]);

    if(block[i].type == LATCH) {
       for(j=block[i].num_out+block[i].num_in;j<block[i].num_out+block[i].num_in+1;j++)
         fprintf(fp,"\tc:%d", block[i].nets[j]);
    }
 }  

 fprintf(fp,"\n");
 fclose(fp);
}

// Echo back the clustering data structures (validated, i.e., post-mapping) to file XX.echo
void echo_clustering_validated_info (char *echo_file)
{
 int i, j; 
 FILE *fp;
 
 fp = fopen (echo_file, "w");

 fprintf(fp, "Num cluster : %d\n",num_blocks);

 fprintf(fp,"\n\n\nBlocks\t\t\tBlock Type Legend:\n");
 fprintf(fp,"\t\t\tINPAD = %d\tOUTPAD = %d\n", INPAD, OUTPAD);
 fprintf(fp,"\t\t\tGATE = %d\t\tLATCH = %d\n", GATE, LATCH);
 fprintf(fp,"\t\t\tEMPTY = %d\t\n\n", EMPTY);
 
 fprintf(fp,"\nBlock\t\tName\t\tType\t\t#Nets\t\tCluster\t\tOutput\t\tInputs\t\t");
 fprintf(fp,"Clock\n\n");

 for (i=0;i<num_blocks_validated;i++) { 
    fprintf(fp,"\n%d\t\t%s\t\t",i, block_validated[i].name);
    if (strlen(block_validated[i].name) < 8)
       fprintf(fp,"\t\t"); // Name field is 16 chars wide
       
    if(block_validated[i].type == BUF)
       fprintf(fp,"%d\t\t%d\t\t", block_validated[i].type, block_validated[i].num_nets);
    else
       fprintf(fp,"%d\t\t%d\t\t%d", block_validated[i].type, block_validated[i].num_nets,cluster_of_block[i]);

    for(j=block_validated[i].num_in;j<block_validated[i].num_out+block_validated[i].num_in;j++)
       fprintf(fp,"\t\to:%d",block_validated[i].nets[j]);
    for(j=0;j<block_validated[i].num_in;j++)
       fprintf(fp,"\t\ti:%d",block_validated[i].nets[j]);

    if(block_validated[i].type == LATCH) {
       for(j=block_validated[i].num_out+block_validated[i].num_in;j<block_validated[i].num_out+block_validated[i].num_in+1;j++)
         fprintf(fp,"\t\tc:%d", block_validated[i].nets[j]);
    }
 }  

 fprintf(fp,"\n");
 fclose(fp);
}

// Returns the number of used input pins on this block.
int num_input_pins (int iblk) {
 int conn_inps;

 switch (block[iblk].type) {
    case GATE:
        conn_inps = block[iblk].num_in;
        break;

    case LATCH: /// CONSIDER the latchs
        conn_inps = block[iblk].num_nets - 2; // -2 for output + clock
        break;

    default: // This routine should only be used for logic blocks
        fprintf(stderr, "Error in num_input_pins:  Unexpected block type %d for block %d.  Aborting.\n", block[iblk].type, iblk);
        exit(EXIT_FAILURE);
        break;
 }
 
 return (conn_inps);
}


static int num_ext_inputs (int iblk) {

/* Returns the number of input pins on this block that must be hooked * 
 * up through external interconnect.  That is, the number of input    *
 * pins used - the number which connect (internally) to the output.   */

 int ext_inps, output_net, ipin, i;

 ext_inps = num_input_pins(iblk);


 for (i=block[iblk].num_in;i<(block[iblk].num_in+block[iblk].num_out);i++)
 {

      output_net = block[iblk].nets[i];

//       printf("num ext inputs : blk=%s, net=%s\n", block[iblk].name, net[output_net].name);
 
      /* NB:  I could speed things up a bit by computing the number of inputs *
      * and number of external inputs for each logic block at the start of   *
      * clustering and storing them in arrays.  Look into if speed is a      *
      * problem.                                                             */
      
      for (ipin=0;ipin<block[iblk].num_in;ipin++) {
         if (block[iblk].nets[ipin] == output_net)
            ext_inps--;
      }
 }

 return (ext_inps);
}

// Checks that nets used as clock inputs to latches are never also used as gates inputs.
static void check_clocks (boolean *is_clock) {
 int inet, iblk, ipin;

 // Check for all blocks
 for (iblk=0;iblk<num_blocks;iblk++) {
    if (block[iblk].type == GATE) {
       for(ipin=0; ipin<block[iblk].num_in; ipin++) {
          inet = block[iblk].nets[ipin]; 
          if (inet != OPEN) {
             if (is_clock[inet]) {
                fprintf(stderr, "Error in check_clocks.  Net %d (%s) is a clock, but also connects to a GATE input on block %d (%s).\n", inet, net[inet].name, iblk, block[iblk].name);
                fprintf(stderr, "Clustering has been aborted.\n");
                exit(EXIT_FAILURE);
             }
          }
       }
    }

    if (block[iblk].type == LATCH) {
          inet = block[iblk].nets[0]; 
          if (inet != OPEN) {
             if (is_clock[inet]) {
                fprintf(stderr, "Error in check_clocks.  Net %d (%s) is a clock, but also connects to a LATCH input on block %d (%s).\n", inet, net[inet].name, iblk, block[iblk].name);
                fprintf(stderr, "Clustering has been aborted.\n");
                exit(EXIT_FAILURE);
             }
          }
    }
 }
}


// Allocates the main data structures used for clustering and properly initializes them.       
static void alloc_and_init_clustering (int cell_size, int nets_per_matrix) {

 int i, ext_inps, ipin, driving_blk, inet;
 struct s_ilink *next_ptr;

 cluster_of_block = (int *) malloc (num_blocks * sizeof(int));
 gain = (int *) malloc (num_blocks * sizeof (int));

 for (i=0;i<num_blocks;i++) {
    gain[i] = NOT_VALID;
    if (block[i].type == GATE) {
        cluster_of_block[i] = NO_CLUSTER;
        if (first_clusterable_block == -1)
           first_clusterable_block = i;
    }
    else
        cluster_of_block[i] = NEVER_CLUSTER; // Allows to go throw latches without clustering them
 }

    if (first_clusterable_block != num_p_inputs + num_p_outputs) {
       fprintf(stderr, "Error in alloc_and_init_clustering: first_clusterable_block = %d, expected %d.\n", first_clusterable_block, num_p_inputs + num_p_outputs);
       exit(EXIT_FAILURE);
    }

 num_pins_of_net_in_cluster = (int *) malloc (num_nets * sizeof(int));
 net_output_in_cluster = (boolean *) malloc (num_nets * sizeof(boolean));

 for (i=0;i<num_nets;i++) {
    num_pins_of_net_in_cluster[i] = 0;
    net_output_in_cluster[i] = FALSE;
 }

 marked_nets = (int *) malloc (nets_per_matrix*sizeof(int));
 marked_blocks = (int *) malloc (num_blocks * sizeof(int));

 num_marked_nets = 0;
 num_marked_blocks = 0;

 unclustered_list_head = (struct s_ilink *) malloc ((cell_size+1) * sizeof(struct s_ilink));
 for (i=0;i<=cell_size;i++)
    unclustered_list_head[i].next = NULL;
 
 memory_pool = (struct s_ilink *) malloc ((num_blocks - first_clusterable_block) * sizeof (struct s_ilink));
 next_ptr = memory_pool;
 
 for (i=first_clusterable_block;i<num_blocks;i++) {
    if (block[i].type == GATE) {
       ext_inps = num_ext_inputs(i);
       next_ptr->next = unclustered_list_head[ext_inps].next;
       unclustered_list_head[ext_inps].next = next_ptr;
       next_ptr->iblk = i;
       next_ptr++;
    }
 }

 // This is used in the gain routines to avoid double counting the connections from the current cluster to other blocks (hence yielding better clusterings).
 // The only time a block should connect to the same net twice is when one connection is an output and the other is an input, so this should take care of all multiple connections.
 net_output_feeds_driving_block_input = (boolean *) malloc (num_nets * sizeof (boolean));

 for (inet=0;inet<num_nets;inet++) {
    net_output_feeds_driving_block_input[inet] = FALSE;
    driving_blk = net[inet].pins[0];
    for (ipin=1;ipin<net[inet].num_pins;ipin++) {
       if (net[inet].pins[ipin] == driving_blk) {
          net_output_feeds_driving_block_input[inet] = TRUE;
          break;
       }
    }
 }

 // Store the content of the matrices
 matrix_contents=NULL;
 matrix_contents_tmp=NULL;
}

// Allocates the structures saving the contents of the clustering main data structures used for clustering and properly initializes them.       
static void alloc_matrix_contents (int num_clusters, matrix *arch) {
  
 matrix_contents=(int**) realloc(matrix_contents, (num_clusters+1)*sizeof(int*));
 matrix_contents[num_clusters]=(int*) malloc(arch->adjm_size*sizeof(int));
 
 matrix_contents_tmp=(int**) realloc(matrix_contents_tmp, (num_clusters+1)*sizeof(int*));
 matrix_contents_tmp[num_clusters]=(int*) malloc(arch->adjm_size*sizeof(int));
}

// Releases all the memory used by clustering data structures.
static void free_clustering (void) {

 free (cluster_of_block);
 free (gain);
 free (num_pins_of_net_in_cluster);
 free (net_output_in_cluster);
 free (marked_nets);
 free (marked_blocks);
 free (unclustered_list_head);
 free (memory_pool);
 free (net_output_feeds_driving_block_input);
}

// Used only for debug purpose. Dump the current uncluster list.
void print_uncluster_list(int ext_inps)
{
   struct s_ilink *ptr, *prev_ptr;

   printf("Print unclustered_list_head : %d\n",ext_inps);

   prev_ptr = &unclustered_list_head[ext_inps];
   ptr = unclustered_list_head[ext_inps].next;

   while (ptr != NULL) {
      printf("iblk = %d\n",ptr->iblk);

    ptr = ptr->next;
 }
}

// Create a temporary storage of the blocks and nets
static void nets_and_blocks_temporary_storage () {   
  
   int i;
   
   // First clean the previous temp structures ...
   for(i=0; i<num_blocks_current; i++) // For the logic blocks
      if(block_current[i].name!=NULL) free(block_current[i].name);
   free(block_current);
   
   for(i=0; i<num_nets_current; i++) // For the nets
   {
      if(net_current[i].pins!=NULL) free(net_current[i].pins);
      if(net_current[i].name!=NULL) free(net_current[i].name);
   }
   free(net_current);
  
   // ... and reinitialize it
   num_blocks_current = num_blocks_validated; // Logic blocks
   block_current = (struct s_block *) malloc(num_blocks_validated * sizeof(struct s_block));
   memcpy(block_current, block_validated, num_blocks_validated * sizeof(struct s_block));
 
   for(i=0; i<num_blocks_validated; i++)
   {
      (block_current+i)->name=(char*) malloc((strlen((block_validated+i)->name)+1)*sizeof(char));
      memcpy((block_current+i)->name, (block_validated+i)->name,(strlen((block_validated+i)->name)+1)*sizeof(char));  
   }
         
   num_nets_current = num_nets_validated; // Nets
   net_current = (struct s_net *) malloc(num_nets_validated * sizeof(struct s_net));
   memcpy(net_current, net_validated, num_nets_validated * sizeof(struct s_net));
 
   for(i=0; i<num_nets_current; i++)
   {
      (net_current+i)->name=(char*) malloc((strlen((net_validated+i)->name)+1)*sizeof(char));
      memcpy((net_current+i)->name, (net_validated+i)->name,(strlen((net_validated+i)->name)+1)*sizeof(char));  
      
      (net_current+i)->pins=(int*) malloc((net_validated+i)->num_pins*sizeof(int));
      memcpy((net_current+i)->pins, (net_validated+i)->pins,((net_validated+i)->num_pins)*sizeof(int));  
   }
}

// Extracts the blocks belonging to cluster imatrix into the table nodes and adds iblk
// Considers only the block from the initial netlist
static int cluster_nodes_extraction (int **nodes, int imatrix, int iblk) {
  int nnb=0;
  int i;
  
  // Store the blocks of the cluster
  for(i=0; i<num_blocks; i++)
  {
    if(cluster_of_block[i] == imatrix)
    {
        *nodes=(int*)realloc(*nodes, (nnb+1)*sizeof(int));
        *(*nodes+nnb)=i; nnb++;
    }
  }
  
  // Finally add the new block
  *nodes=(int*)realloc(*nodes, (nnb+1)*sizeof(int));
  *(*nodes+nnb)=iblk; nnb++;
  return nnb;
}

// Extracts the nets belonging to the nodes listed in table nodes cluster
// Considers the validated structure to take into account the added buffers
static int cluster_nets_extraction (int **cnets, int *nodes, int nnb) {
 int netnb=0;
 int i,j,z,found;
 
 // For all the blocks in the cluster
 for(i=0; i<nnb; i++)
 {
    // For all the nets of each blocks
    for(j=0; j<block_validated[nodes[i]].num_nets; j++)
    {
      if(block_validated[nodes[i]].nets[j]!=-1) // Check if it is a floating net from formatted low-fanout gates
      { // If yes, do not add it
        
        found=0;
          
        // Check if the net has not been already added
        for(z=0; z<netnb; z++)
          if(*(*cnets+z)==block_validated[nodes[i]].nets[j]) found=1;
        
        // If the node is not already existing
        if(found==0)
        {
          *cnets=(int*)realloc(*cnets, (netnb+1)*sizeof(int));
          *(*cnets+netnb)=block_validated[nodes[i]].nets[j];
          netnb++;
        }
      }
    }
 }
 
 return netnb;
}

// Extracts the input and outpout blocks of the cluster
// Considers the validated structure to take into account the added buffers
static void cluster_inputs_outputs_extraction (int **inputs, int **outputs, int *inb, int *onb, int *cnets, int netnb, int *nodes, int nnb) {
  
 int idblk=-1;
 int drv, rcv;
 int i,j,k;
  
 // For all the nets in the clusters
 for(i=0; i<netnb; i++)
 { 
   idblk= net_validated[cnets[i]].pins[0];
   drv=0;
   // ... and checks if it belongs to the cluster
   for(j=0; j<nnb; j++)
   {
      if(idblk==nodes[j]) drv=1;
   }

   // The driver is not included in the cluster, then add it as an input
   if(drv==0)
   {
      *inputs=(int*)realloc(*inputs, (*inb+1)*sizeof(int));
      *(*inputs+*inb)=cnets[i];
      *inb=*inb+1;
   }
   // The driver is included in the cluster, so check if it is connected to a gate outside of the cluster
   // If yes, we will need to add it as an output
   else
   {
      // Check if all the receivers are included in the cluster
      rcv=0;
      // For all the receiving blocks
      for(j=1; j<net[cnets[i]].num_pins; j++)
      {
         idblk=net[cnets[i]].pins[j];

         // increment rcv if the current receiving block belongs to the cluster
         for(k=0; k<nnb; k++)
         {
            if(idblk==nodes[k]) rcv++;
         }
      }

      if(rcv != (net[cnets[i]].num_pins-1)) // All pins have not been found in the cluster, then adds an output
      {
         *outputs=(int*)realloc(*outputs, (*onb+1)*sizeof(int));
         *(*outputs+*onb)=cnets[i];
         *onb=*onb+1;
      }
   }
 } 
}

// Initializes a function using the cluster informations
// Considers the validated structure to take into account the added buffers
static void function_creation_from_cluster_informations (function *fct, int *inputs, int inb, int *outputs, int onb, int *cnets, int netnb, int *nodes, int nnb) {
   
 int i, j, k, netidx;
 int rcv, drv;
 int num_in, num_out;
 
 // Basic properties initializations 
 fct->nb_input=inb;
 fct->nb_nodes=nnb;
 fct->nb_output=onb;

 fct->msize=inb+nnb+onb;

 // Instantiate the adjacency matrix of the function and initialize it
 fct->adjm=(int**)malloc(fct->msize*sizeof(int*));
 for(i=0; i<fct->msize; i++)
   *(fct->adjm+i)=(int*)malloc(fct->msize*sizeof(int));
 for(i=0; i<fct->msize; i++)
   for(j=0; j<fct->msize; j++)
      *matrix_access(fct->adjm,i,j)=0;

 for(i=0; i<netnb; i++) // For all the nets involved in the cluster, we identify the drv and the rcv
 {
   netidx=cnets[i];

   drv=-1; // drv identies the position of the driver in the adjacency matrix of the cluster
   // Find the driver among the inputs 
   for(j=0; j<inb; j++)
   {
      if(inputs[j]==netidx) drv=j;

   }
   // or among the internal nodes
   for(j=0; j<nnb; j++)
   {
      num_in=block[nodes[j]].num_in; num_out=block[nodes[j]].num_out;
      for(k=num_in; k<num_in+num_out; k++)
         if(block_validated[nodes[j]].nets[k]==netidx) drv=j+inb;
   }

   rcv=-1; // rcv identies the position of the driver in the adjacency matrix of the cluster
   // Find the receiver among the outputs ...
   for(j=0; j<onb; j++)
   {
      if(outputs[j]==netidx)
      {
         rcv=j+nnb+inb;
         *matrix_access(fct->adjm,drv,rcv)=1; // drv and rcv identified: update the adjacency matrix
      }
   }
   // ... or amond the internal nodes
   for(j=0; j<nnb; j++)
   {
      for(k=0; k<block_validated[nodes[j]].num_in; k++)
      {
         if(block_validated[nodes[j]].nets[k]==netidx)
         {
            rcv=j+inb;
            *matrix_access(fct->adjm,drv,rcv)=1; // drv and rcv identified: update the adjacency matrix
         }
      }
   }
 }

 // Instantiates the full connectivity matrix
 fct->fconm=alloc_and_fill_full_connectivity_matrix(fct->adjm,fct->msize);

 // Instantiates and fills the names and original indices of the different blocks - useful for debug
 fct->names=(char**) malloc(fct->msize*sizeof(char*));
 fct->original_netlist_idx=(int*) malloc(fct->msize*sizeof(int));
 j=0;
 for(i=0; i<inb; i++) // Fill the structures for all the inputs
 {
   fct->names[j]=(char*)malloc((strlen(net_validated[inputs[i]].name)+1)*sizeof(char));
   strcpy(fct->names[j],net_validated[inputs[i]].name);
   fct->original_netlist_idx[j]=net_validated[inputs[i]].pins[0];
   j++;
 }

 for(i=0; i<nnb; i++) // Fill the structures for all the nodes
 {
   fct->names[j]=(char*)malloc((strlen(block[nodes[i]].name)+1)*sizeof(char));
   strcpy(fct->names[j],block[nodes[i]].name);
   fct->original_netlist_idx[j]=nodes[i];
   j++;
 }

 for(i=0; i<onb; i++) // Fill the structures for all the outputs
 {
   fct->names[j]=(char*)malloc((strlen(net_validated[outputs[i]].name)+1)*sizeof(char));
   strcpy(fct->names[j],net_validated[outputs[i]].name);
   fct->original_netlist_idx[j]=net_validated[outputs[i]].pins[0];
   j++;
 }

 // Fill the level stucture
 autofill_level (fct);

#ifdef VERBOSE_DEBUG
 printf("\nFUNCTION CREATION : Adjacency matrix\n");
 print_matrix(fct->adjm, fct->msize, fct->msize);
 printf("\nFUNCTION CREATION : Original netlist\n");
 print_table(fct->original_netlist_idx, fct->msize);
 printf("\nFUNCTION CREATION : Names\n");
 print_char_table(fct->names, fct->msize);
 printf("\nFUNCTION CREATION : Levels\n");
 print_table(fct->levels, fct->msize);
#endif
}

// Identifies the buffers inserted in the matrix and adds them to the block and net structurs
static void buffers_processing (int imatrix, function fct_original, function fct_current) {  

   char buf_name[BUFSIZE];
   int buf_idx = 0, drv_idx = 0;
   int i, j, k;
    
   int current_block, connected_block, connected_net, node_connected_A, node_connected_B;
   
   int cnum, bnum, bbnum, nnum, pnum;
   int offset, last_node, output_node, output_net, tag;
      
#ifdef VERBOSE_DEBUG
   printf("BUFFER PROCESSING: Matrix = %d, Initial num blocks = %d\n", imatrix, num_blocks_current);
   printf("BUFFER PROCESSING: Buffer count before = %d, Buffer count after formatting = %d, Inserted buffers =%d\n", buffer_counting_validated, buffer_counting_current, buffer_counting_current-buffer_counting_validated);
#endif
   // Keep track of the inserted buffer number between occurences of mapping success and failures
   buffer_counting_rollback=buffer_counting_current; 
      
   // For all the buffers inserted in the current matrix, after the function formatting operation
   for(i=0; i<(buffer_counting_current-buffer_counting_validated); i++)
   {
      // Add a buffer in the block list
      num_blocks_current++;
      sprintf(buf_name,"buf_%d_%d", imatrix, buf_idx);      
      block_current = (struct s_block *) realloc(block_current, num_blocks_current*sizeof(struct s_block));
      block_current[num_blocks_current-1].name=(char*) malloc((strlen(buf_name)+1)*sizeof(char));
      strcpy(block_current[num_blocks_current-1].name, buf_name);
      block_current[num_blocks_current-1].type=BUF;
      block_current[num_blocks_current-1].num_nets=3;
      block_current[num_blocks_current-1].num_out=1;
      block_current[num_blocks_current-1].num_in=2; // BUF blocks have 2 inputs in MClusters --> select either BUF on A or BUF on B 

      // Add a net in the net list       
      num_nets_current++;
      net_current = (struct s_net *) realloc(net_current, num_nets_current*sizeof(struct s_net));
      net_current[num_nets_current-1].name=(char*) malloc((strlen(buf_name)+1)*sizeof(char));
      strcpy(net_current[num_nets_current-1].name, buf_name); 
      net_current[num_nets_current-1].num_pins=1;  
      net_current[num_nets_current-1].pins=(int*) malloc(sizeof(int));
      net_current[num_nets_current-1].pins[0]=num_blocks_current-1;  
              
      block_current[num_blocks_current-1].nets[0]=-1; // Inputs not yet assigned
      block_current[num_blocks_current-1].nets[1]=-1;
      block_current[num_blocks_current-1].nets[2]=num_nets_current-1; // Output is the new added buffer
      
      buf_idx++;  
    }
    
   // Kill all the original function connections --> Will insert the buffers in place of an initial direct connection
   for(i=fct_original.nb_input; i<fct_original.nb_input+fct_original.nb_nodes; i++) // For every nodes in the function
   {
      current_block=fct_original.original_netlist_idx[i];
      // For each cells, reset all the input nets
      for(j=0; j<block_current[current_block].num_in; j++)
          block_current[current_block].nets[j]=-1;
   }
   // Kill all the nets receiver informations --> to prevent using corrupted data
   for(i=0; i<num_nets_current; i++) // For every nodes in the function (hence drivers)
   {
      for(j=1;j<net_current[i].num_pins;j++) // reset the receiver
        net_current[i].pins[j]=-1;
   }
   
   // Recreate the current connections using the formatted functions --> we basically translate the information of the function back into the netlist  
   for(i=fct_current.nb_input; i<fct_current.nb_input+fct_current.nb_nodes; i++) // For every nodes in the function
   {
      current_block=fct_current.original_netlist_idx[i];

      for(j=0; j<fct_current.msize; j++) // Node i which corresponds to the current_block is connected to ...
      {
          if(*matrix_access(fct_current.adjm,j,i)==1) // Node j is driving the node i
          {
             connected_block= fct_current.original_netlist_idx[j];
             
             drv_idx=block_current[connected_block].num_in;                     
             connected_net=block_current[connected_block].nets[drv_idx]; 
                                  
             // Identify if it is a A or B connections - Note that indix i cannot be used as here we need the mapping information
             /// Work only for 2 input functions
              for(k=arch.width*GATE_MAX_INPUT_NB; k<(arch.adjm_size-(arch.width*GATE_MAX_OUTPUT_NB)); k++) // For all the physical nodes of the matrix
              {
                  if(matrix_contents_tmp[imatrix][k]==current_block) // If the current block is mapped to the physical node
                  {
                      drv_idx=arch.terminal_interconnect_matrix[k][1]; // Identify the node driving the input A
                      node_connected_A=matrix_contents_tmp[imatrix][drv_idx];
                      drv_idx=arch.terminal_interconnect_matrix[k][2]; // Indentify the node driving the input B
                      node_connected_B=matrix_contents_tmp[imatrix][drv_idx];
                  }
              }
  
              if(connected_block==node_connected_A)
                  block_current[current_block].nets[0]=connected_net;
                          
              if(connected_block==node_connected_B)
                  block_current[current_block].nets[1]=connected_net;
          }
      } 
   }
   
   // BUF Type correction --> BUF is added by the function formatting, but we only know which terminal is used after the mapping step
    for(i=0; i<(buffer_counting_current-buffer_counting_validated); i++)
    {
       if(block_current[num_blocks_current-1-i].type!=BUF)
       {
          fprintf(stderr, "ERROR: Should look at a buffer right now\n");
          exit(EXIT_FAILURE);
       }
       
       if(block_current[num_blocks_current-1-i].nets[0]==-1)
         block_current[num_blocks_current-1-i].gate_type=BUF1B;
       if(block_current[num_blocks_current-1-i].nets[1]==-1)
         block_current[num_blocks_current-1-i].gate_type=BUF1A;
    }
  
   // Update the output of the cluster in case a buffer has been added and make it match with the logic block names
   // The initial output net of a MCluster is not anymore the initial node but its bufferized version

   // For all the outputs in the MCluster
   for(bnum=0; bnum<number_of_outputs(&arch); bnum++)
   {
      // Get the block indix of the initial output driver and the last node connected to it (can be a buffer)
      offset=number_of_inputs(&arch)+number_of_nodes(&arch);
      last_node=matrix_contents_tmp[imatrix][offset+bnum-(arch.width)];
      output_node=matrix_contents_tmp[imatrix][offset+bnum];
       
      if(last_node!=output_node) // Need to update the names ! because a buffer has been inserted !
      {
         matrix_contents_tmp[imatrix][offset+bnum]=last_node; // Correct the output of the Mcluster

         // Look at the initial net going out of the block
         output_net=block_validated[output_node].nets[block_validated[output_node].num_in];
         
         // Check if this net is a primary output (and correct if yes)
         for(nnum=num_p_inputs;nnum<num_p_inputs+num_p_outputs;nnum++)
         {
              // The output net is a primary output! --> Then, we update the net informations (to the last node connecting the output) of the primary output to match the buf name
              if(block_current[nnum].nets[0]==output_net)
              {
                  block_current[nnum].nets[0]=block_current[last_node].nets[block_current[last_node].num_in]; // Update the net id 
                  free(block_current[nnum].name); // Update the name
                  block_current[nnum].name=(char*) malloc((strlen(block_current[last_node].name)+1)*sizeof(char));
                  strcpy(block_current[nnum].name, block_current[last_node].name);
              }
         }
         
         // Correct the inputs of subsequent blocks (that were initially connected to the output)
         for(nnum=num_p_inputs+num_p_outputs; nnum<num_blocks_validated; nnum++)
         {  // For all the blocks
         
            // First check if the block belongs to this cluster (tag=1 --> block belongs to this cluster
            tag=0;
            for(bbnum=number_of_inputs(&arch); bbnum<number_of_nodes(&arch); bbnum++)
            { // For all the nodes of the cluster
              if(matrix_contents_tmp[imatrix][bbnum]==nnum)
              tag=1;
            }

            // Only if the node is not part of the bluster --> We correct the inputs names
            if(tag==0)
            {
                // For every inputs of the block
                for(pnum=0; pnum<block_current[nnum].num_in; pnum++)
                {   // Update the net id if the net if match with the net to correct
                    if(block_current[nnum].nets[pnum]==block_current[output_node].nets[block_current[output_node].num_in])
                      block_current[nnum].nets[pnum]=block_current[last_node].nets[block_current[last_node].num_in]; 
                }
            }
         }

         // For all the previously mapped clusters, update the input net ids.
         for(cnum=0; cnum<=imatrix; cnum++)
         {
              for(bbnum=0; bbnum<number_of_inputs(&arch); bbnum++)
              {
                    if(matrix_contents_tmp[cnum][bbnum]==output_node)
                        matrix_contents_tmp[cnum][bbnum]=last_node;
              }
         }
      }
   }
}

// Checks if adding iblk to the currently open cluster is possible
// We first check if it violates the cluster input and clock pin limitations.
// Then, we try to map the current cluster graph onto a physical matrix
// Returns TRUE if iblk can be added to the cluster, FALSE otherwise.
static boolean matrix_feasible (int iblk, int imatrix, boolean echo_dot) {

 int i;

 // Used to create the logic functions
 int inb=0, onb=0, nnb=0, netnb=0;

 function fct_current, fct_original; // fct_original Used for print purpose

 // Store the cluster informations
 int *cnets=NULL, *nodes=NULL, *inputs=NULL, *outputs=NULL;

 int result=0;

 char name[BUFSIZE];

 //
 if(iblk==-1) return FALSE;

 init_function(&fct_current); init_function(&fct_original);
 
 buffer_counting_current = buffer_counting_validated;

#ifdef VERBOSE_DEBUG
 printf("Matrix feasability : iblk = %d (%s), cluster = %d\n",iblk, block[iblk].name, imatrix);
#endif

/////////////////
// We first perform some basic tests on the cluster size
////////////////

 // Store the nodes of the cluster
 nnb = cluster_nodes_extraction(&nodes, imatrix, iblk);
   
 // BASIC feasability test 1: Do we have more blocks than the physical matrix can accomodate ?
 if(nnb>number_of_nodes(&arch))
 {
   free(nodes);  
   buffer_counting_current = buffer_counting_rollback; // Function too big for the matrix: no buufer inserted
   return FALSE;
 }
 
 // Store the nets of the cluster 
 netnb = cluster_nets_extraction (&cnets, nodes, nnb);
 
 // Store the inputs and outputs of the cluster
 cluster_inputs_outputs_extraction (&inputs, &outputs, &inb, &onb, cnets, netnb, nodes, nnb);

 // BASIC feasability test 2: Do we have more inputs or outputs than the physical matrix can accomodate ?
 if(inb>number_of_inputs(&arch) ||onb>number_of_outputs(&arch))
 {
    free(nodes);
    free(inputs);
    free(outputs);
    free(cnets);
    buffer_counting_current = buffer_counting_rollback; // Function too big for the matrix: no buufer inserted
    return FALSE;
 }
 
#ifdef VERBOSE_DEBUG
 printf("\n Nodes in cluster :nnb = %d\n",nnb);
 for(i=0; i<nnb; i++) printf("%d\t",nodes[i]);

 printf("\n Nets of cluster : netnb = %d\n",netnb);
 for(i=0; i<netnb; i++)  printf("%d\t",cnets[i]);
 
 printf("\n Inputs of cluster : inb = %d\n",inb);
 for(i=0; i<inb; i++)  printf("%d\t",inputs[i]);
 
 printf("\n Outputs of cluster : onb = %d\n",onb);
 for(i=0; i<onb; i++)  printf("%d\t",outputs[i]);

// echo_debug_validated ("Debug echo post mapping", "./WORK/debug_validated.echo");
// echo_debug_current ("Debug echo post mapping tmp", "./WORK/debug_current.echo");   
#endif

/////////////////
// When the initial assumptions are fullfiled, we try to map the cluster on the physical matrix
// We will use the mapper module, therefore we need to consider a function
////////////////

 // Create the function according from the cluster informations
 function_creation_from_cluster_informations (&fct_current, inputs, inb, outputs, onb, cnets, netnb, nodes, nnb);
//   fill_test_fct1 (&fct_current);
//   fill_test_fct1 (&fct_original);
 
 if(echo_dot) // If we want to further echo the function before formatting, do a local copy
   copy_function(&fct_original,&fct_current);
 
 // Format the function according to the physical architecture --> Will add some buffers to the list of nodes
 function_formatting(&fct_current, arch.depth);
 
  // BASIC feasability test 3: Does the formatted function, i.e., with buffers, can fit into the matrix ?
 if(fct_current.nb_nodes>number_of_nodes(&arch) || fct_current.nb_input>number_of_inputs(&arch) || fct_current.nb_output>number_of_outputs(&arch) )
 {
   free(nodes); free(cnets); free(inputs); free(outputs); 
   free_function(&fct_current); free_function(&fct_original);
   buffer_counting_current = buffer_counting_rollback; // Function too big for the matrix: no buufer inserted
   return FALSE;
 }

  // MAPPING OPERATION: Try to map the formatted function on the physical matrix
 if(echo_dot) // If we want to echo the mapping result, prepare the name
   sprintf(name,"./WORK/FIG/map_cluster%04d.dot",imatrix);
 result = matrix_mapping(&arch, &fct_current, matrix_contents[imatrix], echo_dot, name);
 
/////////////////
// Process the mapping result
// If success --> echo the mapping, and update the block and net structures to take into account the additional buffers
// If fail --> Rollback
////////////////
 if(result)
 {
   if(echo_dot) // First, echo the original and the formatted functions
   {
      sprintf(name,"./WORK/FIG/fct_cluster%04d.dot",imatrix);
      dot_print_function_with_name(&fct_original, name);
#ifdef VERBOSE_DEBUG
      printf("DOT print function : %s\n", name);
#endif
      sprintf(name,"./WORK/FIG/fct_cluster%04df.dot",imatrix);
      dot_print_function_with_name(&fct_current, name);
#ifdef VERBOSE_DEBUG
      printf("DOT print formatted function : %s\n", name);
#endif

   }
   
   // If Mapping success --> Store the different structures into a temporary structure --> Enable a rollback when the cluster is not yet finalized
   nets_and_blocks_temporary_storage();
   for(i=0; i<(imatrix+1); i++)
      memcpy(matrix_contents_tmp[i], matrix_contents[i], (arch.adjm_size) * sizeof(int));

   // Add the buffers added to the matrix to the blocks and nets structures
   buffers_processing(imatrix, fct_original, fct_current);

#ifdef DEBUG
   printf("Mapping success\n");
#endif

#ifdef VERBOSE_DEBUG
   printf("CLUSTER %d: Matrix_content\n",imatrix);
   print_table(matrix_contents_tmp[imatrix], arch.adjm_size);
#endif

 }
 else 
 { // The mapping was unsuccessful --> Rollback to the previous state

   buffer_counting_current = buffer_counting_rollback; // No buffers have been inserted, restore the previous state
#ifdef DEBUG
   printf("Mapping failed\n");
#endif

 }

  // Clean the data structures
  free(nodes); free(inputs); free(outputs); free(cnets);
  free_function(&fct_current); free_function(&fct_original);
   
  return result;
}


// This routine returns a block which has not been clustered, has no connection to the current cluster,
// satisfies the cluster clock constraints, and has ext_inps external inputs.
// If there is no such block it returns NO_CLUSTER.
// Remove_flag controls whether or not blocks that have already been clustered are removed from the unclustered_list data structures.
// NB:to get a block regardless of clock constraints just set clocks_avail > 0.       
static int get_block_by_num_ext_inputs (int ext_inps, enum e_removal_policy remove_flag) {

 struct s_ilink *ptr, *prev_ptr;
 int iblk;

#ifdef VERBOSE_DEBUG
//  print_uncluster_list(ext_inps);
#endif
 
 prev_ptr = &unclustered_list_head[ext_inps];
 ptr = unclustered_list_head[ext_inps].next;

 while (ptr != NULL) {
    iblk = ptr->iblk;

    if (cluster_of_block[iblk] == NO_CLUSTER) {
       return (iblk);
    }
    else if (remove_flag == REMOVE_CLUSTERED) {
       prev_ptr->next = ptr->next;
    }

    ptr = ptr->next;
 }
 
 return (NO_CLUSTER);
}


// This routine is used to find the first seed block for the clustering.
// It is also used to find new blocks for clustering when there are no feasible blocks with any attraction to the current cluster,
// i.e., it finds blocks which are unconnected from the current cluster.
// It returns the block with the largest number of used inputs that satisfies the clocking and number of inputs constraints.
// If no suitable block is found, the routine returns NO_CLUSTER.
static int get_free_block_with_most_ext_inputs (int cell_size, int inputs_avail, int current_cluster, boolean echo_dot) {
  
 int iblk, ext_inps, istart; // ext_inps for external inputs

 // The search starts with blocks with the largest number of inputs
 istart = min (cell_size, inputs_avail); // (or less if less inputs are available) 

 for (ext_inps=istart;ext_inps>=0;ext_inps--) { // then it looks at nodes with a lower count of inputs
   
    iblk = get_block_by_num_ext_inputs (ext_inps, REMOVE_CLUSTERED);

    // Test if the node is valid, i.e., not already clustered and mappable on a matrix
    if (iblk != NO_CLUSTER && matrix_feasible(iblk,current_cluster,echo_dot)==TRUE)
       return (iblk);
 }

 return (NO_CLUSTER); // No suitable block found.
}


// Call this routine when starting to fill up a new cluster.
// It resets the cluster data structure, the gain vector, etc.
static void reset_cluster (int *inputs_used, int *outputs_used, int *cells_used, int *clocks_used) {

 int i;

 *inputs_used = 0; *outputs_used = 0;  *cells_used = 0;  *clocks_used = 0;
 
 // The IF statement below is for speed (coming from VPACK)
 // If nets are reasonably low-fanout, only a relatively small number of blocks/nets will be marked, and updating only those block/net structures will be fastest.
 // If almost all blocks have been touched, it should be faster to just run through them all in order (less addressing and better cache locality).

 if (num_marked_blocks < num_blocks/MARKED_FRAC) {
    for (i=0;i<num_marked_blocks;i++) 
       gain[marked_blocks[i]] = NOT_VALID;
 }
 else {
    for (i=first_clusterable_block;i<num_blocks;i++) 
       gain[i] = NOT_VALID;
 }
 
 num_marked_blocks = 0;

 if (num_marked_nets < num_nets/MARKED_FRAC) {
    for (i=0;i<num_marked_nets;i++) {
       num_pins_of_net_in_cluster[marked_nets[i]] = 0;
       net_output_in_cluster[marked_nets[i]] = FALSE;
    }
 }
 else {
    for (i=0;i<num_nets;i++) {
       num_pins_of_net_in_cluster[i] = 0;
       net_output_in_cluster[i] = FALSE;
    }
 }
 
 num_marked_nets = 0;
}

// Updates the marked data structures, and if gain_flag is GAIN, the gain when a logic block is added to a cluster.
// The gain is basically the number of input pins saved when a block is added to a cluster (vs. putting it in a cluster with unrelated logic).
// When a logic block has more than one pin of a net connected to it, in practice this only happens when you have
//    1- a cluster + LATCH logic block where the output is fed back to an input
//    2- when two functions are mapped on a single cluster and the function 2 needs the result of function 1
// then the gain from a cluster to this block will increase by however many pins the net has on this block, rather than 1.
// This border phenemenon will be compensated after the return function
static void mark_and_update_gain (int inet, enum e_gain_update gain_flag, enum e_gain_type gain_type) {

 int iblk, ipin, ifirst;

 // Mark net as being visited, if necessary
 if (num_pins_of_net_in_cluster[inet] == 0) {
    marked_nets[num_marked_nets] = inet;
    num_marked_nets++;
 }

 // Update gains of affected blocks
 if (num_pins_of_net_in_cluster[inet] == 0 && gain_flag == GAIN) {

   // Check if this net lists its driving block twice.
   // The only time a block should connect to the same net twice is when one connection is an output and the other is an input
   // If so, avoid double counting this block by skipping the first (driving) pin.
    if (net_output_feeds_driving_block_input[inet] == FALSE) 
       ifirst = 0;
    else
       ifirst = 1;
    
    // For all the pins of the net (input and output)
    for (ipin=ifirst;ipin<net[inet].num_pins;ipin++) {
       iblk = net[inet].pins[ipin];
       if (cluster_of_block[iblk] == NO_CLUSTER) { // The block is not yet clustered
          if (gain[iblk] == NOT_VALID) { // The gain has never been computed yet
             marked_blocks[num_marked_blocks] = iblk;
             num_marked_blocks++;
             if (gain_type == SHARED_PINS) 
                gain[iblk] = 1;
             else // INPUT_REDUCTION
                gain[iblk] = 1 - num_ext_inputs (iblk);
          }
          else { // If already computed once, just increment
             gain[iblk]++;
          }
       }
    }
 }
    
 num_pins_of_net_in_cluster[inet]++; // Net inet belonging to a cluster
}

// Marks new_blk as belonging to cluster cluster_index and updates all the gain, etc. structures.
// Cluster_index should always be the index of the currently open cluster.
static void add_to_cluster (int new_blk, int cluster_index, boolean *is_clock, boolean global_clocks, int *inputs_used, int *output_used, int *cells_used) {

  int ipin, inet, i;

  printf("ADD TO CLUSTER : add %d (%s) to cluster %d\n",new_blk, block[new_blk].name, cluster_index);

  cluster_of_block[new_blk] = cluster_index;
  (*cells_used)++; // Increase the number of used cells

  // For all the output nets of the gate
  for(i=block[new_blk].num_in; i<(block[new_blk].num_in+block[new_blk].num_out);i++)
  {
     (*output_used)++; // Increase the number of used outputs
      
     inet = block[new_blk].nets[i]; // Get the net id
      
     // Output should never be open but I check anyway.
     // The gain is not updated for clock outputs when clocks are globally distributed.     
     if (inet != OPEN) {
        if (!is_clock[inet] || !global_clocks) 
           mark_and_update_gain (inet, GAIN, SHARED_PINS);
        else
           mark_and_update_gain (inet, NO_GAIN, SHARED_PINS);
      
        net_output_in_cluster[inet] = TRUE;
      
        if (num_pins_of_net_in_cluster[inet] > 1 && !is_clock[inet])
           (*inputs_used)--; // A given net is used at least 2 times in the cluster --> We don't need more inputs (only one is enough)
     }
  }

  // For all the gate input nets of the gate
  for (ipin=0;ipin<block[new_blk].num_in;ipin++) {
   
     inet = block[new_blk].nets[ipin];
     // Only update the connected input pins
     // This condition does not exist in general except for INV, BUF, ZERO and ONES where the fan-in is artificially increased
     if (inet != OPEN) {
        mark_and_update_gain (inet, GAIN, SHARED_PINS);
   
        // If num_pins_of_net_in_cluster != 1, then either the output is in the cluster or an input for this net is already in the cluster. In either case we don't need a new pin.
        if (num_pins_of_net_in_cluster[inet] == 1) 
           (*inputs_used)++;
     }
  }
}

// This routine finds the block with the highest gain that is not currently in a cluster and satisfies the feasibility function.
// If there are no feasible blocks, it returns NO_CLUSTER.                               
static int get_highest_gain_block_for_matrix (int inputs_avail, boolean *is_clock, int current_cluster, boolean echo_dot) {
 int best_gain, best_block, i, iblk;

 best_gain = NOT_VALID + 1; 
 best_block = NO_CLUSTER;

 // Divide into two cases for speed only.

 // Typical case:  not too many blocks have been marked.
 if (num_marked_blocks < num_blocks / MARKED_FRAC) {
    for (i=0;i<num_marked_blocks;i++) {
       iblk = marked_blocks[i];
       if (cluster_of_block[iblk] == NO_CLUSTER) {
          if (gain[iblk] > best_gain) {
             if (matrix_feasible (iblk, current_cluster, echo_dot)) {
                best_gain = gain[iblk];
                best_block = iblk;
             }
          }
       }
    }
 }
 // Some high fanout nets marked lots of blocks.
 else {   
    for (iblk=first_clusterable_block;iblk<num_blocks;iblk++) {
       if (cluster_of_block[iblk] == NO_CLUSTER) {
          if (gain[iblk] > best_gain) {
             if (matrix_feasible (iblk, current_cluster, echo_dot)) {
                best_gain = gain[iblk];
                best_block = iblk;
             }
          }
       }
    }
 }

 return (best_block);
}

// Selects the next gate candidate for the cluster: either one with the largest gain to the current portion of the circuit already clustered or an independant one
static int get_gate_for_matrix(int inputs_avail, int cell_size, int cells_used, int outputs_used, boolean *is_clock, int current_cluster, int ** matrix_contents, boolean echo_dot)
{
 int best_block;

 best_block = get_highest_gain_block_for_matrix (inputs_avail, is_clock, current_cluster, echo_dot);

 // If no blocks have any gain to the current cluster, the code above will not find anything.
 // However, another block with no inputs in common with the cluster may still be inserted into the cluster.
 if (best_block == NO_CLUSTER)
 { 
#ifdef VERBOSE_DEBUG
    printf("GET GATE FOR MATRIX : Free block - Inputs available=%d\n",inputs_avail);
#endif
    best_block = get_free_block_with_most_ext_inputs (cell_size, inputs_avail, current_cluster, echo_dot);

 }
 return (best_block);
}

// Transfer the working copy of the mapping informations into the validated copy.
static void validate_mapping_copy(int num_clusters) {

  int i;
  
  // Store the current value for the buffer counting
  buffer_counting_validated= buffer_counting_current;
       
  // Store the new block state to take into account the inserted buffers
  // First clean the previous structure
  for(i=0; i<num_blocks_validated; i++)
    if(block_validated[i].name!=NULL) free(block_validated[i].name);
  free(block_validated);       
  // Then, start the copy
  num_blocks_validated = num_blocks_current;
  block_validated = (struct s_block *) malloc(num_blocks_current*sizeof(struct s_block));
  memcpy(block_validated, block_current, num_blocks_current * sizeof(struct s_block));
  
  for(i=0; i<num_blocks_current; i++)
  {
    (block_validated+i)->name=(char*) malloc((strlen((block_current+i)->name)+1)*sizeof(char));
    memcpy((block_validated+i)->name, (block_current+i)->name,(strlen((block_current+i)->name)+1)*sizeof(char));  
  }
       
  // Store the new net state to take into account the inserted buffers
  // First, clean the previous structure
  for(i=0; i<num_nets_validated; i++)
  {
    if(net_validated[i].name!=NULL) free(net_validated[i].name);
    if(net_validated[i].pins!=NULL) free(net_validated[i].pins);
  }
  free(net_validated);
  // Then, start the copy
  num_nets_validated = num_nets_current;
  net_validated = (struct s_net *) malloc(num_nets_current*sizeof(struct s_net));
  memcpy(net_validated, net_current, num_nets_current * sizeof(struct s_net));
  
  for(i=0; i<num_nets_current; i++)
  {
    (net_validated+i)->name=(char*) malloc((strlen((net_current+i)->name)+1)*sizeof(char));
    memcpy((net_validated+i)->name, (net_current+i)->name,(strlen((net_current+i)->name)+1)*sizeof(char));
            
    (net_validated+i)->pins=(int*) malloc((net_current+i)->num_pins*sizeof(int));
    memcpy((net_validated+i)->pins, (net_current+i)->pins,(net_current+i)->num_pins*sizeof(int));  
  }
        
  // Store the matrix contents
  for(i=0; i<num_clusters; i++)
  {
    memcpy(matrix_contents[i], matrix_contents_tmp[i], arch.adjm_size * sizeof(int));
  }
}

// MAIN ROUTINE
// Does the actual work of clustering multiple Gates+FF logic blocks into clusters.
// Matrix charactistics: matrix_width, matrix_depth, matrix_topo
// Cell characteristics: Cell size
// Clock management: global_clocks, is_clock
// Output data management: out_fname, echo_dot
void do_clustering (int matrix_width, int matrix_depth, int matrix_topo, int cell_size, boolean global_clocks, boolean *is_clock, char *out_fname, boolean echo_dot) {

 int num_clusters, istart, i;
 int inputs_used, outputs_used, cells_used, clocks_used, next_blk;
 
 num_clusters = 0;
 buffer_counting_validated= num_blocks;
 buffer_counting_current = num_blocks;
 buffer_counting_rollback = num_blocks;
 
 check_clocks (is_clock);

 // Architecture setup
 init_architecture(&arch);
 create_architecture(&arch, matrix_topo, matrix_width, matrix_depth);

 // Clustering setup
 alloc_and_init_clustering (cell_size, number_of_nets(&arch));
 alloc_matrix_contents (num_clusters, &arch);
 
#ifdef DEBUG
 echo_clustering_info ("./WORK/init_clustering.echo");
 echo_clustering_validated_info ("./WORK/init_clustering_validated.echo");
#endif
 
 // Start the clustering with the initial seed gate (the one with the largest count of external inputs)
 istart = get_free_block_with_most_ext_inputs (cell_size, number_of_inputs(&arch), num_clusters, echo_dot);

 if(istart==NO_CLUSTER)
 {
    fprintf(stderr, "ERROR : Seed unmappable\n");
    exit(EXIT_FAILURE);
 }

 // Loop until no more gates has to be mapped
 while (istart != NO_CLUSTER) {
   
#ifdef DEBUG
    printf("\n\n");
#endif
    printf("\n\nNEW CLUSTER - %d\n",num_clusters);
#ifdef DEBUG
    printf("INITIAL BLOCK - %d\n",istart);
#endif

    // Initialize the cluster
    reset_cluster(&inputs_used, &outputs_used, &cells_used, &clocks_used);
    num_clusters++;
    
    // Add the initial cluster seed
    add_to_cluster(istart, num_clusters - 1, is_clock, global_clocks, &inputs_used, &outputs_used, &cells_used);

    next_blk = get_gate_for_matrix (number_of_inputs(&arch) - inputs_used, cell_size, cells_used, outputs_used, is_clock, num_clusters-1, matrix_contents, echo_dot);

#ifdef VERBOSE_DEBUG
    printf("NEXT BLOCK - %d\n",next_blk);
#endif

    // Identify and pack subsequent blocks into the cluster.
    while (next_blk != NO_CLUSTER) {
      add_to_cluster(next_blk, num_clusters - 1, is_clock, global_clocks, &inputs_used, &outputs_used, &cells_used);

      next_blk = get_gate_for_matrix (number_of_inputs(&arch) - inputs_used, cell_size, cells_used, outputs_used, is_clock, num_clusters-1, matrix_contents, echo_dot);

#ifdef VERBOSE_DEBUG
    printf("NEXT BLOCK - %d\n",next_blk);
#endif
    }
    
    // At this point, the current cluster is fully packed. Before moving to the next cluster, we tranfer the current working copy of the mapping to the validated copy.
    validate_mapping_copy(num_clusters);
 
    // Prepare the mapping data structure for the next cluster
    alloc_matrix_contents (num_clusters, &arch);
          
    // Find a seed for the next cluster
    istart = get_free_block_with_most_ext_inputs (cell_size, number_of_inputs(&arch),num_clusters, echo_dot);                
 }

  // Clustering/Mapping steps are done
  for(i=0; i<num_clusters; i++)
  {
    printf("CLUSTER %d: Matrix_content\n",i);
    print_table(matrix_contents[i], arch.adjm_size);
  }
   
#ifdef DEBUG
//  echo_clustering_info ("output_clustering.echo");
 echo_clustering_validated_info ("./WORK/output_clustering_validated.echo");
 echo_debug_validated ("Debug echo", "./WORK/debug_validated.echo");
#endif

 // Output the current mapping results
 print_blif_cluster (&arch, num_clusters, matrix_contents, global_clocks, is_clock, out_fname); // Netlist of clusters
 print_blif_gate (&arch, num_clusters, matrix_contents, global_clocks, is_clock, out_fname); // Netlist of gates
 print_blif_names (&arch, num_clusters, matrix_contents, global_clocks, is_clock, out_fname); // Equivalent netlist of LUTs
 print_rpt_mapping (&arch, num_clusters, matrix_contents, global_clocks, is_clock, out_fname); // Internal matrix mapping


 // Output the mapping statistics
 mapping_stat (& arch, num_clusters, matrix_contents);
 
 // Clean up the structures
#ifdef DEBUG
 printf("DO_CLUSTERING : Free structures\n");
#endif

 if(matrix_contents_tmp!=NULL)
 {
   for(i=0; i<(num_clusters+1); i++)
      if(matrix_contents_tmp[i]!=NULL) free(matrix_contents_tmp[i]);
   free(matrix_contents_tmp);
 }

 if(matrix_contents!=NULL)
 {
   for(i=0; i<(num_clusters+1); i++)
      if(matrix_contents[i]!=NULL) free(matrix_contents[i]);
   free(matrix_contents);
 }

 free_clustering();
 free_architecture(&arch);
}
