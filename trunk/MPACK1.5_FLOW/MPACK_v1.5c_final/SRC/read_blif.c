// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon

// Input netlist read and data structures allocation

// This source file will read in a FLAT blif netlist consisting of .inputs, .outputs, .gates and .latch commands.
// The design should be mapped on a library of standard cells, according to the gate names given in mpack.h.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mpack.h"
#include "globals.h"
#include "read_blif.h"

static int *num_driver, *temp_num_pins;

// # of .input, .output, .model and .end lines
static int linenum, ilines, olines, model_lines, endlines;  
static struct hash_nets **hash;
static FILE *blif;
static int cont;

// Prototypes
static int add_net (char *ptr, int type, int bnum, int doall); 
static void get_tok(char *buffer, int pass, int doall, int *done);
static void init_parse(int doall);
static void check_net (int cell_size);
static void free_parse (void);
static void io_line (int in_or_out, int doall);
static void add_gate (int doall);
static void add_latch (int doall);
static void dum_parse (char *buf);
static int hash_value (char *name);
static void low_fanout_gate_formatting();
char *my_fgets(char *buf, int max_size, FILE *fp);
char *my_strtok(char *ptr, char *tokens, FILE *fp, char *buf);

// Main read routing
// Three passes to ensure inputs are first blocks, outputs second and gates and latches third.
// Just makes the output netlist more readable.
void read_blif (char *blif_file, int cell_size) {
 char buffer[BUFSIZE];
 int pass, done, doall, i;
 
 blif = NULL;

 blif = fopen (blif_file, "r");

 if(blif==NULL)
 {
    fprintf(stderr,"Error: Blif file doesn't exist! Quit!\n");
    exit(EXIT_FAILURE);
    return;
 }
 
 for (doall=0;doall<=1;doall++) {
    init_parse(doall);

    for (pass=1;pass<=3;pass++) { 
       linenum = 0;   // Reset line number.
       done = 0;
       while((my_fgets(buffer,BUFSIZE,blif) != NULL) && !done) {
          get_tok(buffer, pass, doall, &done);
       }
       rewind (blif); // Start at beginning of file again
    }
 } 
 fclose(blif);
 check_net(cell_size);
 free_parse();
 
 // Single input logic gate (INV, BUF, ZERO, ONE) are mapped on gates with larger fan-in --> Increase the fan-in artificially
 low_fanout_gate_formatting();
 
 // Initial structures are initialized
 // Now, creates the rollback structures (validated and current), by duplicating the initial structures
 
 num_blocks_validated = num_blocks;
 block_validated = (struct s_block *) malloc(num_blocks_validated*sizeof(struct s_block));
 memcpy(block_validated, block, num_blocks_validated * sizeof(struct s_block));
 
 for(i=0; i<num_blocks_validated; i++)
 {
    (block_validated+i)->name=(char*) malloc((strlen((block+i)->name)+1)*sizeof(char));
    memcpy((block_validated+i)->name, (block+i)->name,(strlen((block+i)->name)+1)*sizeof(char));  
 }
 
 num_blocks_current = num_blocks;
 block_current = (struct s_block *) malloc(num_blocks_current*sizeof(struct s_block));
 memcpy(block_current, block, num_blocks_current * sizeof(struct s_block));
 
 for(i=0; i<num_blocks_current; i++)
 {
    (block_current+i)->name=(char*) malloc((strlen((block+i)->name)+1)*sizeof(char));
    memcpy((block_current+i)->name, (block+i)->name,(strlen((block+i)->name)+1)*sizeof(char));  
 }
 
 num_nets_validated = num_nets;
 net_validated = (struct s_net *) malloc(num_nets_validated*sizeof(struct s_net));
 memcpy(net_validated, net, num_nets_validated * sizeof(struct s_net));
 
 for(i=0; i<num_nets_validated; i++)
 {
    (net_validated+i)->name=(char*) malloc((strlen((net+i)->name)+1)*sizeof(char));
    memcpy((net_validated+i)->name, (net+i)->name,(strlen((net+i)->name)+1)*sizeof(char));  
    
    (net_validated+i)->pins=(int*) malloc((net+i)->num_pins*sizeof(int));
    memcpy((net_validated+i)->pins, (net+i)->pins,(net+i)->num_pins*sizeof(int));  
 }
 
 num_nets_current = num_nets;
 net_current = (struct s_net *) malloc(num_nets_current*sizeof(struct s_net));
 memcpy(net_current, net, num_nets_current * sizeof(struct s_net));
 
 for(i=0; i<num_nets_current; i++)
 {
    (net_current+i)->name=(char*) malloc((strlen((net+i)->name)+1)*sizeof(char));
    memcpy((net_current+i)->name, (net+i)->name,(strlen((net+i)->name)+1)*sizeof(char));  
    
    (net_current+i)->pins=(int*) malloc((net+i)->num_pins*sizeof(int));
    memcpy((net_current+i)->pins, (net+i)->pins,(net+i)->num_pins*sizeof(int));  
 }
}

// Looks through all the block to find and mark all the clocks.
// Sets the corresponding entry in is_clock to true.
// Global_clocks is used only for an error check.    
boolean *alloc_and_load_is_clock (boolean global_clocks) {
 int num_clocks, bnum, clock_net;
 boolean *is_clock;

 num_clocks = 0;

 is_clock = (boolean *) calloc (num_nets, sizeof(boolean));

 // Want to identify all the clock nets.
 for (bnum=0;bnum<num_blocks;bnum++) {
    if (block[bnum].type == LATCH) {
       clock_net = block[bnum].nets[2];
       if (is_clock[clock_net] == FALSE) {
          is_clock[clock_net] = TRUE;
          num_clocks++;
       }
    }
 }

// If we have multiple clocks and we're supposed to declare them global,print a warning message, since it looks like this circuit may have locally generated clocks.                                             */
 if (num_clocks > 1 && global_clocks) {
    printf("Warning:  circuit contains %d clocks.\n", num_clocks);
    printf("          All will be marked global.\n");
 }

 return (is_clock);
}

// Allocates and initializes the data structures needed for the parse.
static void init_parse(int doall) {
 int i, j, len;
 struct hash_nets *h_ptr;

 if (!doall) {  // Initialization before first (counting) pass
    num_nets = 0;  
    
    hash = (struct hash_nets **) calloc(sizeof(struct hash_nets *), HASHSIZE);
 }

// Allocate memory for second (load) pass

 else {   
    net = (struct s_net *) malloc(num_nets*sizeof(struct s_net));
    block = (struct s_block *) malloc(num_blocks*sizeof(struct s_block));
    printf("num_nets=%d, num_blocks=%d\n",num_nets,    num_blocks);
    for(i=0;i<num_blocks;i++)
    {
      block[i].type=EMPTY;
      block[i].gate_type=NONE;
      for(j=0;j<GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB;j++)
        block[i].nets[j]=-1;
    }
      
    num_driver = (int *) malloc(num_nets * sizeof(int));
    temp_num_pins = (int *) malloc(num_nets*sizeof(int));

    for (i=0;i<num_nets;i++) {
       num_driver[i] = 0;
       net[i].num_pins = 0;
    }

    for (i=0;i<HASHSIZE;i++) {
       h_ptr = hash[i];   
       while (h_ptr != NULL) {
          net[h_ptr->index].pins = (int *) malloc(h_ptr->count*sizeof(int));

          // For avoiding assigning values beyond end of pins array.
          temp_num_pins[h_ptr->index] = h_ptr->count;
          len = strlen (h_ptr->name);
          net[h_ptr->index].name = (char *) malloc ((len + 1)* sizeof(char));
          strcpy (net[h_ptr->index].name, h_ptr->name);
          h_ptr = h_ptr->next;
       }
    }
 }

// Initializations for both passes.

 ilines = 0;
 olines = 0;
 model_lines = 0;
 endlines = 0;
 num_p_inputs = 0;
 num_p_outputs = 0;
 num_gates = 0;
 num_latches = 0;
 num_blocks = 0;
}

// Figures out which, if any, token is at the start of this line and takes the appropriate action.
static void get_tok (char *buffer, int pass, int doall, int *done) {
#define TOKENS " \t\n"
 char *ptr; 
 
 ptr = my_strtok(buffer,TOKENS,blif,buffer);
 if (ptr == NULL) return; 
 
 if (strcmp(ptr,".names") == 0) {
    printf("ERROR: LUTs are not supported\n");
    exit(EXIT_FAILURE);
    return;
 }

 if (strcmp(ptr,".gate") == 0) {
    if (pass == 3) {
       add_gate(doall);
    }
    else {
       dum_parse(buffer);
    }
    return;
 }

 if (strcmp(ptr,".latch") == 0) {
    if (pass == 3) {
       add_latch (doall);
    }
    else {
       dum_parse(buffer);
    }
    return;
 }

 if (strcmp(ptr,".model") == 0) {
    ptr = my_strtok(NULL,TOKENS,blif,buffer);

    if (doall && pass == 3) { // Only bother on main second pass.
       model_lines++; // For error checking only
    }
    return;
 }

 if (strcmp(ptr,".inputs") == 0) {
    if (pass == 1) {
       io_line(DRIVER, doall);
       *done = 1;
    }
    else {
       dum_parse(buffer);
       if (pass == 3 && doall) ilines++; // Error checking only
    }
    return;
 }

 if (strcmp(ptr,".outputs") == 0) {
    if (pass == 2) {
       io_line(RECEIVER, doall);
       *done = 1;
    }
    else {
       dum_parse(buffer);
       if (pass == 3 && doall) olines++;  // For error checking only: makes sure only one .output line
    }
    return;
 }

 if (strcmp(ptr,".end") == 0) {
    if (pass == 3 && doall) endlines++; // Error checking only
    return;
 }
}

// Continue parsing to the end of this (possibly continued) line.
static void dum_parse (char *buf) {
 while (my_strtok(NULL,TOKENS,blif,buf) != NULL);
}

// Adds the gate (.gate) currently being parsed to the block array.
// If doall is zero this is a counting pass; if it is 1 this is the final (loading) pass
// Adds its pins to the nets data structure by calling add_net.
// Blif format for a latch is: .gate <name_according_to_mpack.h> <inputs: A=XX,...> <output: Y=XX>
// The gate pins are in .nets starting from inputs [O, nb_inputs-1] to outputs [nb_inputs, nb_inputs+nb_outputs-1]
static void add_gate (int doall) 
{
 char *ptr, saved_names[GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB+1][BUFSIZE], buf[BUFSIZE];
 char name[BUFSIZE]="", nb_in_string[BUFSIZE]="", nb_out_string[BUFSIZE]="";
 int i, j, len;
 int nb_in, nb_out;

 num_blocks++;

// Read the cell name and extract informations on numbers of inputs and outputs
// Available gate functions given in mpack.h
// gate name format: FUNCTION_NUMBERofINPUTS_NUMBERofOUTPUTS
 ptr = my_strtok(NULL,TOKENS,blif,buf);

 nb_in=-1;
 nb_out=-1;
 j = 0;
 for(i=0;i<strlen(ptr);i++)
 {
    if((nb_in>-1) && (nb_out>-1))
    {
       if(*(ptr+i)!='_') { *(nb_out_string+j)=*(ptr+i); j++; }
       else { *(nb_out_string+j) = '\0'; }
    }

    if((nb_in>-1) && (nb_out==-1))
    {
       if(*(ptr+i)!='_') { *(nb_in_string+j)=*(ptr+i); j++; }
       else { *(nb_in_string+j) = '\0'; nb_out = 0; j=0; }
    }

    if((nb_in==-1) && (nb_out==-1)) // Type of the gate
    {
       if(*(ptr+i)!='_') { *(name+j)=*(ptr+i); j++; }
       else { *(name+j) = '\0'; nb_in = 0; j=0; }
    }

 }

 nb_in=atoi(nb_in_string);
 nb_out=atoi(nb_out_string);
 
// Count # nets connecting
 i=0;
 while ((ptr = my_strtok(NULL,TOKENS,blif,buf)) != NULL)  {
    if (i == GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB+1) {
       fprintf(stderr,"Error:  GATE #%d has too many connections : %d.  Check the netlist, line %d.\n",num_blocks-1,i,linenum);
       exit(EXIT_FAILURE);
    }

   j=0;
   while( (*(ptr+j) != '=') && (*(ptr+j)!= '\0') )  j++; // Deleting the first part of netlist mapping (ex: remove "A=" in "A=n12")

   if(*(ptr+j)=='=') ptr=ptr+j+1;
   strcpy(saved_names[i], ptr);

   i++;
 }

 if (i != (nb_in+nb_out)) {
    fprintf(stderr,"Error:  GATE #%d has %d connections, while expecting %d. check the netlist, line %d.\n",num_blocks-1,i, nb_in+nb_out,linenum);
    exit(EXIT_FAILURE);
 }

 if (!doall) { // Counting pass only ...
    for (j=0;j<i;j++) 
       add_net(saved_names[j],RECEIVER,num_blocks-1,doall);
    return;
 }

 block[num_blocks-1].num_nets = i;
 block[num_blocks-1].num_in = nb_in;
 block[num_blocks-1].num_out = nb_out;
 block[num_blocks-1].type = GATE;
 
 if(strcmp(name,"ZERO")==0) block[num_blocks-1].gate_type = ZERO;
 if(strcmp(name,"ONE")==0) block[num_blocks-1].gate_type = ONE;
 if(strcmp(name,"INV1")==0) block[num_blocks-1].gate_type = INV1A;
 if(strcmp(name,"BUF1")==0) block[num_blocks-1].gate_type = BUF1A;
 if(strcmp(name,"AND2")==0) block[num_blocks-1].gate_type = AND2;
 if(strcmp(name,"NAND2")==0) block[num_blocks-1].gate_type = NAND2;
 if(strcmp(name,"OR2")==0) block[num_blocks-1].gate_type = OR2;
 if(strcmp(name,"NOR2")==0) block[num_blocks-1].gate_type = NOR2; 
 if(strcmp(name,"NOTAND2")==0) block[num_blocks-1].gate_type = NOTAND2;
 if(strcmp(name,"ANDNOT2")==0) block[num_blocks-1].gate_type = ANDNOT2;
 if(strcmp(name,"NOTOR2")==0) block[num_blocks-1].gate_type = NOTOR2;
 if(strcmp(name,"ORNOT2")==0) block[num_blocks-1].gate_type = ORNOT2;
 if(strcmp(name,"XOR2")==0) block[num_blocks-1].gate_type = XOR2;
 if(strcmp(name,"XNOR2")==0) block[num_blocks-1].gate_type = XNOR2; 
 
 for (i=0;i<nb_in;i++) // Do inputs
    block[num_blocks-1].nets[i] = add_net (saved_names[i],RECEIVER,num_blocks-1,doall); 

 for (i=nb_in;i<(nb_in+nb_out);i++) // Do outputs
    block[num_blocks-1].nets[i] = add_net (saved_names[i],DRIVER,num_blocks-1,doall); 

 len = strlen (saved_names[block[num_blocks-1].num_nets-1]);
 block[num_blocks-1].name = (char *) malloc ((len+1) * sizeof (char));
 strcpy(block[num_blocks-1].name, saved_names[block[num_blocks-1].num_nets-1]);

#ifdef VERBOSE_DEBUG
 printf("BLOCK READ NAME %s - nbin %d - nbout %d - nbnet %d\n", block[num_blocks-1].name, block[num_blocks-1].num_in, block[num_blocks-1].num_out, block[num_blocks-1].num_nets);
#endif

 num_gates++;
}

// This routine creates a net to the global structure net
// Net name in *ptr, either DRIVER or RECEIVER specifying whether the block number given by bnum is driving this net or in the fan-out
// doall, which is 0 for the counting pass and 1 for the loading pass.
// It updates the net data structure and returns the net number so the calling routine can update the block data structure.      
static int add_net (char *ptr, int type, int bnum, int doall) 
{   
 struct hash_nets *h_ptr, *prev_ptr;
 int index, j, nindex;

 index = hash_value(ptr);
 h_ptr = hash[index]; 
 prev_ptr = h_ptr;

// Net already in hash table
 while (h_ptr != NULL) {
    if (strcmp(h_ptr->name,ptr) == 0) {
       nindex = h_ptr->index;

       if (!doall) {   // Counting pass only
          (h_ptr->count)++;
          return (nindex);
       }

       net[nindex].num_pins++;
       if (type == DRIVER) {
          num_driver[nindex]++;
          j=0; // Driver always in position 0 of pinlist
       }
       else {
          j = net[nindex].num_pins - num_driver[nindex]; // num_driver is the number of signal drivers of this net. Should always be zero or 1 unless the netlist is bad.
          if (j >= temp_num_pins[nindex]) {
             fprintf(stderr,"Error:  Net #%d (%s) has no driver and will cause\n", nindex, ptr);
             fprintf(stderr,"Memory corruption.\n");
             exit(EXIT_FAILURE);
          }
       }
       net[nindex].pins[j] = bnum;
       return (nindex);
    }
    prev_ptr = h_ptr;
    h_ptr = h_ptr->next;
 }

// Net was not in the hash table.

 if (doall == 1) {
    fprintf(stderr,"Error in add_net:  the second (load) pass could not\n");
    fprintf(stderr,"Find net %s in the symbol table.\n", ptr);
    exit(EXIT_FAILURE);
 }

// Add the net (only counting pass will add nets to symbol table).

 num_nets++;
 h_ptr = (struct hash_nets *) malloc (sizeof(struct hash_nets));
 if (prev_ptr == NULL) {
    hash[index] = h_ptr;
 }     
 else {  
    prev_ptr->next = h_ptr;
 }    
 h_ptr->next = NULL;
 h_ptr->index = num_nets - 1;
 h_ptr->count = 1;
 h_ptr->name = (char *) malloc((strlen(ptr)+1)*sizeof(char));
 strcpy(h_ptr->name,ptr);
 return (h_ptr->index);
}

// Adds the flipflop (.latch) currently being parsed to the block array.
// If doall is zero this is a counting pass; if it is 1 this is the final (loading) pass
// Adds its pins to the nets data structure by calling add_net.
// Blif format for a latch is: .latch <input> <output> <type (latch on)> <control (clock)> <init_val>
// The latch pins are in .nets 0 to 2 in the order: Q D CLOCK.
static void add_latch (int doall) 
{

 char *ptr, buf[BUFSIZE], saved_names[6][BUFSIZE];
 int i, len;

 num_blocks++;

 for (i=0;i<6;i++) {
    ptr = my_strtok (NULL,TOKENS,blif,buf);
    if (ptr == NULL) 
       break;
    strcpy (saved_names[i], ptr);
 }

 if (i != 5) {
    fprintf(stderr,"Error:  .latch does not have 5 parameters.Check the netlist, line %d.\n",linenum);
    exit(EXIT_FAILURE);
 }

 if (!doall) { // If only a counting pass ...
    add_net(saved_names[0],RECEIVER,num_blocks-1,doall);  // D
    add_net(saved_names[1],DRIVER,num_blocks-1,doall);    // Q
    add_net(saved_names[3],RECEIVER,num_blocks-1,doall);  // Clock
    return;
 }

 block[num_blocks-1].num_nets = 3; block[num_blocks-1].num_in = 1; block[num_blocks-1].num_out = 1;
 block[num_blocks-1].type = LATCH;

 block[num_blocks-1].nets[0] = add_net(saved_names[0],RECEIVER,num_blocks-1, doall); // D
 block[num_blocks-1].nets[1] = add_net(saved_names[1],DRIVER,num_blocks-1, doall); // Q                               
 block[num_blocks-1].nets[2] = add_net(saved_names[3],RECEIVER, num_blocks-1,doall); // Clock
 
#ifdef VERBOSE_DEBUG
 printf("\n\nREAD LATCH !!!! : Q=%d, D=%d, CLK=%d \n\n\n", block[num_blocks-1].nets[0], block[num_blocks-1].nets[1], block[num_blocks-1].nets[2]);
#endif
    
 len = strlen (saved_names[1]);
 block[num_blocks-1].name = (char *) malloc ((len+1) * sizeof (char));
 strcpy(block[num_blocks-1].name,saved_names[1]);
 num_latches++;
}

// Adds an input or output block to the block data structures.
// in_or_out:  DRIVER for input, RECEIVER for output.
// doall:  1 for final pass when structures are loaded. 0 for first pass when hash table is built and pins, nets, etc. are counted. 
static void io_line(int in_or_out, int doall) 
{  
 char *ptr;
 char buf2[BUFSIZE];
 int nindex, len;
  
 while (1) {
    ptr = my_strtok(NULL,TOKENS,blif,buf2);
    if (ptr == NULL) return;
    num_blocks++;
    
    nindex = add_net(ptr,in_or_out,num_blocks-1,doall); 

    if (!doall) continue;   // Just counting things when doall == 0

    len = strlen (ptr);
    block[num_blocks-1].name = (char *) malloc ((len+1) * sizeof (char));     
    strcpy(block[num_blocks-1].name,ptr);

    block[num_blocks-1].num_nets = 1; 
    block[num_blocks-1].nets[0] = nindex;  // Put in driver position for OUTPAD, since it has only one pin (even though it's a receiver)

    if (in_or_out == DRIVER) { // processing .inputs line
       num_p_inputs++;
       block[num_blocks-1].type = INPAD;
       block[num_blocks-1].num_in = 0;
       block[num_blocks-1].num_out = 1;
    }
    else { // processing .outputs line
       num_p_outputs++;
       block[num_blocks-1].type = OUTPAD;
       block[num_blocks-1].num_in = 1;
       block[num_blocks-1].num_out = 0;
    }
 } 
}

static int hash_value (char *name) 
{
 int i,k;
 int val=0, mult=1;
 
 i = strlen(name);
 k = max (i-7,0);
 for (i=strlen(name)-1;i>=k;i--) {
    val += mult*((int) name[i]);
    mult *= 10;
 }
 val += (int) name[0];
 val %= HASHSIZE;
 return(val);
}

// Echo back the netlist data structures to file XX.echo
void echo_input (char *blif_file, char *echo_file) 
{
 int i, j; 
 FILE *fp;

 printf("Input netlist file: %s\n", blif_file);
 printf("Primary Inputs: %d.  Primary Outputs: %d.\n", num_p_inputs,
                num_p_outputs);
 printf("Gates: %d.  Latches: %d.\n", num_gates, num_latches);
 printf("Total Blocks: %d.  Total Nets: %d\n", num_blocks, num_nets);
 
 fp = fopen (echo_file, "w"); 

 fprintf(fp,"Input netlist file: %s\n",blif_file);
 fprintf(fp,"num_p_inputs: %d, num_p_outputs: %d, num_gates: %d, num_latches: %d\n",num_p_inputs,num_p_outputs,num_gates, num_latches);
 fprintf(fp,"num_blocks: %d, num_nets: %d\n",num_blocks,num_nets);

 fprintf(fp,"\nNet\tName\t\t#Pins\tDriver\tRecvs.\n");
 for (i=0;i<num_nets;i++) {
    fprintf(fp,"\n%d\t%s\t", i, net[i].name);
    if (strlen(net[i].name) < 8)
       fprintf(fp,"\t"); // Name field is 16 chars wide
    fprintf(fp,"%d", net[i].num_pins);
    for (j=0;j<net[i].num_pins;j++) 
        fprintf(fp,"\t%d",net[i].pins[j]);
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

 for (i=0;i<num_blocks;i++) { 
    fprintf(fp,"\n%d\t%s\t",i, block[i].name);
    if (strlen(block[i].name) < 8)
       fprintf(fp,"\t"); // Name field is 16 chars wide
    fprintf(fp,"%d\t%d\t%d", block[i].type, block[i].gate_type, block[i].num_nets);

    if (block[i].type == INPAD || block[i].type == OUTPAD || block[i].type == LATCH || block[i].type==GATE)
    {
       for(j=block[i].num_in;j<block[i].num_out+block[i].num_in;j++)
          fprintf(fp,"\to:%d",block[i].nets[j]);
       for(j=0;j<block[i].num_in;j++)
          fprintf(fp,"\ti:%d",block[i].nets[j]);

       if(block[i].type == LATCH) {
         for(j=block[i].num_out+block[i].num_in;j<block[i].num_out+block[i].num_in+1;j++)
         fprintf(fp,"\tc:%d", block[i].nets[j]);
       }
   }
   else
   {
       if(block[i].type == EMPTY)
         printf("Warning: Block %d is unused (Type EMPTY)\n", i);
       else
       {
         fprintf(stderr,"echo_input (read_blic.c): Wrong block type\n");
         exit(EXIT_FAILURE);
       }
   }
 }

 fprintf(fp,"\n");
 fclose(fp);
}

// Checks the input netlist for obvious errors.
static void check_net (int cell_size) 
{
 int i, error, iblk;
 
 error = 0;

 if (ilines != 1) {
     printf("Warning:  found %d .inputs lines; expected 1.\n",
        ilines);
     error++;
 }

 if (olines != 1) {
     printf("Warning:  found %d .outputs lines; expected 1.\n",
        olines);
     error++;
 }

 if (model_lines != 1) {
     printf("Warning:  found %d .model lines; expected 1.\n",
        model_lines);
     error++;
 }

 if (endlines != 1) {
     printf("Warning:  found %d .end lines; expected 1.\n",
        endlines);
     error++;
 }

 for (i=0;i<num_nets;i++) {

    if (num_driver[i] != 1) {
       printf ("Warning:  net %s has"
         " %d signals driving it.\n",net[i].name,num_driver[i]);
       error++;
    }

    if ((net[i].num_pins - num_driver[i]) < 1) {

// If this is an input pad, it is unused and I just remove it with
// a warning message.  Lots of the mcnc circuits have this problem.

       iblk = net[i].pins[0];
       if (block[iblk].type == INPAD) {
          printf("Warning:  Input %s is unused; removing it.\n", block[iblk].name);
          net[i].pins[0] = OPEN;
          block[iblk].type = EMPTY;
       }
          
       else {
          printf("Warning:  net %s has no fanout.\n",net[i].name);
          error++;
       }
    }

    if (strcmp(net[i].name, "open") == 0) {
       printf("Warning:  net #%d has the reserved name %s.\n",i,net[i].name);
       error++;
    }
 }

 for (i=0;i<num_blocks;i++) {
    if (block[i].type == GATE) {
       if (block[i].num_nets < 2) {
          printf("Warning:  logic block #%d with output %s has only %d pin.\n", i, block[i].name, block[i].num_nets);
          if (block[i].num_nets != 1) { // Block with 1 pin (an output) can be a constant generator. Warn the user, and remove it (it shouldn't take any critical part of the packing)
             error++;
          }
          else {
             printf("\tPin is an output -- may be a constant generator.\n");
             printf("\tNon-fatal error.\n");
          }
       }

       if (block[i].num_nets > cell_size + 1) {
          printf("Warning:  logic block #%d (%s) has %d pins.\n", i, block[i].name, block[i].num_nets);
          error++;
       }
    }

    else if (block[i].type == LATCH) {
       if (block[i].num_nets != 3) {
          printf("Warning:  Latch #%d (%s) has %d pin(s).\n", i, block[i].name, block[i].num_nets);
          error++;
       }
    }
       
    else {
       if (block[i].num_nets != 1) {
          printf("Warning:  io block #%d (%s) of type %d has %d pins.\n", i, block[i].name, block[i].type, block[i].num_nets);
          error++;
       }
    }
 }
 
 if (error != 0) {
    fprintf(stderr,"Found %d fatal errors in the input netlist.\n",error);
    exit(EXIT_FAILURE);
 }
}

// Release memory needed only during blif network parsing.
static void free_parse (void)
{  
 int i;
 struct hash_nets *h_ptr, *temp_ptr;

 for (i=0;i<HASHSIZE;i++) {
    h_ptr = hash[i];
    while (h_ptr != NULL) {
       free ((void *) h_ptr->name);
       temp_ptr = h_ptr->next;
       free ((void *) h_ptr);
       h_ptr = temp_ptr;
    }
 }
 free ((void *) num_driver);
 free ((void *) hash);
 free ((void *) temp_num_pins);
}

static void low_fanout_gate_formatting() {
  int i, output_net;
  
  for(i=0; i<num_blocks; i++) // For every nodes in the function
  {
      // Transform the orginal netlist INV/BUF in CELLs --> Fan-out 1
      if(block[i].type==GATE && block[i].num_in==1)
      {
#ifdef VERBOSE_DEBUG
        printf("LOW FANOUT FORMATTING: INV/BUF detected = %d\n", i);      
#endif
        // Add an input net
        output_net=block[i].nets[block[i].num_in]; // Store the initial output
        block[i].num_in++; // Add the input
        block[i].nets[block[i].num_in-1]=-1; // Reset the input
        block[i].nets[block[i].num_in]=output_net; // Reposition the output          
        block[i].num_nets=block[i].num_in+block[i].num_out;
      }
      // Transform the orginal netlist ZERO/ONE in CELLs --> Fan-out 0
      if(block[i].type==GATE && block[i].num_in==0)
      {
#ifdef VERBOSE_DEBUG
        printf("LOW FANOUT FORMATTING: ZERO/ONE detected = %d\n", i);      
#endif  
          output_net=block[i].nets[block[i].num_in]; // Store the initial output
          block[i].num_in++; // Add the first input
          block[i].nets[block[i].num_in-1]=-1; // Reset it
          block[i].num_in++; // Add the second input
          block[i].nets[block[i].num_in-1]=-1; // Reset      
          block[i].nets[block[i].num_in]=output_net; // Reposition the output          
          block[i].num_nets=block[i].num_in+block[i].num_out;
      } 
   }
}

// Get an input line, update the line number and cut off any comment part.
// A \ at the end of a line with no comment part (#) means continue. 
char *my_fgets(char *buf, int max_size, FILE *fp) {

  char *val;
 int i;
 
 cont = 0;
 val = fgets(buf,max_size,fp);
 linenum++;
 if (val == NULL) return(val);

// Check that line completely fit into buffer. (Flags long line truncation).

 for (i=0;i<max_size;i++) {
    if (buf[i] == '\n') 
       break;
    if (buf[i] == '\0') {
       fprintf(stderr, "Error on line %d -- line is too long for input buffer.\n", linenum);
       fprintf(stderr, "All lines must be at most %d characters long.\n",BUFSIZE-2);
       fprintf(stderr, "The problem could also be caused by a missing newline.\n");
       exit(EXIT_FAILURE);
    }
 }

 for (i=0;i<max_size && buf[i] != '\0';i++) {
    if (buf[i] == '#') {
        buf[i] = '\0';
        break;
    }
 }

 if (i<2) return (val);
 if (buf[i-1] == '\n' && buf[i-2] == '\\') { 
    cont = 1;   // line continued */
    buf[i-2] = '\n';  // May need this for tokens
    buf[i-1] = '\0';
 }
 return(val);
}

// Get next token, and wrap to next line if \ at end of line. There is a bit of a "gotcha" in strtok.
// It does not make a copy of the character array which you pass by pointer on the first call.
// Thus, you must make sure this array exists for as long as you are using strtok to parse that line.
char *my_strtok(char *ptr, char *tokens, FILE *fp, char *buf) {

  char *val;

 val = strtok(ptr,tokens);
 while (1) {
    if (val != NULL || cont == 0) return(val); // return unless we have a null value and a continuation line
    if (my_fgets(buf,BUFSIZE,fp) == NULL) 
       return(NULL);
    val = strtok(buf,tokens);
 }
}
