#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
//#include <malloc.h>
#include "util.h"
#include "mpack_conf.h"
#include "mpack_types.h"
#include "blearch_parser.h"

void init_ble_arch(t_ble_arch* ble_arch)
{
  ble_arch->name = NULL;
  ble_arch->input_num = 0;
  ble_arch->inputs = NULL;
  ble_arch->output_num = 0;
  ble_arch->outputs = NULL;
  ble_arch->blend_num = 0;
  ble_arch->blends = NULL;
  ble_arch->curnd = 0;
  ble_arch->area = 0.0;
  ble_arch->rmetal = 0.0;
  ble_arch->cmetal = 0.0;
  ble_arch->cell_delay = 0.0;
  ble_arch->inner_cluster_delay = 0.0;
}

void init_blend(t_blend* blend)
{
  blend->idx = -1;
  blend->name = NULL;
  blend->input_num = -1;
  blend->inputs = NULL;
  blend->sat2_capable = -1;
  blend->sat3_capable = -1;
  blend->pattern_type = -1;
  blend->curin = 0;
  blend->priority= 0.0;
  blend->depth = 0;
}

void init_bleport(t_bleport* bleport)
{
  bleport->idx = -1;
  bleport->name = NULL;
  bleport->cand_num = -1;
  bleport->candtoks = NULL;
  bleport->cands = NULL;
  bleport->type = 0;
  bleport->blend_idx = -1;
  bleport->is_comb = -1;
  bleport->inv_capable = -1;
}

/*Fill the BLE Arch Name, just for backup*/
int fill_ble_arch_name(t_ble_arch* ble_arch,
                       char** tokens,
                       int token_len)
{
  if (token_len > 2)
  {
    printf("Warning: space isn't allowed in BLE arch name! Only the first string is valid.\n");
  }
  /* Copy the name of ble_arch*/
  ble_arch->name = my_strdup(tokens[1]);

  return 1;
}

/*Fill the BLE arch inputs*/
int fill_ble_arch_inputs(t_ble_arch* ble_arch,
                         char** tokens,
                         int token_len)
{
  int im;  
  
  for(im=0; im<ble_arch->input_num; im++)
  {
    init_bleport(&ble_arch->inputs[im]);
    ble_arch->inputs[im].idx = im;  
    ble_arch->inputs[im].name = my_strdup(tokens[im+1]);
    ble_arch->inputs[im].cand_num = 0;
    ble_arch->inputs[im].cands = NULL;
    ble_arch->inputs[im].type = BLE_IN;
    ble_arch->inputs[im].blend_idx = -1;
    ble_arch->inputs[im].is_comb = 0;
  }

  return 1;
}

/*Fill the BLE arch outputs*/
int fill_ble_arch_outputs(t_ble_arch* ble_arch,
                          char** tokens,
                          int token_len) 
{
  int otok_len;
  char** otoks;
  int candtok_len;
  char** candtoks;
  int ip;
  
  for(ip=0; ip<ble_arch->output_num; ip++)
  {
    /*Split the tokens define Output*/
    otoks = my_strtok(tokens[ip+1],"=",&otok_len); 
    if (otok_len > 2)
    {
      printf("Warning: Redundant equation found in Output definition!(%s)\n",tokens[ip+1]);
    }
    if (otok_len < 2)
    {
      printf("Error: Ilegal Output definition!(%s)\n",tokens[ip+1]);
      exit(1);
    }
    candtoks = my_strtok(otoks[1],"|",&candtok_len);
    if (0 == candtok_len)
    {
      printf("Error: Zero candidates for output connections!(%s)\n",tokens[ip+1]);
      exit(1);
    }
    /*Initial*/
    init_bleport(&ble_arch->outputs[ip]);
    /*Fill info*/
    ble_arch->outputs[ip].idx = ip;
    ble_arch->outputs[ip].name = my_strdup(otoks[0]);
    /*Assign candidates*/
    ble_arch->outputs[ip].cand_num = candtok_len;
    ble_arch->outputs[ip].candtoks = candtoks;
    ble_arch->outputs[ip].cands = (t_bleport**)my_malloc(sizeof(t_bleport*)*ble_arch->outputs[ip].cand_num);
    ble_arch->outputs[ip].type = BLE_OUT;
    ble_arch->outputs[ip].blend_idx = -1;
    ble_arch->outputs[ip].is_comb = 0;

    free_tokens(otoks,otok_len);
  }

  return 1;
}

/*Fill BLE arch nodes*/
int fill_ble_arch_blends(t_ble_arch* ble_arch,
                         char** tokens,
                         int token_len)
{
  int itok; 
  int eqtok_len;
  char** eqtoks;
  int candtok_len;
  char** candtoks;
  int ndin_num = 0;
  int curin;

  /*Initial*/
  init_blend(&ble_arch->blends[ble_arch->curnd]);

  ble_arch->blends[ble_arch->curnd].curin = 0;
  curin = ble_arch->blends[ble_arch->curnd].curin;
   

  for (itok=1; itok<token_len; itok++)
  {
    eqtoks = my_strtok(tokens[itok],"=",&eqtok_len); 
    if (eqtok_len > 2)
    {
      printf("Warning: Redundant equation found in cell definition!(%s)\n",tokens[itok]);
    }
    if (eqtok_len < 2)
    {
      printf("Error: Ilegal cell definition!(%s)\n",tokens[itok]);
      exit(1);
    }
    
    if (0 == strcmp(eqtoks[0],"index"))
    {
      ble_arch->blends[ble_arch->curnd].idx = my_atoi(eqtoks[1]);
      ble_arch->blends[ble_arch->curnd].name = (char*)my_malloc(sizeof(char)*(5+strlen(eqtoks[1])));
      strcpy(ble_arch->blends[ble_arch->curnd].name,"cell");
      strcat(ble_arch->blends[ble_arch->curnd].name,eqtoks[1]);
    }
    else if (0 == strcmp(eqtoks[0],"priority"))
    {
      ble_arch->blends[ble_arch->curnd].priority = my_atof(eqtoks[1]);
    }
    else
    {ndin_num++;}
    
    free_tokens(eqtoks,eqtok_len);
  } 
  
  /*Check input number >0*/
  if (ndin_num < 1)
  {
    printf("Error: %s has no inputs!\n",ble_arch->blends[ble_arch->curnd].name);
    exit(1);
  }

  /*Determine input number and malloc*/
  ble_arch->blends[ble_arch->curnd].input_num = ndin_num;
  ble_arch->blends[ble_arch->curnd].inputs = (t_bleport*)my_malloc(sizeof(t_bleport)*ble_arch->blends[ble_arch->curnd].input_num);
  

  /*Scan twice*/
  for (itok=1; itok<token_len; itok++)
  {
    eqtoks = my_strtok(tokens[itok],"=",&eqtok_len); 
    if (eqtok_len > 2)
    {
      printf("Warning: Redundant equation found in cell definition!(%s)\n",tokens[itok]);
    }
    if (eqtok_len < 2)
    {
      printf("Error: Ilegal cell definition!(%s)\n",tokens[itok]);
      exit(1);
    }
    
    if (0 == strcmp(eqtoks[0],"index"))
    {}
    else if (0 == strcmp(eqtoks[0],"priority"))
    {}
    else
    {
      /*Fill in the inputs info*/
      /*Basic infomation*/
      ble_arch->blends[ble_arch->curnd].inputs[curin].idx = curin;
      ble_arch->blends[ble_arch->curnd].inputs[curin].name = my_strdup(eqtoks[0]);
      /*Candidates*/      
      candtoks = my_strtok(eqtoks[1],"|",&candtok_len);
      if (0 == candtok_len)
      {
        printf("Error: Zero candidates for BLE node connections!(%s)\n",ble_arch->blends[ble_arch->curnd].name);
        exit(1);
      }
      
      ble_arch->blends[ble_arch->curnd].inputs[curin].cand_num = candtok_len;
      ble_arch->blends[ble_arch->curnd].inputs[curin].candtoks = candtoks;
      ble_arch->blends[ble_arch->curnd].inputs[curin].cands = (t_bleport**)my_malloc(sizeof(t_bleport*)*ble_arch->blends[ble_arch->curnd].inputs[curin].cand_num);

      ble_arch->blends[ble_arch->curnd].inputs[curin].type = BLE_ND;
      ble_arch->blends[ble_arch->curnd].inputs[curin].blend_idx = ble_arch->curnd;
      ble_arch->blends[ble_arch->curnd].inputs[curin].is_comb = -1;
      
      /*Incremental curin*/
      curin++; 
    }

    free_tokens(eqtoks,eqtok_len);
  }
  
  ble_arch->blends[ble_arch->curnd].curin = curin;
  ble_arch->curnd++;

  return 1;
}

/**
 * Check the name of each combinational output,
 * Try to match it in the output list
 */
int fill_ble_arch_combouts(t_ble_arch* ble_arch,
                           char** tokens,
                           int token_len)
{
  int itok;
  int ip;
  int match = 0;

  for (itok=1; itok<token_len; itok++)
  {
    match = 0;
    for (ip=0; ip<ble_arch->output_num; ip++)
    {
      if (0 == strcmp(ble_arch->outputs[ip].name,tokens[itok]))
      {
        ble_arch->outputs[ip].is_comb = 1;
        match = 1;
      }
    }
    /*Check if there is match*/
    if (0 == match)
    {
      printf("Warning: combinational output(%s) isn't in the output pin list!\n", tokens[itok]);
    }
  }
   
  return 1;
}

/*Spot the candidate name in input, output, cells*/
t_bleport* spot_cand_ptr(t_ble_arch* ble_arch,
                         char* cand)
{
  int i;
  t_bleport* ret = NULL;
  
  /*Scan inputs*/
  for(i=0; i<ble_arch->input_num; i++)
  {
    if (0 == strcmp(ble_arch->inputs[i].name,cand))
    {
      ret = &(ble_arch->inputs[i]); 
      return ret;
    }
  }

  /*Scan outputs*/
  for(i=0; i<ble_arch->output_num; i++)
  {
    if (0 == strcmp(ble_arch->outputs[i].name,cand))
    {
      ret = &(ble_arch->outputs[i]); 
      return ret;
    }
  }

  /*Scan BLE node*/
  for(i=0; i<ble_arch->blend_num; i++)
  {
    if (0 == strcmp(ble_arch->blends[i].name,cand))
    {
      /*If match a BLE node, return the pointor
       * of first input. Check if it has at least one input
       */
      if (ble_arch->blends[i].input_num)
      {ret = &(ble_arch->blends[i].inputs[0]);}
      else
      {
        printf("Error: No inputs found to cell%d!\n",i);
        exit(1);
      }

      return ret;
    }
  }

  if (NULL == ret)
  {
    printf("Error: given name(%s) has no match in BLE inputs, outputs, and cells!\n",cand);
    exit(1);
  }

  return ret; 
}

/*Search the outputs and cells, 
 *try to match the candidate name in outputs, cells, and inputs
 */
int assign_ble_candidates_ptr(t_ble_arch* ble_arch)
{
  int i;  
  int im;
  int itok;

  /*Scan BLE outputs. Fill their cands*/    
  for (i=0; i<ble_arch->output_num; i++)
  {
    for(itok=0; itok<ble_arch->outputs[i].cand_num; itok++)
    {
      ble_arch->outputs[i].cands[itok] = spot_cand_ptr(ble_arch,ble_arch->outputs[i].candtoks[itok]);
    }
  }
  
  /*Scan BLE nodes.*/
  for (i=0; i<ble_arch->blend_num; i++)
  {
    for (im=0; im<ble_arch->blends[i].input_num; im++)
    {
      for (itok=0; itok<ble_arch->blends[i].inputs[im].cand_num; itok++)
      {
        ble_arch->blends[i].inputs[im].cands[itok] = spot_cand_ptr(ble_arch,ble_arch->blends[i].inputs[im].candtoks[itok]); 
      }
    }
  }

  return 1;
}

int parse_ble_arch(char* ble_arch_file,
                   t_ble_arch* ble_arch)
{
  char buffer[BUFSIZE];
  int feof;
  char* line = NULL;
  int linenum = 0;
  char* delims = " ";
  int token_len;
  char** tokens;
  FILE* fblearch;

  int input_num = 0;
  int output_num = 0;
  int combout_num = 0;
  int cell_num = 0;

  /*Initial BLE arch*/
  init_ble_arch(ble_arch);

  printf("Read BLE architecture (File:%s)...\n", ble_arch_file);   
  /* Open File read-only*/
  fblearch = my_fopen(ble_arch_file,"r");

  /**
   * First scan of file, count the number of 
   * inputs, outputs, cell and combinational output
   */
  while(1)
  {
    /*Get a fully-processed line*/
    line = full_freadlns(buffer,BUFSIZE,fblearch,ble_arch_file,&linenum,&feof);
    /*Check if it is End Of File*/
    if (-1 == feof)
    {break;}
    /*Split the line by delims*/
    tokens = my_strtok(line,delims,&token_len);
    
    /* Check if it is End Of File*/
    if (0 == strcmp(tokens[0],".end"))
    {break;}
  
    /* Check Keywords*/
    if (0 == strcmp(tokens[0],".arch"))
    {
      fill_ble_arch_name(ble_arch,tokens,token_len); 
    } 
    if (0 == strcmp(tokens[0],".input"))
    {
      if (input_num)
      {
        printf("Error: Intend to redefine .input in file(%s) line(%d)!\n", ble_arch_file,linenum);
        exit(1);
      }
      input_num = token_len - 1; 
      /*Check input number >0*/
      if (input_num < 1)
      {
        printf("Error: BLE has no inputs!\n");
        exit(1);
      }
      ble_arch->input_num = input_num;
      ble_arch->inputs = (t_bleport*)my_malloc(sizeof(t_bleport)*ble_arch->input_num);
      fill_ble_arch_inputs(ble_arch,tokens,token_len);
    }
    if (0 == strcmp(tokens[0],".cell"))
    {
      cell_num++;
    }
    if (0 == strcmp(tokens[0],".output"))
    {
      if (output_num)
      {
        printf("Error: Intend to redefine .output in file(%s) line(%d)!\n", ble_arch_file,linenum);
        exit(1);
      }
      output_num = token_len - 1; 
      /*Check output number >0*/
      if (output_num < 1)
      {
        printf("Error: BLE has no outputs!\n");
        exit(1);
      }
      ble_arch->output_num = output_num;
      ble_arch->outputs = (t_bleport*)my_malloc(sizeof(t_bleport)*ble_arch->output_num);
      fill_ble_arch_outputs(ble_arch,tokens,token_len);
    }
    if (0 == strcmp(tokens[0],".combout"))
    {
      if (combout_num)
      {
        printf("Error: Intend to redefine .combout in file(%s) line(%d)!\n", ble_arch_file,linenum);
        exit(1);
      }
      combout_num += token_len - 1;
    }
    
    /*Clean and Reset memory*/
    memset(buffer,0,sizeof(char)*BUFSIZE);
    free(line);
    free(tokens);
  }

  /*Check combinational output number overflows*/
  if (combout_num > output_num)
  {
    printf("Error: Combination output number exceeds the number of outputs");
    exit(1);
  }
  /**
   * Define number of cells, inputs, outputs
   * Malloc cells, inputs, and outputs
   */
  ble_arch->blend_num = cell_num;
  
  /*Malloc*/  
  ble_arch->blends = (t_blend*)my_malloc(sizeof(t_blend)*ble_arch->blend_num);

 /**
  * Second scan BLE arch file
  * fill info of cells, inputs, and outputs
  */ 
  rewind(fblearch);
  memset(buffer,0,sizeof(char)*BUFSIZE);
  free(line);
  linenum = 0;
  while(1)
  {
    /*Get a fully-processed line*/
    line = full_freadlns(buffer,BUFSIZE,fblearch,ble_arch_file,&linenum,&feof);
    /*Check if it is End Of File*/
    if (-1 == feof)
    {break;}
    /*Split the line by delims*/
    tokens = my_strtok(line,delims,&token_len);
    
    /* Check if it is End Of File*/
    if (0 == strcmp(tokens[0],".end"))
    {break;}
  
    /* Check Keywords*/
    if (0 == strcmp(tokens[0],".arch"))
    {} 
    if (0 == strcmp(tokens[0],".input"))
    {
    }
    if (0 == strcmp(tokens[0],".cell"))
    {
      fill_ble_arch_blends(ble_arch,tokens,token_len);
    }
    if (0 == strcmp(tokens[0],".output"))
    {
    }
    if (0 == strcmp(tokens[0],".combout"))
    {
      fill_ble_arch_combouts(ble_arch,tokens,token_len);
      /*Clean and Reset memory*/
      memset(buffer,0,sizeof(char)*BUFSIZE);
      free(line);
      continue;
    }
    if (0 == strcmp(tokens[0],".area")) {
      assert(token_len == 2);
      ble_arch->area = my_atof(tokens[1]); 
    }
    if (0 == strcmp(tokens[0],".rmetal")) {
      assert(token_len == 2);
      ble_arch->rmetal = my_atof(tokens[1]); 
    }
    if (0 == strcmp(tokens[0],".cmetal")) {
      assert(token_len == 2);
      ble_arch->cmetal = my_atof(tokens[1]); 
    }
    if (0 == strcmp(tokens[0],".cell_delay")) {
      assert(token_len == 2);
      ble_arch->cell_delay = my_atof(tokens[1]); 
    }
    if (0 == strcmp(tokens[0],".inner_cluster_delay")) {
      assert(token_len == 2);
      ble_arch->inner_cluster_delay = my_atof(tokens[1]); 
    }
    
    /*Clean and Reset memory*/
    memset(buffer,0,sizeof(char)*BUFSIZE);
    free(line);
    free(tokens);
  }

  printf("Input No.: %d, Output No.: %d, Cell No.: %d.\n",ble_arch->input_num,ble_arch->output_num,ble_arch->blend_num);
  printf("Configure BLE internal connections...");
  assign_ble_candidates_ptr(ble_arch);

  printf("Complete!\n");

  return 1;
}

/**
 * Free BLE port
 */
void free_ble_port(t_bleport* ble_port)
{
  int i;

  //free(ble_port->name);  
  for (i=0; i<ble_port->cand_num; i++) {
    //free(ble_port->candtoks[i]);
  }
  if (ble_port->cand_num > 1) {
    free(ble_port->cands);
  }
  
}


/**
 * Free BLE node
 */
void free_blend(t_blend* blend)
{
  int i;

  //free(blend->name);
  for (i=0; i<blend->input_num; i++) {
    free_ble_port(&blend->inputs[i]);
  }
  free(blend->inputs);
}

/**
 * Free BLE arch
 */
void free_ble_arch(t_ble_arch* ble_arch)
{
  int i;

  //free(ble_arch->name);
  /*Free inputs*/
  for (i=0; i<ble_arch->input_num; i++) {
    free_ble_port(&ble_arch->inputs[i]);
  }
  free(ble_arch->inputs);
  /*Free outputs*/
  for (i=0; i<ble_arch->output_num; i++) {
    free_ble_port(&ble_arch->outputs[i]);
  }
  free(ble_arch->outputs);
  /*Free blends*/
  for (i=0; i<ble_arch->blend_num; i++) {
    free_blend(&ble_arch->blends[i]);
  }
  free(ble_arch->blends);

}
