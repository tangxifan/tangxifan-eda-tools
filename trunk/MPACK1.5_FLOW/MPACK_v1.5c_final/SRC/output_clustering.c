// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon
// Output clustering informations (netlist, mappings, ...)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mpack.h"
#include "globals.h"
#include "output_clustering.h"
#include "function.h"
#include "access.h"
#include "matrix_architecture.h"

// Define the maximum number of characters per line
#define LINELENGTH 2000

//////////////////////////////////////
// Utility and Debug
//////////////////////////////////////

// Echo back the current netlist data structures to echo_file to allow the user to look at the internal state of the program.
void echo_debug_current (char *blif_file, char *echo_file) 
{
 int i, j; 
 FILE *fp;

 printf("Input netlist file: %s\n", blif_file);
 printf("Primary Inputs: %d.  Primary Outputs: %d.\n", num_p_inputs, num_p_outputs);
 printf("Gates: %d.  Latches: %d.\n", num_gates, num_latches);
 printf("Total Blocks: %d.  Total Nets: %d\n", num_blocks_current, num_nets_current);
 
 fp = fopen (echo_file, "w"); 

 fprintf(fp,"Input netlist file: %s\n",blif_file);
 fprintf(fp,"num_p_inputs: %d, num_p_outputs: %d, num_gates: %d, num_latches: %d\n",num_p_inputs,num_p_outputs,num_gates, num_latches);
 fprintf(fp,"num_blocks: %d, num_nets_current: %d\n",num_blocks_current,num_nets_current);

 fprintf(fp,"\nNet\tName\t\t#Pins\tDriver\tRecvs.\n");
 for (i=0;i<num_nets_current;i++) {
    fprintf(fp,"\n%d\t%s\t", i, net_current[i].name);
    if (strlen(net_current[i].name) < 8)
       fprintf(fp,"\t"); // Name field is 16 chars wide
    fprintf(fp,"%d", net_current[i].num_pins);
    for (j=0;j<net_current[i].num_pins;j++) 
        fprintf(fp,"\t%d",net_current[i].pins[j]);
 }

 fprintf(fp,"\n\n\nBlocks\t\t\tBlock Type Legend:\n");
 fprintf(fp,"\t\t\tINPAD = %d\tOUTPAD = %d\n", INPAD, OUTPAD);
 fprintf(fp,"\t\t\tGATE = %d\t\tLATCH = %d\n", GATE, LATCH);
 fprintf(fp,"\t\t\tEMPTY = %d\t\n\n", EMPTY);
 
 fprintf(fp,"\n\n\nGATE\t\t\tGATE Type Legend:\n");
 fprintf(fp,"\t\t\tNONE = %d\tZERO = %d\tONE = %d\n", NONE, ZERO, ONE);
 fprintf(fp,"\t\t\tINV1A = %d\tINV1B = %d\tBUF1A = %d\tBUF1B = %d\n", INV1A, INV1B, BUF1A, BUF1B);
 fprintf(fp,"\t\t\tAND2 = %d\tNAND2 = %d\tOR2 = %d\tNOR2 = %d\n", AND2, NAND2, OR2, NOR2);
 fprintf(fp,"\t\t\tNOTAND2 = %d\tANDNOT2 = %d\tNOTOR2 = %d\tORNOT2 = %d\n", NOTAND2, ANDNOT2, NOTOR2, ORNOT2);
 
 fprintf(fp,"\nBlock\tName\t\tType\tGate_Type\t#Nets\tOutput\tInputs\t");
 fprintf(fp,"Clock\n\n");

 for (i=0;i<num_blocks_current;i++) { 
    fprintf(fp,"\n%d\t%s\t",i, block_current[i].name);
    if (strlen(block_current[i].name) < 8)
       fprintf(fp,"\t"); // Name field is 16 chars wide
    fprintf(fp,"%d\t%d\t%d", block_current[i].type, block_current[i].gate_type, block_current[i].num_nets);

    if (block_current[i].type == INPAD || block_current[i].type == OUTPAD || block_current[i].type == LATCH || block_current[i].type==GATE || block_current[i].type==BUF)
    {
       for(j=block_current[i].num_in;j<block_current[i].num_out+block_current[i].num_in;j++)
          fprintf(fp,"\to:%d",block_current[i].nets[j]);
       for(j=0;j<block_current[i].num_in;j++)
          fprintf(fp,"\ti:%d",block_current[i].nets[j]);

       if(block_current[i].type == LATCH) {
         for(j=block_current[i].num_out+block_current[i].num_in;j<block_current[i].num_out+block_current[i].num_in+1;j++)
         fprintf(fp,"\tc:%d", block_current[i].nets[j]);
       }
   }
   else
   {
       if(block[i].type == EMPTY)
         printf("Warning: Block %d is unused (Type EMPTY)\n", i);
       else
       {
         fprintf(stderr, "echo_input: Wrong block type\n");
         exit(EXIT_FAILURE);
       }
   }
 }

 fprintf(fp,"\n");
 fclose(fp);
}

// Echo back the validated netlist data structures to echo_file to allow the user to look at the internal state of the program.
void echo_debug_validated (char *blif_file, char *echo_file) 
{
 int i, j; 
 FILE *fp;

 printf("Input netlist file: %s\n", blif_file);
 printf("Primary Inputs: %d.  Primary Outputs: %d.\n", num_p_inputs, num_p_outputs);
 printf("Gates: %d.  Latches: %d.\n", num_gates, num_latches);
 printf("Total Blocks: %d.  Total Nets: %d\n", num_blocks_validated, num_nets_validated);
 
 fp = fopen (echo_file, "w"); 

 fprintf(fp,"Input netlist file: %s\n",blif_file);
 fprintf(fp,"num_p_inputs: %d, num_p_outputs: %d, num_luts: %d, num_latches: %d\n",num_p_inputs,num_p_outputs,num_gates, num_latches);
 fprintf(fp,"num_blocks: %d, num_nets_current: %d\n",num_blocks_validated,num_nets_validated);

 fprintf(fp,"\nNet\tName\t\t#Pins\tDriver\tRecvs.\n");
 for (i=0;i<num_nets_validated;i++) {
    fprintf(fp,"\n%d\t%s\t", i, net_validated[i].name);
    if (strlen(net_validated[i].name) < 8)
       fprintf(fp,"\t"); // Name field is 16 chars wide
    fprintf(fp,"%d", net_validated[i].num_pins);
    for (j=0;j<net_validated[i].num_pins;j++) 
        fprintf(fp,"\t%d",net_validated[i].pins[j]);
 }

 fprintf(fp,"\n\n\nBlocks\t\t\tBlock Type Legend:\n");
 fprintf(fp,"\t\t\tINPAD = %d\tOUTPAD = %d\n", INPAD, OUTPAD);
 fprintf(fp,"\t\t\tGATE = %d\t\tLATCH = %d\n", GATE, LATCH);
 fprintf(fp,"\t\t\tEMPTY = %d\t\n\n", EMPTY);
 
 fprintf(fp,"\n\n\nGATE\t\t\tGATE Type Legend:\n");
 fprintf(fp,"\t\t\tNONE = %d\tZERO = %d\tONE = %d\n", NONE, ZERO, ONE);
 fprintf(fp,"\t\t\tINV1A = %d\tINV1B = %d\tBUF1A = %d\tBUF1B = %d\n", INV1A, INV1B, BUF1A, BUF1B);
 fprintf(fp,"\t\t\tAND2 = %d\tNAND2 = %d\tOR2 = %d\tNOR2 = %d\n", AND2, NAND2, OR2, NOR2);
 fprintf(fp,"\t\t\tNOTAND2 = %d\tANDNOT2 = %d\tNOTOR2 = %d\tORNOT2 = %d\n", NOTAND2, ANDNOT2, NOTOR2, ORNOT2);
 
 fprintf(fp,"\nBlock\tName\t\tType\tGate_Type\t#Nets\tOutput\tInputs\t");
 fprintf(fp,"Clock\n\n");

 for (i=0;i<num_blocks_validated;i++) { 
    fprintf(fp,"\n%d\t%s\t",i, block_validated[i].name);
    if (strlen(block_validated[i].name) < 8)
       fprintf(fp,"\t"); // Name field is 16 chars wide
    fprintf(fp,"%d\t%d\t%d", block_validated[i].type, block_validated[i].gate_type, block_validated[i].num_nets);

    if (block_validated[i].type == INPAD || block_validated[i].type == OUTPAD || block_validated[i].type == LATCH || block_validated[i].type==GATE || block_validated[i].type==BUF)
    {
       for(j=block_validated[i].num_in;j<block_validated[i].num_out+block_validated[i].num_in;j++)
          fprintf(fp,"\to:%d",block_validated[i].nets[j]);
       for(j=0;j<block_validated[i].num_in;j++)
          fprintf(fp,"\ti:%d",block_validated[i].nets[j]);

       if(block_validated[i].type == LATCH) {
         for(j=block_validated[i].num_out+block_validated[i].num_in;j<block_validated[i].num_out+block_validated[i].num_in+1;j++)
         fprintf(fp,"\tc:%d", block_validated[i].nets[j]);
       }
   }
   else
   {
       if(block[i].type == EMPTY)
         printf("Warning: Block %d is unused (Type EMPTY)\n", i);
       else
       {
         fprintf(stderr, "echo_input: Wrong block type\n");
         exit(EXIT_FAILURE);
       }
   }
 }

 fprintf(fp,"\n");
 fclose(fp);
}

// Prints string without making any lines longer than LINELENGTH.
// Column points to the column in which the next character will go (both used and updated), and fpout points to the output file.
static void print_string (char *str_ptr, int *column, FILE *fpout) {
 int len;

 len = strlen(str_ptr);
 if (len + 3 > LINELENGTH) {
     fprintf(stderr, "Error in print_string: String %s is too long for desired maximum line length.\n", str_ptr);
     exit(EXIT_FAILURE);
 }
 
 if (*column + len + 2 > LINELENGTH) {
    fprintf(fpout,"\\\n");
    *column = 1;
 }
 
 fprintf(fpout,"%s ",str_ptr);
 *column += len + 1;
}

//////////////////////////////////////
// Matrix cluster level output
//////////////////////////////////////

// Prints the MATRIX model (input/output mapping structure).
void print_matrix_model (FILE *fpout, matrix *arch)
{
   int i;
   int column, len;
   char buffer[LINELENGTH];

   fprintf(fpout,".model MATRIX\n");

   fprintf(fpout,".inputs ");
   column=8;
   for(i=0; i<number_of_inputs(arch); i++)
   {
      len=sprintf(buffer,"I[%d] ",i);
      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in print_matrix_model: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
   fprintf(fpout,"\n");

   fprintf(fpout,".outputs ");
   column=9;
   for(i=0; i<number_of_outputs(arch); i++)
   {
      len=sprintf(buffer,"O[%d] ",i);
      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in print_matrix_model: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
   fprintf(fpout,"\n");

   fprintf(fpout,".blackbox\n");
   fprintf(fpout,".end\n");
}

// Prints the output netlist in terms of MATRIX subckt elements.
void print_matrix_cluster (FILE *fpout, matrix *arch, int num_clusters, int **matrix_contents)
{
   int cnum, bnum, idx, onet_idx, offset;

   int column, len=0;
   char buffer[LINELENGTH];

   for(cnum=0; cnum<num_clusters; cnum++)
   {
      fprintf(fpout,".subckt MATRIX ");
      column=14;

      for(bnum=0; bnum<number_of_inputs(arch); bnum++)
      {
         idx=matrix_contents[cnum][bnum];
         if(idx<0) len=sprintf(buffer,"I[%d]=unconn", bnum);
         else
         {
            onet_idx=block_validated[idx].nets[block_validated[idx].num_in];
            len=sprintf(buffer,"I[%d]=%s",bnum,net_validated[onet_idx].name);
         }

         if (len+1 > LINELENGTH) {
            fprintf(stderr, "Error in print_matrix_cluster: Buffer too short\n");
            exit(EXIT_FAILURE);
         }

         print_string(buffer, &column,fpout);

      }

      offset=number_of_inputs(arch)+number_of_nodes(arch);
      for(bnum=0; bnum<number_of_outputs(arch); bnum++)
      {
         idx=matrix_contents[cnum][offset+bnum];

         if(idx<0) len=sprintf(buffer,"O[%d]=unconn", bnum);
         else
         {
            onet_idx=block_validated[idx].nets[block_validated[idx].num_in];
            len=sprintf(buffer,"O[%d]=%s",bnum,net_validated[onet_idx].name);
         }

         if (len+1 > LINELENGTH) {
            fprintf(stderr, "Error in print_matrix_cluster: Buffer too short\n");
            exit(EXIT_FAILURE);
         }

         print_string(buffer, &column,fpout);
      }
      fprintf(fpout,"\n");
   }
   fprintf(fpout,"\n");
}

// Prints the global output netlist (including input, output, latch and MATRIX elements)
void print_blif_cluster (matrix *arch, int num_clusters, int **matrix_contents, boolean global_clocks, boolean *is_clock, char *out_fname) {

 FILE *fpout;
 int bnum, netnum;
 int column, len;
 char buffer[LINELENGTH];
 time_t t;

 sprintf(buffer, "%s_matrix.blif", out_fname);
 fpout = fopen (buffer, "w");

 time ( &t );

 // Start with the header
 fprintf(fpout,"# Benchmark written by MPACK on %s", ctime(&t));
 fprintf(fpout,".model unknown\n");

 // Print out the input pads
 fprintf(fpout,".inputs ");
 column=8;
 for (bnum=0;bnum<num_blocks_validated;bnum++) {
   if(block_validated[bnum].type==INPAD)
   {
      len=sprintf(buffer,"%s", block_validated[bnum].name);

      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in output_matrix_clustering: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
 }
 fprintf(fpout,"\n");

 // Print out the output pads
 fprintf(fpout,".outputs ");
 column=9;
 for (bnum=0;bnum<num_blocks_validated;bnum++) {
   if(block_validated[bnum].type==OUTPAD)
   {
      len=sprintf(buffer,"%s", block_validated[bnum].name);

      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in output_matrix_clustering: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
 }
 fprintf(fpout,"\n");

 // Print out the global nets
 if (global_clocks) {
    fprintf(fpout,".global ");
    column=8;
    for (netnum=0;netnum<num_nets;netnum++) {
       if (is_clock[netnum])
       {
          len=sprintf(buffer,"%s", net[netnum].name);

          if (len+1 > LINELENGTH) {
            fprintf(stderr, "Error in output_matrix_clustering: Buffer too short\n");
            exit(EXIT_FAILURE);
          }
          print_string(buffer, &column,fpout);
       }
    }

 fprintf(fpout,"\n");
 }

 fprintf(fpout,"\n");

 // Print out the latch
 for (bnum=0;bnum<num_blocks;bnum++) {
   if(block[bnum].type==LATCH)
   {
      len=sprintf(buffer,".latch %s %s re %s 0\n", net[block[bnum].nets[0]].name, net[block[bnum].nets[1]].name, net[block[bnum].nets[2]].name);

      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in output_matrix_clustering: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
 }
 fprintf(fpout,"\n");

  print_matrix_cluster (fpout, arch, num_clusters, matrix_contents);

 fprintf(fpout,".end\n");
 fprintf(fpout,"\n");

 print_matrix_model (fpout, arch);


 fclose (fpout);
}

//////////////////////////////////////
// Gate level output
//////////////////////////////////////

// Prints the output netlist in terms of CELL subckt elements (GATE level).
void print_cell (FILE *fpout, matrix *arch, int num_clusters, int **matrix_contents)
{
   int cnum, pnum, idx;

   int column, len, offset;
   char buffer[LINELENGTH];
   
   for(cnum=0; cnum<num_blocks_validated; cnum++)
   {  
      if(block_validated[cnum].type==GATE || block_validated[cnum].type==BUF)
      {
         fprintf(fpout,".subckt CELL ");
         column=14;

         for(pnum=0; pnum<block_validated[cnum].num_in; pnum++)
         {
            idx=block_validated[cnum].nets[pnum];

            if(idx<0) len=sprintf(buffer,"I[%d]=unconn", pnum);
            else len=sprintf(buffer,"I[%d]=%s",pnum,net_validated[idx].name);
            
            if (len+1 > LINELENGTH) {
              fprintf(stderr, "Error in print_matrix_cluster: Buffer too short\n");
              exit(EXIT_FAILURE);
            }
            
            print_string(buffer, &column,fpout);
         }
         
         for(pnum=0; pnum<block_validated[cnum].num_out; pnum++)
         {
            offset=block_validated[cnum].num_in;
            idx=block_validated[cnum].nets[offset+pnum];
            if(idx<0) len=sprintf(buffer,"O[%d]=unconn", pnum);
            else len=sprintf(buffer,"O[%d]=%s",pnum,net_validated[idx].name);
            
            if (len+1 > LINELENGTH) {
              fprintf(stderr, "Error in print_matrix_cluster: Buffer too short\n");
              exit(EXIT_FAILURE);
            }
            
            print_string(buffer, &column,fpout);
         }
         fprintf(fpout,"\n");
      }     
   }
   fprintf(fpout,"\n");
}

// Prints the CELL model (input/output mapping structure).
void print_cell_model (FILE *fpout, matrix *arch)
{
   int i;
   int column, len;
   char buffer[LINELENGTH];

   fprintf(fpout,".model CELL\n");

   fprintf(fpout,".inputs ");
   column=8;
   for(i=0; i<GATE_MAX_INPUT_NB; i++)
   {
      len=sprintf(buffer,"I[%d] ",i);
      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in print_matrix_model: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
   fprintf(fpout,"\n");

   fprintf(fpout,".outputs ");
   column=9;
   for(i=0; i<GATE_MAX_OUTPUT_NB; i++)
   {
      len=sprintf(buffer,"O[%d] ",i);
      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in print_matrix_model: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
   fprintf(fpout,"\n");

   fprintf(fpout,".blackbox\n");
   fprintf(fpout,".end\n");
}

// Prints the global output netlist (including input, output, latch and CELL elements)
void print_blif_gate (matrix *arch, int num_clusters, int **matrix_contents, boolean global_clocks, boolean *is_clock, char *out_fname) {

 FILE *fpout;
 int bnum, netnum;
 int column, len;
 char buffer[LINELENGTH];
 time_t t;
 
 sprintf(buffer, "%s_formatted.blif", out_fname);
 fpout = fopen (buffer, "w");

 time ( &t );

 // Start with the header
 fprintf(fpout,"# Benchmark written by MPACK on %s", ctime(&t));
 fprintf(fpout,".model unknown\n");

 // Print out the input pads
 fprintf(fpout,".inputs ");
 column=8;
 for (bnum=0;bnum<num_blocks_validated;bnum++) {
   if(block_validated[bnum].type==INPAD)
   {
      len=sprintf(buffer,"%s", block_validated[bnum].name);

      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in output_matrix_clustering: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
 }
 fprintf(fpout,"\n");

 // Print out the output pads
 fprintf(fpout,".outputs ");
 column=9;
 for (bnum=0;bnum<num_blocks_validated;bnum++) {
   if(block_validated[bnum].type==OUTPAD)
   {
      len=sprintf(buffer,"%s", block_validated[bnum].name);

      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in output_matrix_clustering: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
 }
 fprintf(fpout,"\n");

 // Print out the global nets
 if (global_clocks) {
    fprintf(fpout,".global ");
    column=8;
    for (netnum=0;netnum<num_nets;netnum++) {
       if (is_clock[netnum])
       {
          len=sprintf(buffer,"%s", net[netnum].name);

          if (len+1 > LINELENGTH) {
            fprintf(stderr,"Error in output_matrix_clustering: Buffer too short\n");
            exit(EXIT_FAILURE);
          }
          print_string(buffer, &column,fpout);
       }
    }

 fprintf(fpout,"\n");
 }

 fprintf(fpout,"\n");

 // Print out the latch
 for (bnum=0;bnum<num_blocks;bnum++) {
   if(block[bnum].type==LATCH)
   {
      len=sprintf(buffer,".latch %s %s re %s 0\n", net[block[bnum].nets[0]].name, net[block[bnum].nets[1]].name, net[block[bnum].nets[2]].name);

      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in output_matrix_clustering: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
 }
 fprintf(fpout,"\n");
 
 print_cell (fpout, arch, num_clusters, matrix_contents);

 fprintf(fpout,".end\n");
 fprintf(fpout,"\n");

 print_cell_model (fpout, arch);


 fclose (fpout);
}

//////////////////////////////////////
// Names output
//////////////////////////////////////

// Prints the output netlist in terms of names elements (updated input netlist with additional buffers).
void print_names (FILE *fpout, matrix *arch, int num_clusters, int **matrix_contents)
{
   int cnum, pnum, idx;

   int offset;
   
   for(cnum=0; cnum<num_blocks_validated; cnum++)
   {  
      if(block_validated[cnum].type==GATE || block_validated[cnum].type==BUF)
      {
         fprintf(fpout,".names ");
         
         for(pnum=0; pnum<block_validated[cnum].num_in; pnum++)
         {
            idx=block_validated[cnum].nets[pnum];

            if(idx>0) fprintf(fpout,"%s ",net_validated[idx].name);
         }
         
         for(pnum=0; pnum<block_validated[cnum].num_out; pnum++)
         {
            offset=block_validated[cnum].num_in;
            idx=block_validated[cnum].nets[offset+pnum];
            if(idx>0) fprintf(fpout,"%s",net_validated[idx].name);
         }
         fprintf(fpout,"\n");
         
         switch(block_validated[cnum].gate_type)
         {
           case NONE:   fprintf(stderr, "GATE or BUF with no type!\n");
                        exit(EXIT_FAILURE);
                        break;
           case ZERO:   fprintf(fpout,"0\n"); /// TO DO: handle a dynamic number of inputs
                        break;
           case ONE:    fprintf(fpout,"1\n"); /// TO DO: Same as previous
                        break;
           case INV1A:  fprintf(fpout,"0 1\n");
                        break;                          
           case INV1B:  fprintf(fpout,"0 1\n");
                        break;                        
           case BUF1A:  fprintf(fpout,"1 1\n");
                        break;                        
           case BUF1B:  fprintf(fpout,"1 1\n");
                        break;                        
           case AND2:   fprintf(fpout,"11 1\n");
                        break;                        
           case NAND2:  fprintf(fpout,"00 1\n");
                        fprintf(fpout,"01 1\n");
                        fprintf(fpout,"10 1\n");
                        break;                                             
           case OR2:    fprintf(fpout,"11 1\n");
                        fprintf(fpout,"01 1\n");
                        fprintf(fpout,"10 1\n");
                        break;
           case NOR2:   fprintf(fpout,"00 1\n");
                        break;
           case NOTAND2:fprintf(fpout,"01 1\n");
                        break;
           case ANDNOT2:fprintf(fpout,"10 1\n");
                        break;                       
           case NOTOR2: fprintf(fpout,"00 1\n");
                        fprintf(fpout,"01 1\n");
                        fprintf(fpout,"11 1\n");
                        break;                        
           case ORNOT2: fprintf(fpout,"00 1\n");
                        fprintf(fpout,"10 1\n");
                        fprintf(fpout,"11 1\n");
                        break;                      
           case XOR2:   fprintf(fpout,"01 1\n");
                        fprintf(fpout,"10 1\n");
                        break;                        
           case XNOR2:  fprintf(fpout,"00 1\n");
                        fprintf(fpout,"11 1\n");
                        break;                           
           default:     fprintf(stderr, "GATE or BUF with no type!\n");
                        exit(EXIT_FAILURE);
                        break; 
         }
         
         fprintf(fpout,"\n");
      }     
   }
   fprintf(fpout,"\n");
}

// Prints the global blif output netlist (including input, output, latch and NAMES elements)
void print_blif_names (matrix *arch, int num_clusters, int **matrix_contents, boolean global_clocks, boolean *is_clock, char *out_fname) {

 FILE *fpout;
 int bnum, netnum;
 int column, len;
 char buffer[LINELENGTH];
 time_t t;
 
 sprintf(buffer, "%s_formatted_names.blif", out_fname);
 fpout = fopen (buffer, "w");

 time ( &t );

 // Start with the header
 fprintf(fpout,"# Benchmark written by MPACK on %s", ctime(&t));
 fprintf(fpout,".model unknown\n");

 // Print out the input pads
 fprintf(fpout,".inputs ");
 column=8;
 for (bnum=0;bnum<num_blocks_validated;bnum++) {
   if(block_validated[bnum].type==INPAD)
   {
      len=sprintf(buffer,"%s", block_validated[bnum].name);

      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in output_matrix_clustering: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
 }
 fprintf(fpout,"\n");

 // Print out the output pads
 fprintf(fpout,".outputs ");
 column=9;
 for (bnum=0;bnum<num_blocks_validated;bnum++) {
   if(block_validated[bnum].type==OUTPAD)
   {
      len=sprintf(buffer,"%s", block_validated[bnum].name);

      if (len+1 > LINELENGTH) {
         fprintf(stderr, "Error in output_matrix_clustering: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
 }
 fprintf(fpout,"\n");

 // Print out the global nets
 if (global_clocks) {
    fprintf(fpout,".global ");
    column=8;
    for (netnum=0;netnum<num_nets;netnum++) {
       if (is_clock[netnum])
       {
          len=sprintf(buffer,"%s", net[netnum].name);

          if (len+1 > LINELENGTH) {
            fprintf(stderr,"Error in output_matrix_clustering: Buffer too short\n");
            exit(EXIT_FAILURE);
          }
          print_string(buffer, &column,fpout);
       }
    }

 fprintf(fpout,"\n");
 }

 fprintf(fpout,"\n");

 // Print out the latch
 for (bnum=0;bnum<num_blocks;bnum++) {
   if(block[bnum].type==LATCH)
   {
      len=sprintf(buffer,".latch %s %s re %s 0\n", net[block[bnum].nets[0]].name, net[block[bnum].nets[1]].name, net[block[bnum].nets[2]].name);

      if (len+1 > LINELENGTH) {
         fprintf(stderr,"Error in output_matrix_clustering: Buffer too short\n");
         exit(EXIT_FAILURE);
      }
      print_string(buffer, &column,fpout);
   }
 }
 fprintf(fpout,"\n");
 
 print_names (fpout, arch, num_clusters, matrix_contents);

 fprintf(fpout,".end\n");
 fprintf(fpout,"\n");

 fclose (fpout);
}

//////////////////////////////////////
// Mapping output
//////////////////////////////////////

// Prints the internal mapping information report for each cluster
void print_rpt_mapping (matrix *arch, int num_clusters, int **matrix_contents, boolean global_clocks, boolean *is_clock, char *out_fname) {

 FILE *fpout;
 int cnum, cell_offset, block_id, output_net_id;
//  int lnum;
 int i,j,k;
 char buffer[LINELENGTH];
 time_t t;
 
 sprintf(buffer, "%s_mapped.rpt", out_fname);
 fpout = fopen (buffer, "w");

 time ( &t );

 // Start with the header
 fprintf(fpout,"# Mapping report written by MPACK on %s", ctime(&t));
 fprintf(fpout,"# Model unknown\n");

 // Print out the general arch parameters
 fprintf(fpout,"matrix_width = %d\n", arch->width);
 fprintf(fpout,"matrix_depth = %d\n", arch->depth);
 fprintf(fpout,"cell_size = %d\n", GATE_MAX_INPUT_NB); 
 
 // Print out the interconnection scheme
//  fprintf(fpout,"# Interconnection scheme for each layer\n");
//  fprintf(fpout,"layer\n");
// 
//  for(lnum=0; lnum<arch->width-1; lnum++)
//  {
//     fprintf(fpout,"# Layer %d : Cross-connectivity matrix X%d%d\n", lnum+1, lnum, lnum+1);
// 
//     for(i=0; i<arch->width;i++)
//     {
//        cell_offset=(arch->width*GATE_MAX_INPUT_NB);
//        fprintf(fpout,"X[%d][%d] = ",lnum+1,i);
//        
//        for(j=0; j<arch->width;j++)
//           fprintf(fpout,"%d, ",arch->adjacency_matrix[cell_offset+i][cell_offset+arch->width+j]);
//        
//        fseek(fpout, -2, SEEK_CUR);
//        fprintf(fpout,"\n");
//     }
//  }
//  fprintf(fpout,"end\n");
 
 fprintf(fpout,"# Detailled interconnection scheme for each layer valid only for 2 input cells\n");
 fprintf(fpout,"layer_detailed\n");
 
 cell_offset=arch->width*GATE_MAX_INPUT_NB;
 for(i=cell_offset+arch->width; i<arch->width*(arch->depth+GATE_MAX_INPUT_NB); i++)
 {
    fprintf(fpout,"X[%d].I[0] = %d\n",i-cell_offset, arch->terminal_interconnect_matrix[i][1]-cell_offset);
    fprintf(fpout,"X[%d].I[1] = %d\n",i-cell_offset, arch->terminal_interconnect_matrix[i][2]-cell_offset);
 }
 
 fprintf(fpout,"end\n");

  // Print out the BLE connectivity scheme
 fprintf(fpout,"# BLE connectivity\n");
 fprintf(fpout,"mclusters_number =%d\n", num_clusters);

 for(cnum=0; cnum<num_clusters; cnum++)
 {
    fprintf(fpout,"\nMCluster\n");
   
    cell_offset=arch->width*GATE_MAX_INPUT_NB;
    for(i=0;i<arch->depth;i++)
    {
      for(j=0; j<arch->width; j++)
      {
         fprintf(fpout,"cell[%d][%d] = ",i, j);
         block_id=matrix_contents[cnum][cell_offset+(i*arch->width)+j];
         
         if(block_id<0) // Cell not used
         {
            fprintf(fpout,"unconn,unconn,open");
         }
         else
         {
            for(k=0; k<block_validated[block_id].num_in; k++)
            {
              if(block_validated[block_id].nets[k]==-1)
                  fprintf(fpout,"unconn,");
              else
                  fprintf(fpout,"conn,");
            }
            
            output_net_id=block_validated[block_id].nets[block_validated[block_id].num_in];
            fprintf(fpout,"%s",net_validated[output_net_id].name);
         }
         fprintf(fpout,"\n");
      }
    }
    fprintf(fpout,"end\n");
 }
 fclose (fpout);
}

//////////////////////////////////////
// Statistics
//////////////////////////////////////

// Prints the mapping statistics
void mapping_stat (matrix *arch, int num_clusters, int **matrix_contents) {
 int cnum, cell_offset, block_id, count, global_count, buffer_count, global_buffer_count;
 int i,j;
 
 // Start with the header
 printf("\n\nMapping Efficiency Statistics\n\n");

 global_count=0;
 global_buffer_count=0;
 for(cnum=0; cnum<num_clusters; cnum++)
 {
    cell_offset=arch->width*GATE_MAX_INPUT_NB;
    count=0;
    buffer_count=0;
    for(i=0;i<arch->depth;i++)
    {
      for(j=0; j<arch->width; j++)
      {
         block_id=matrix_contents[cnum][cell_offset+(i*arch->width)+j]; 
         if(block_id>=0)
         {
           count++;
           if(block_validated[block_id].gate_type==BUF1A || block_validated[block_id].gate_type==BUF1B)
             buffer_count++;
         }
      }
    }
    global_count+=count;
    global_buffer_count+=buffer_count;
#ifdef DEBUG
     printf("Mapping of cluster %d: buffer_count %d --- occupancy %d/%d --- efficiency %f ---  occupancy wo buf %d/%d --- efficiency wo buf %f\n",cnum, buffer_count, count, arch->depth*arch->width, (double)count/(double)(arch->depth*arch->width), count-buffer_count, arch->depth*arch->width, (double)(count-buffer_count)/(double)(arch->depth*arch->width));
#endif
 }
 printf("Global mapping efficiency: %d/%d\n--- efficiency: %f\n--- occupancy wo buf: %d/%d\n--- efficiency wo buf: %f\n",global_count, num_clusters*arch->depth*arch->width, (double)global_count/(double)(num_clusters*arch->depth*arch->width), global_count-global_buffer_count, num_clusters*arch->depth*arch->width, (double)(global_count-global_buffer_count)/(double)(num_clusters*arch->depth*arch->width));
}
