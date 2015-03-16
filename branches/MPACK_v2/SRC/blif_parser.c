#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
//#include <malloc.h>
#include "blif_parser.h"
#include "util.h"

/*Local-Only Subroutines*/
int init_blif_arch(t_blif_arch* blif_arch);

t_conkt* create_conkt_list(int len,
                         size_t size_node);

t_conkt* insert_conkt_list_node(t_conkt* cur,
                             size_t size_node);

void remove_conkt_list_node(t_conkt* cur);

t_conkt* cat_conkt_lists(t_conkt* head1,
                         t_conkt* head2);

t_conkt* ready_new_conkt(t_conkt* cur,
                         char* name,
                         int net_cnt);

void init_conkt(t_conkt* conkt);

t_conkt* spot_conkt_name(t_conkt* head,
                         char* name);

void mark_pi_po_conkt(t_blif_arch* blif_arch,
                      t_conkt* nets);

/*Subroutines*/
/* Initial the blif architecture*/
int init_blif_arch(t_blif_arch* blif_arch)
{
  blif_arch->model = NULL;
  blif_arch->input_num = 0;
  blif_arch->inputs = NULL;
  blif_arch->output_num = 0;
  blif_arch->outputs = NULL;
  blif_arch->clk_num = 0;
  blif_arch->clk_names = NULL;
  blif_arch->gate_num = 0;
  blif_arch->gates = NULL;
  blif_arch->libgate_num = 0;
  blif_arch->libgates = NULL;
  blif_arch->latch_num = 0;
  blif_arch->latches = NULL;
  blif_arch->nets = NULL;
  return 1;
}

/* Fill model name in blif arch*/
int fill_blif_arch_model(t_blif_arch* blif_arch,
                         char** tokens, 
                         int tokens_len)
{
  if (tokens_len > 2) 
  {printf("Warning: There should be no space inside model name!\n");}
  blif_arch->model = my_strdup(tokens[1]);
  printf("Blif model: %s\n",blif_arch->model);
  return 1;
}

/* Fill inputs in blif arch*/
int fill_blif_arch_inputs(t_blif_arch* blif_arch,
                          char** tokens, 
                          int tokens_len)
{
  int iin;

  /*Determine the number of inputs*/
  blif_arch->input_num = tokens_len - 1;
  printf("Blif info: %d inputs detected.\n", blif_arch->input_num);

  /* Allocate inputs structs*/
  blif_arch->inputs = (t_blif_ios*)my_malloc(blif_arch->input_num*sizeof(t_blif_ios));
  for (iin=0; iin<blif_arch->input_num; iin++)
  {
    /*assign same to port and name*/
    blif_arch->inputs[iin].idx = iin;
    blif_arch->inputs[iin].port = my_strdup(tokens[iin+1]);
    blif_arch->inputs[iin].name = blif_arch->inputs[iin].port;
  }

  return 1;
}

/* Fill outputs in blif arch*/
int fill_blif_arch_outputs(t_blif_arch* blif_arch,
                           char** tokens, 
                           int tokens_len)
{
  int iout;

  /*Determine the number of inputs*/
  blif_arch->output_num = tokens_len - 1;
  printf("Blif info: %d outputs detected.\n", blif_arch->output_num);

  /* Allocate outputs structs*/
  blif_arch->outputs = (t_blif_ios*)my_malloc(blif_arch->output_num*sizeof(t_blif_ios));
  for (iout=0; iout<blif_arch->output_num; iout++)
  {
    /*assign same to port and name*/
    blif_arch->outputs[iout].idx = iout;
    blif_arch->outputs[iout].port = my_strdup(tokens[iout+1]);
    blif_arch->outputs[iout].name = blif_arch->outputs[iout].port;
  }

  return 1;
}

/* Fill clocks in blif arch*/
int fill_blif_arch_clk(t_blif_arch* blif_arch,
                       char** tokens,
                       int tokens_len,
                       int linenum)
{
  int clk_total;
  int iclk;

  /* Check if there exists clocks*/  
  if (blif_arch->clk_num > 0)
  {
    printf("Multiple clocks are found at line(%d)!\n", linenum);
    /* Incremental clock number*/
    clk_total = blif_arch->clk_num + tokens_len - 1; 
    /* Realloc memory for clocks*/
    blif_arch->clk_names = (t_blif_ios*)my_realloc(blif_arch->clk_names,clk_total*sizeof(t_blif_ios));
    
    /*Assign clock names*/ 
    for (iclk=blif_arch->clk_num; iclk<clk_total; iclk++)
    {
      blif_arch->clk_names[iclk].idx = iclk;
      blif_arch->clk_names[iclk].port = my_strdup(tokens[iclk - blif_arch->clk_num + 1]);
      blif_arch->clk_names[iclk].name = blif_arch->clk_names[iclk].port;
    }
    /*Assign clock number*/
    blif_arch->clk_num = clk_total;
  }
  else
  {
    /*Assign clock number*/
    blif_arch->clk_num = tokens_len - 1;

    /* Malloc memory for clocks*/
    blif_arch->clk_names = (t_blif_ios*)my_malloc(clk_total*sizeof(t_blif_ios));
    
    /*Assign clock names*/ 
    for (iclk=0; iclk<blif_arch->clk_num; iclk++)
    {
      blif_arch->clk_names[iclk].idx = iclk;
      blif_arch->clk_names[iclk].port = my_strdup(tokens[iclk+1]);
      blif_arch->clk_names[iclk].name = blif_arch->clk_names[iclk].port;
    } 
    
  }

  
  return 1;
}

/* Fill libgates in blif arch*/
int fill_blif_arch_libgates(t_blif_arch* blif_arch,
                            char** tokens, 
                            int tokens_len,
                            int* cnt,
                            int linenum)
{
  int itok;  
  char** toks;
  char* delims = "=";
  int tok_len;

  /* File the name of libgate*/
  blif_arch->libgates[*cnt].idx = (*cnt);
  blif_arch->libgates[*cnt].name = my_strdup(tokens[1]);
  /* Identify the input number*/
  /* The first token is ".gate", the second is "<gate_name>", the last is output*/
  blif_arch->libgates[*cnt].input_num = tokens_len - 3;
  //if (blif_arch->libgates[*cnt].input_num < 1) { 
  //  printf("Error: illegal definition of port map at line(%d)!\n", linenum);
  //  exit(1);
  //}
  /* Allocate memory*/
  blif_arch->libgates[*cnt].inputs = (t_blif_ios*)my_malloc(blif_arch->libgates[*cnt].input_num*sizeof(t_blif_ios));
  /* Fill inputs*/
  for (itok=2; itok<tokens_len-1; itok++)
  {
    toks = my_strtok(tokens[itok],delims,&tok_len); 
    if (tok_len > 2) {
      printf("Warning: more than 2 equations found in line(%d)!Ignore reduntants...\n", linenum);
    }
    blif_arch->libgates[*cnt].inputs[itok-2].idx = itok-2;
    blif_arch->libgates[*cnt].inputs[itok-2].port = my_strdup(toks[0]);
    blif_arch->libgates[*cnt].inputs[itok-2].name = my_strdup(toks[1]);
    free_tokens(toks,tok_len);
  }
  /* Fill in outputs*/ 
  toks = my_strtok(tokens[tokens_len-1],delims,&tok_len); 
  if (tok_len > 2)
  {printf("Warning: more than 2 equations found in line(%d)!Ignore reduntants...\n", linenum);}
  blif_arch->libgates[*cnt].output.idx = 1;
  blif_arch->libgates[*cnt].output.port = my_strdup(toks[0]);
  blif_arch->libgates[*cnt].output.name = my_strdup(toks[1]);
  free_tokens(toks,tok_len); 

  (*cnt)++;

  return 1;
}

/* Fill latches in blif arch*/
int fill_blif_arch_latches(t_blif_arch* blif_arch,
                            char** tokens, 
                            int tokens_len,
                            int* cnt,
                            int linenum)
{

  /* check the number of tokens*/ 
  if (tokens_len > 6)
  {
    printf("Warning: latches have at most 5 parameters. The rest will be ignored!\n");
  }
  if (tokens_len < 4)
  {
    printf("Error: latches should have at least 3 parameters!\n");
    exit(1);
  }
  if (5 == tokens_len)
  {
    printf("Warning: latches have 3 or 5 parameters. The rest will be ignored!\n");
  }

  /* Assign latch attributes*/
  blif_arch->latches[*cnt].idx = (*cnt);

  blif_arch->latches[*cnt].input.idx = 0;
  blif_arch->latches[*cnt].input.port = my_strdup(tokens[1]);
  blif_arch->latches[*cnt].input.name = blif_arch->latches[*cnt].input.port;

  blif_arch->latches[*cnt].output.idx = 0;
  blif_arch->latches[*cnt].output.port = my_strdup(tokens[2]);
  blif_arch->latches[*cnt].output.name = blif_arch->latches[*cnt].output.port;

  /*
   * Compact definition of latch
   * Autofill the type as re(rising_edge)
   *          the control as CLK
   */
  if ((3 < tokens_len)&&( tokens_len < 6))
  {
    blif_arch->latches[*cnt].type = "re";
    /* if there is no clock defined. Assign with NIL*/
    if (blif_arch->clk_num < 2)
    {
      blif_arch->latches[*cnt].clk = "NIL";
    }
    else
    {
      printf("Error: more than 2 clock is defined in blif. Compact definition of latch is not allowed! Clock for latches must be assigned!\n");
      exit(1);
    }
    
    blif_arch->latches[*cnt].init_val = atoi(tokens[3]);
  }

  if (5 < tokens_len)
  {
    blif_arch->latches[*cnt].type = my_strdup(tokens[3]);
    blif_arch->latches[*cnt].clk = my_strdup(tokens[4]);
    
    blif_arch->latches[*cnt].init_val = atoi(tokens[5]);
  }
  
  (*cnt)++;
 
  return 1;
}

/* Fill gates in blif arch*/
int fill_blif_arch_gates(t_blif_arch* blif_arch,
                         char** tokens, 
                         int tokens_len,
                         int* cnt,
                         int linenum)
                         //FILE* blif,
                         //char* blifname)
{
  int itok; 
  //int icvr;
  //char buffer[BUFSIZE];
  //char* line;

  /*
   * Two steps:
   * 1st is fill the information of input and output
   * 2nd is fill the information of single-output covers
   */ 
  /* File the name of gate*/
  blif_arch->gates[*cnt].idx = (*cnt);
  /* Identify the input number*/
  /* The first token is ".name", the last is output*/
  blif_arch->gates[*cnt].input_num = tokens_len - 2;
  if (blif_arch->gates[*cnt].input_num == 0) { 
    printf("Warning: constant generator found at line(%d)!\n", linenum);
    blif_arch->gates[*cnt].inputs = NULL; 
  } else if (blif_arch->gates[*cnt].input_num > 0) {
    /* Allocate memory*/
    blif_arch->gates[*cnt].inputs = (t_blif_ios*)my_malloc(blif_arch->gates[*cnt].input_num*sizeof(t_blif_ios));
  } else {
    printf("Error: invalid definition for LUT at line(%d).\n",linenum);
    exit(1);
  }
  /* Fill inputs*/
  for (itok=1; itok<tokens_len-1; itok++)
  {
    blif_arch->gates[*cnt].inputs[itok-1].idx = itok-1;
    blif_arch->gates[*cnt].inputs[itok-1].port = my_strdup(tokens[itok]);
    blif_arch->gates[*cnt].inputs[itok-1].name = blif_arch->gates[*cnt].inputs[itok-1].port;
  }
  /* Fill in outputs*/ 
  blif_arch->gates[*cnt].output.idx = 1;
  blif_arch->gates[*cnt].output.port = my_strdup(tokens[tokens_len-1]);
  blif_arch->gates[*cnt].output.name = blif_arch->gates[*cnt].output.port;
  // Initial is_buf
  blif_arch->gates[*cnt].is_buf = 0;
  /*Fill the covers*/ 
  /*Allocate memory*/
  //blif_arch->gates[*cnt].covers = (t_blif_ios*)my_malloc(blif_arch->gates[*cnt].input_num);
  /*Read the next lines*/
  /*
  for(icvr=0; icvr<blif_arch->gates[*cnt].input_num; )
  {
    if (-1 == full_freadlns(buffer,BUFSIZE,blif,blifname,linenum,line))
    {
      printf("Error: Incomplete single-output-cover definition in line(%d)!\n", *linenum);
      exit(1);
    }
    blif_arch->gates[*cnt].covers[icvr].port = 
    icvr++;
  }
  */
  (*cnt)++;  

  return blif_arch->gates[(*cnt)-1].input_num;
}

/*
 * Blif parser
 */
int blif_parser(IN char* blifname,
                INOUT t_blif_arch* blif_arch)
{
  char buffer[BUFSIZE];
  int feof;   
  FILE* blif;
  char* line = NULL;
  int linenum = 0;
  char* delims = " ";
  int tokens_len;
  char** tokens;

  int gate_cnt = 0;
  int libgate_cnt = 0;
  int latch_cnt = 0;

  init_blif_arch(blif_arch);
 
  printf("Reading Blif (%s)...\n",blifname);
  /* Open blif file*/
  blif = my_fopen(blifname,"r");

  /*First scan of file. Count the number of gates, libgates, latches*/
  while(1)
  {
    line = full_freadlns(buffer,BUFSIZE,blif,blifname,&linenum,&feof);
    //printf("DEBUG:[LINE%d]%s\n",linenum,buffer);
    /*Read in a line*/
    if (-1 == feof) 
    {break;}
    
    /*DEBUG*/
    //printf("ReadLine(No. %d): %s\n",linenum,line);
    
    /* Split the line with space*/ 
    tokens = my_strtok(line,delims,&tokens_len); 

    /* Process EOF*/
    if (0 == strcmp(tokens[0],".end"))
    {
      //printf("End of file(%s) is reached at line(%d). First scan over...\n",blifname,linenum);
      break;
    } 
   
    /*Find match with gate,latch,name and clock*/ 
    if (0 == strcmp(tokens[0],".gate"))
    {blif_arch->libgate_num++;}
    else if (0 == strcmp(tokens[0],".latch"))
    {blif_arch->latch_num++;}
    else if (0 == strcmp(tokens[0],".names"))
    {blif_arch->gate_num++;} 
    else if (0 == strcmp(tokens[0],".clock"))
    {
      /* Assign clock first. Multiple clock is allowed*/
      fill_blif_arch_clk(blif_arch,tokens,tokens_len,linenum);
    } 

    //printf("Clean memory...");
    memset(buffer,0,sizeof(char)*BUFSIZE);
    //memset(line,0,sizeof(char)*(strlen(line)+1));
    free(line);
    free_tokens(tokens,tokens_len);
    //printf("Over\n");
  }

  printf("First scan of blif(%s) finished...\n",blifname);
  printf("Blif info: %d libgates, %d latches, %d gates(names) found!\n", blif_arch->libgate_num, blif_arch->latch_num ,blif_arch->gate_num);
  rewind(blif);

  /*Allocate memory for gates,libgates, and latches*/
  if (blif_arch->libgate_num)
  {blif_arch->libgates = (t_blif_libgate*)my_malloc(blif_arch->libgate_num*sizeof(t_blif_libgate));}
  if (blif_arch->gate_num)
  {blif_arch->gates = (t_blif_gate*)my_malloc(blif_arch->gate_num*sizeof(t_blif_gate));}
  if (blif_arch->latch_num)
  {blif_arch->latches = (t_blif_latch*)my_malloc(blif_arch->latch_num*sizeof(t_blif_latch));}
  

  /*Second Scan of file. Fill the blif struct*/
  /*Make sure memory is clean*/
  linenum = 0;
  free(line);
  memset(buffer,0,sizeof(char)*BUFSIZE);
  while(1)
  {
    line = full_freadlns(buffer,BUFSIZE,blif,blifname,&linenum,&feof);
    /*Read in a line*/
    if (-1 == feof)
    {break;}

    /*DEBUG*/
    //printf("ReadLine(No. %d): %s\n",linenum,line);
    
    /* Split the line with space*/ 
    tokens = my_strtok(line,delims,&tokens_len); 

    if (0 == strcmp(tokens[0],".end"))
    {
      //printf("End of file(%s) is reached at line(%d).\n",blifname,linenum);
      break;
    } 
  
    /* Fill the blif arch*/
    /* Process the model name*/
    if (0 == strcmp(tokens[0],".model"))
    {
      fill_blif_arch_model(blif_arch,tokens,tokens_len); 
    }
    else if (0 == strcmp(tokens[0],".inputs"))
    {
      fill_blif_arch_inputs(blif_arch,tokens,tokens_len);
    }
    else if (0 == strcmp(tokens[0],".outputs"))
    {
      fill_blif_arch_outputs(blif_arch,tokens,tokens_len);
    }
    else if (0 == strcmp(tokens[0],".gate"))
    {
      fill_blif_arch_libgates(blif_arch,tokens,tokens_len,&libgate_cnt,linenum);
    }
    else if (0 == strcmp(tokens[0],".latch"))
    {
      fill_blif_arch_latches(blif_arch,tokens,tokens_len,&latch_cnt,linenum);
    }
    else if (0 == strcmp(tokens[0],".names")) {
      if (1 == fill_blif_arch_gates(blif_arch,tokens,tokens_len,&gate_cnt,linenum)) {
        // Identify if this is a buffer
        //free previous line
        memset(buffer,0,sizeof(char)*BUFSIZE);
        free(line);
        free_tokens(tokens,tokens_len);
        // Read the next line
        line = full_freadlns(buffer,BUFSIZE,blif,blifname,&linenum,&feof);
        tokens = my_strtok(line,delims,&tokens_len); 
        assert(2 == tokens_len); // Expect 1 1 or 0 1
        if ((0 == strcmp(tokens[0],"1"))&&(0 == strcmp(tokens[1],"1"))) {
          blif_arch->gates[gate_cnt-1].is_buf = 1;
        }
      }
    } else if (0 == strcmp(tokens[0],".default_input_arrival"))
    { /* TODO*/}
    else if (0 == strcmp(tokens[0],".clock"))
    { /* TODO*/}
    else if ('.' == tokens[0][0])
    {
      printf("Warning: Undefined keyword(%s) in Blif(%s), Line(%d)!Ignore it...\n", tokens[0],blifname,linenum);
    }
    memset(buffer,0,sizeof(char)*BUFSIZE);
    //memset(line,0,sizeof(char)*strlen(line));
    free(line);
    free_tokens(tokens,tokens_len);
  }
  
  printf("Blif file is read successfully!\n");
  memset(buffer,0,sizeof(char)*BUFSIZE);
  free(line);

  return 1;
}


/**
 * Construct the nets which connects gates,PI,PO
 * and latches.
 * Use the struct t_conkt
 */
t_conkt* build_nets(t_blif_arch* blif_arch)
{
  /*Define head*/
  t_conkt* head;
  t_conkt* cur;
  int igl;
  int jin;
  int inet;
  int net_num;
  int max_fanout;
  t_conkt* net_max_fout;

  inet = 0;
  net_num = 0;
  max_fanout = 0;

  head = create_conkt_list(0,sizeof(t_conkt));
  init_conkt(head);
  cur = head;
  /**
   * scan libgates. 
   * Scan twice, the first is to define the input number and output number;
   * the second is to allocate inputs and outputs
   */
  /*First scan*/
  for (igl=0; igl<blif_arch->libgate_num; igl++)
  {
    /*Assign outputs of conkt*/
    for(jin=0; jin<blif_arch->libgates[igl].input_num; jin++)
    {
      /*Try to find a matched net name*/
      if (DEBUG)
      {printf("DEBUG: Matching name(%s)...\n",blif_arch->libgates[igl].inputs[jin].name);}
      cur = spot_conkt_name(head,blif_arch->libgates[igl].inputs[jin].name);
      if (NULL == cur)
      {
        /*update counter*/
        inet++;
        cur = ready_new_conkt(head,blif_arch->libgates[igl].inputs[jin].name,inet);
      }
      /*Establish mutual pointor*/
      blif_arch->libgates[igl].inputs[jin].conkt = cur;
      /*Increase the output number*/
      cur->output_num++; 
      if (DEBUG)
      {printf("DEBUG: Net(%s) add input of libgates(%s).\n",cur->name,blif_arch->libgates[igl].inputs[jin].name);}
    }

    /*Assign input of conkt*/
    if (DEBUG)
    {printf("DEBUG: Matching name(%s) ...\n",blif_arch->libgates[igl].output.name);}
    cur = spot_conkt_name(head,blif_arch->libgates[igl].output.name);
    if (NULL == cur)
    {
      /*update counter*/
      inet++;
      cur = ready_new_conkt(head,blif_arch->libgates[igl].output.name,inet);
    }
    /*Establish mutual pointor*/
    blif_arch->libgates[igl].output.conkt = cur;
    /*Increase the input number*/
    cur->input_num++;
    cur->inputs_types = BLIF_LIBGATE;
    cur->inputs = igl;
    // DEBUG
    if (DEBUG)
    {printf("DEBUG: Net(%s) add output of libgates(%s).\n",cur->name,blif_arch->libgates[igl].output.name);}
    /*Check if invalid */
    if (cur->input_num > 1)
    {
      printf("Error: Multiple drivers(%d) to net(%s)!\n", cur->input_num, cur->name);
      exit(1);
    }
  }

  
 /**
   * scan gates. 
   * Scan twice, the first is to define the input number and output number;
   * the second is to allocate inputs and outputs
   */
  /*First scan*/
  for (igl=0; igl<blif_arch->gate_num; igl++)
  {
    /*Assign outputs of conkt*/
    for(jin=0; jin<blif_arch->gates[igl].input_num; jin++)
    {
      /*Try to find a matched net name*/
      cur = spot_conkt_name(head,blif_arch->gates[igl].inputs[jin].name);
      if (NULL == cur)
      {
        /*update counter*/
        inet++;
        cur = ready_new_conkt(head,blif_arch->gates[igl].inputs[jin].name,inet);
      }   
      /*Establish mutual pointor*/
      blif_arch->gates[igl].inputs[jin].conkt = cur;
      /*Increase the output number*/
      cur->output_num++; 
      if (DEBUG)
      {printf("DEBUG: Net(%s) add input of gates(%s).\n",cur->name,blif_arch->gates[igl].inputs[jin].name);}
    }

    /*Assign input of conkt*/
    cur = spot_conkt_name(head,blif_arch->gates[igl].output.name);
    if (NULL == cur)
    {
      /*update counter*/
      inet++;
      cur = ready_new_conkt(head,blif_arch->gates[igl].output.name,inet);
    }
    /*Establish mutual pointor*/
    blif_arch->gates[igl].output.conkt = cur;
    /*Increase the input number*/
    cur->input_num++;
    cur->inputs_types = BLIF_GATE;
    cur->inputs = igl;
    // DEBUG
    if (DEBUG)
    {printf("DEBUG: Net(%s) add output of gates(%s).\n",cur->name,blif_arch->gates[igl].output.name);}
    // DEBUG
    //printf("cur->inputs.input = %s.\n",cur->inputs->input.name);
    /*Check if invalid */
    if (cur->input_num > 1)
    {
      printf("Error: Multiple drivers(%d) to net(%s)!\n", cur->input_num, cur->name);
      exit(1);
    }
  }
 

/**
   * scan latches. 
   * Scan twice, the first is to define the input number and output number;
   * the second is to allocate inputs and outputs
   */
  /*First scan*/
  for (igl=0; igl<blif_arch->latch_num; igl++)
  {
    /*Assign outputs of conkt*/
    /*Try to find a matched net name*/
    cur = spot_conkt_name(head,blif_arch->latches[igl].input.name);
    if (NULL == cur)
    {
      /*update counter*/
      inet++;
      cur = ready_new_conkt(head,blif_arch->latches[igl].input.name,inet);
    }
    /*Establish mutual pointor*/
    blif_arch->latches[igl].input.conkt = cur;
    /*Increase the output number*/
    cur->output_num++; 
    if (DEBUG)
    {printf("DEBUG: Net(%s) add input of latch(%s).\n",cur->name,blif_arch->latches[igl].input.name);}

    /*Assign input of conkt*/
    cur = spot_conkt_name(head,blif_arch->latches[igl].output.name);
    if (NULL == cur)
    {
      /*update counter*/
      inet++;
      cur = ready_new_conkt(head,blif_arch->latches[igl].output.name,inet);
    }
    /*Establish mutual pointor*/
    blif_arch->latches[igl].output.conkt = cur;
    /*Increase the input number*/
    cur->input_num++;
    cur->inputs_types = BLIF_LATCH;
    cur->inputs = igl;
    // DEBUG
    if (DEBUG)
    {printf("DEBUG: Net(%s) add output of latch(%s).\n",cur->name,blif_arch->latches[igl].output.name);}
    /*Check if invalid */
    if (cur->input_num > 1)
    {
      printf("Error: Multiple drivers(%d) to net(%s)!\n", cur->input_num, cur->name);
      exit(1);
    }
  }

  /*allocate memory for each net*/
  net_num = inet;
  cur = head->next;
  while(cur)
  {
    cur->outputs_types = (enum e_blif_type*)my_malloc(cur->output_num*sizeof(int));
    cur->outputs = (int*)my_malloc(cur->output_num*sizeof(int));
    cur = cur->next;  
  }
  
  /*Second scan*/
  /*Assign outputs of each net*/
  /*Scan libgates*/
  for (igl=0; igl<blif_arch->libgate_num; igl++)
  {
    /*Assign outputs of conkt*/
    for(jin=0; jin<blif_arch->libgates[igl].input_num; jin++)
    {
      /*Try to find a matched net name*/
      cur = spot_conkt_name(head,blif_arch->libgates[igl].inputs[jin].name);
      if (NULL == cur)
      {
        printf("Error: Fail to spot target(%s) in the second scan!\nThis may be caused by memory crash or overlapped. Please re-run mpack!\n",blif_arch->libgates[igl].inputs[jin].name);  
        exit(1);
      }
      cur->outputs_types[cur->iout] = BLIF_LIBGATE;
      cur->outputs[cur->iout] = igl;
      cur->iout++;
      /*Check output number valid*/
      if (cur->iout > cur->output_num)
      {
        printf("Error: output number(%d) of net(%s) is more than expected(%d)!\n",cur->iout, cur->name, cur->output_num);
        exit(1);
      }
    }
  }
  
  /*Second scan*/
  /*Assign outputs of each net*/
  /*Scan gates*/
  for (igl=0; igl<blif_arch->gate_num; igl++)
  {
    /*Assign outputs of conkt*/
    for(jin=0; jin<blif_arch->gates[igl].input_num; jin++)
    {
      /*Try to find a matched net name*/
      cur = spot_conkt_name(head,blif_arch->gates[igl].inputs[jin].name);
      if (NULL == cur)
      {
        printf("Error: Fail to spot target(%s) in the second scan!\nThis may be caused by memory crash or overlapped. Please re-run mpack!\n",blif_arch->gates[igl].inputs[jin].name);  
        exit(1);
      }
      cur->outputs_types[cur->iout] = BLIF_GATE;
      cur->outputs[cur->iout] = igl;
      cur->iout++;
      /*Check output number valid*/
      if (cur->iout > cur->output_num)
      {
        printf("Error: output number(%d) of net(%s) is more than expected(%d)!\n",cur->iout, cur->name, cur->output_num);
        exit(1);
      }
    }
  }

  /*Second scan*/
  /*Assign outputs of each net*/
  /*Scan latches*/
  for (igl=0; igl<blif_arch->latch_num; igl++)
  {
    /*Try to find a matched net name*/
    cur = spot_conkt_name(head,blif_arch->latches[igl].input.name);
    if (NULL == cur)
    {
      printf("Error: Fail to spot target(%s) in the second scan!\nThis may be caused by memory crash or overlapped. Please re-run mpack!\n",blif_arch->latches[igl].input.name);  
      exit(1);
    }
    cur->outputs_types[cur->iout] = BLIF_LATCH;
    cur->outputs[cur->iout] = igl;
    cur->iout++;
    /*Check output number valid*/
    if (cur->iout > cur->output_num)
    {
      printf("Error: output number(%d) of net(%s) is more than expected(%d)!\n",cur->iout, cur->name, cur->output_num);
      exit(1);
    }
  } 

  /*Get maximum fan-out */
  cur = head->next;
  while(cur->next)
  {
    if (cur->output_num > max_fanout)
    {
      max_fanout = cur->output_num;
      net_max_fout = cur;
    }
    cur = cur->next;
  }

  /* Print the statistics*/
  printf("%d nets are generated!\n",net_num);
  printf("Net(%s) has maximium fanout: %d.\n", net_max_fout->name, max_fanout);

  if (NULL == head)
  {
    printf("Error: zero net generated! Blif is illegal!\n");
    exit(1);
  }

  /*Mark the PI and PO in conkts*/
  printf("Mark PI&PO in the nets...");
  mark_pi_po_conkt(blif_arch,head);
  printf("Complete!\n");
  
  return head;
}

/**
 * Make a new connection ready
 */
t_conkt* ready_new_conkt(t_conkt* cur,
                         char* name,
                         int net_cnt)
{
  t_conkt* ret;


  if (NULL == cur)
  {
    printf("Error: current node is invaild! Fail to insert conkt node!\n");
    exit(1);
  }
  if (NULL == name)
  {
    printf("Error: try to assign blank name to a new conkt!\n");
    exit(1);
  }
  /*Allocate memory*/
  insert_conkt_list_node(cur,sizeof(t_conkt));
  /*Initial*/
  ret = cur->next;
  init_conkt(ret);
  /*Initial current net*/
  ret->idx = net_cnt;
  ret->name = my_strdup(name);

  return ret;
}

/**
 * Create fixed length of Linked list
 * Node struct must have a pointor named after "next"!
 * The head of this linked list is a scout. Formal start is the second element. 
 */
t_conkt* create_conkt_list(int len,
                         size_t size_node)
{
  t_conkt* head;
  t_conkt* tmp_head;
  int ind;

  /* Create a scout, this is a blank node*/
  head = my_malloc(size_node);
  head->next = NULL;
  
  for (ind=0; ind<len; ind++)
  {
    /* Create a node ahead of the current*/
    tmp_head = my_malloc(size_node); 
    tmp_head->next = head;
    head = tmp_head;
  }
  
  return head; 
}

/**
 * Insert a node inside the linked list
 * Cur is pointer which a new node will be insertedafter.
 */
t_conkt* insert_conkt_list_node(t_conkt* cur,
                             size_t size_node)
{
  t_conkt* cur_next; 
  /* Store the current next*/  
  cur_next = cur->next;
  /* Allocate new node*/
  cur->next = my_malloc(size_node);
  /* Configure the new node*/
  cur->next->next = cur_next;

  return cur_next;
}

/**
 * Romove a node from linked list
 * cur is the node whose next node is to be removed
 */
void remove_conkt_list_node(t_conkt* cur)
{ 
  t_conkt* rm_node = cur->next;
  /* Connect the next next node*/ 
  cur->next = cur->next->next;
  /* free the node*/
  free(rm_node);
}

/**
 * Cat the linked list
 * head2 is connected to the tail of head1
 * return head1
 */
t_conkt* cat_conkt_lists(t_conkt* head1,
                         t_conkt* head2)
{
  t_conkt* tmp = head1;

  /* Reach the tail of head1*/ 
  while(tmp->next != NULL)
  {
    tmp = tmp->next;
  }
  /* Cat*/
  tmp->next = head2->next;
  /* Free head2*/
  free(head2);
  
  return head1;
}

/* Initialize connection node*/
void init_conkt(t_conkt* conkt)
{
  conkt->idx = -1;
  conkt->name = NULL;
  conkt->input_num = 0;
  conkt->inputs_types = BLIF_NONE;
  conkt->inputs = -1;
  conkt->output_num = 0;
  conkt->outputs_types = NULL;
  conkt->outputs = NULL;
  //conkt->next = NULL;
  conkt->is_pi = 0;
  conkt->pi_idx = -1;
  conkt->is_po = 0;
  conkt->po_idx = -1;
  conkt->iout = 0;
}


/* Spot the conkt with given name*/
t_conkt* spot_conkt_name(t_conkt* head,
                         char* name)
{
  t_conkt* tmp = head;
  /*Check name =? NULL*/
  if (NULL == name)
  {
    printf("Warning: Given target name of conkt is NULL!\n");
    return NULL;
  }
  /*Check head =? NULL*/
  if (NULL == head)
  {
    printf("Warning: Given head of conkt is NULL!\n");
    return NULL;
  }
  /*Cross over the scout*/
  tmp = head->next;

  while(tmp)
  {
    if (NULL == tmp->name)
    {continue;}
    if (0 == strcmp(tmp->name, name))
    {
      return tmp;
    }
    tmp = tmp->next;
  } 
  return NULL;
}

void mark_pi_po_conkt(t_blif_arch* blif_arch,
                      t_conkt* nets)
{
  t_conkt* head = nets;
  int im;
  int pi_num = 0;
  int po_num = 0;

  while(head->next)
  {
    head = head->next;
    /*Check if it is PI*/
    for (im=0; im<blif_arch->input_num; im++)
    {
      if (0 == strcmp(blif_arch->inputs[im].name,head->name))
      {
        /* This is a PI, Label it*/
        head->is_pi = 1;
        head->pi_idx = im;
        /*Update counter*/
        pi_num++;
        //printf("Mark input[%s] as PI!\n",head->name);
      }
    }
    /*Check if it is PO*/
    for (im=0; im<blif_arch->output_num; im++)
    {
      if (0 == strcmp(blif_arch->outputs[im].name,head->name))
      {
        /* This is a PI, Label it*/
        head->is_po = 1;
        head->po_idx = im;
        /*Update counter*/
        po_num++;
        //printf("Mark output[%s] as PO!\n",head->name);
      }
    }
    /*Check if this is both PI and PO*/
    if ((1 == head->is_pi*head->is_po))
    {
      printf("Warning: net(%s) performs as both PI and PO! This may cause problems in synthesis...\n", head->name);
    }
  } 
  printf("\n%d PI and %d PO Marked!\n",pi_num,po_num);
}

/**
 * Free blif_ios
 */
void free_blif_ios(t_blif_ios* blif_ios)
{
  if (blif_ios->port) {
    free(blif_ios->port);
  }
  
  if (blif_ios->name != blif_ios->port) {
    free(blif_ios->name);
  } 
}
 
/**
 * Free blif_gate
 */
void free_blif_gate(t_blif_gate* blif_gate)
{
  int i;

  for (i=0; i<blif_gate->input_num; i++) {
    free_blif_ios(&blif_gate->inputs[i]);
  }
  free(blif_gate->inputs);
}

/**
 * Free blif_libgate
 */
void free_blif_libgate(t_blif_libgate* blif_libgate)
{
  int i;

  for (i=0; i<blif_libgate->input_num; i++) {
    free_blif_ios(&blif_libgate->inputs[i]);
  }
  free(blif_libgate->inputs);
  free(blif_libgate->name);
}

/**
 * Free blif latch
 */
void free_blif_latch(t_blif_latch* blif_latch)
{
  free_blif_ios(&blif_latch->input);
  free_blif_ios(&blif_latch->output);

  //printf("Blif latch type: %s\n",blif_latch->type);
  //printf("Blif latch clk: %s\n",blif_latch->clk);

  if (NULL != blif_latch->type) {
    //free(blif_latch->type);
  }
  if (NULL != blif_latch->clk) {
    //free(blif_latch->clk);
  }
}

/**
 * Free conkts
 */
void free_conkt(t_conkt* conkt)
{
  free(conkt->name);
  free(conkt->outputs_types);
  free(conkt->outputs);
}

/**
 * Free conkt list
 */
void free_conkts_list(t_conkt* head)
{
  if (NULL != head->next) {
    free_conkt(head);
    free_conkts_list(head->next);
  }
  free(head);
 
}

/**
 * Free blif arch
 */
void free_blif_arch(t_blif_arch* blif_arch)
{
  int i;
  /*Free Model Name*/
  free(blif_arch->model);
  /*Free Inputs*/
  //printf("Free inputs...\n");
  for (i=0; i<blif_arch->input_num; i++) {
    //printf("Free inputs[%d]...\n",i);
    free_blif_ios(&blif_arch->inputs[i]);
  }
  free(blif_arch->inputs);
  /*Free Outputs*/
  for (i=0; i<blif_arch->output_num; i++) {
    free_blif_ios(&blif_arch->outputs[i]);
  }
  free(blif_arch->outputs);
  /*Free Clks*/
  for (i=0; i<blif_arch->clk_num; i++) {
    free_blif_ios(&blif_arch->clk_names[i]);
  }
  free(blif_arch->clk_names);
  /*Free gates*/
  //printf("Free gates...\n");
  for (i=0; i<blif_arch->gate_num; i++) {
    free_blif_gate(&blif_arch->gates[i]);
  }
  if (blif_arch->gates) {
    free(blif_arch->gates);
  }
  /*Free libgates*/
  //printf("Free libgates...\n");
  for (i=0; i<blif_arch->libgate_num; i++) {
    free_blif_libgate(&blif_arch->libgates[i]);
  }
  if (blif_arch->libgates) {
    free(blif_arch->libgates);
  }
  /*Free Latches*/
  //printf("Free latches...\n");
  for (i=0; i<blif_arch->latch_num; i++) {
    free_blif_latch(&blif_arch->latches[i]);
  }
  if (blif_arch->latches) {
    free(blif_arch->latches);
  }
  /*Free conkts*/
  //printf("Free Connections...\n");
  free_conkts_list(blif_arch->nets);

}
