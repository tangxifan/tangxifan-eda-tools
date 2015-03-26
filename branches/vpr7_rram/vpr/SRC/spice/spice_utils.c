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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Include vpr structs*/
#include "util.h"
#include "physical_types.h"
#include "vpr_types.h"
#include "globals.h"
#include "rr_graph_util.h"
#include "rr_graph.h"
#include "rr_graph2.h"
#include "vpr_utils.h"

/* Include SPICE support headers*/
#include "linkedlist.h"
#include "spice_globals.h"
#include "spice_utils.h"

enum e_dir_err {
 E_DIR_NOT_EXIST,
 E_EXIST_BUT_NOT_DIR,
 E_DIR_EXIST
};

/***** Subroutines *****/
char* my_gettime() {
  time_t current_time;
  char* c_time_string;
  
  /* Obtain current time as seconds elapsed since the Epoch*/
  current_time = time(NULL);
  
  if (current_time == ((time_t)-1)) {
    vpr_printf(TIO_MESSAGE_ERROR,"Failure to compute the current time.\n");
    exit(1);
  }
  
  /* Convert to local time format*/
  c_time_string = ctime(&current_time);
  if (NULL == c_time_string) {
    vpr_printf(TIO_MESSAGE_ERROR,"Failure to convert the current time.\n");
    exit(1);
  }
  /* Return it*/
  return c_time_string;
}

char* format_dir_path(char* dir_path) {
  int len = strlen(dir_path); /* String length without the last "\0"*/
  int i;
  char* ret = (char*)my_malloc(sizeof(char)*(len+2));
  
  strcpy(ret,dir_path);
  /* Replace all the "\" to "/"*/
  for (i=0; i<len; i++) {
    if (ret[i] == '\\') {
      ret[i] = '/'; /* !!! Should not use "" */
    }
  } 
  /* If the path does not end up with "/" we should complete it*/
  if (ret[len-1] != '/') {
    strcat(ret, "/");
  }
  return ret;
}

int try_access_file(char* file_path) {
  /* F_OK checks existence and also R_OK, W_OK, X_OK,
   * for readable, writable, excutable
   */
  int ret = access(file_path,F_OK); 
  if (0 == ret)  {
    vpr_printf(TIO_MESSAGE_WARNING,"File(%s) exists! Will overwrite it!\n",file_path);
  }
  return ret;
}

void my_remove_file(char* file_path) {
  if (NULL == file_path) {
    return;
  } 
  if (0 != remove(file_path)) {
    vpr_printf(TIO_MESSAGE_WARNING, "Fail to remove file(%s)!\n", file_path);
  }
  return;
}

enum e_dir_err try_access_dir(char* dir_path) {
  struct stat s;
  int err = stat(dir_path, &s);
  if (-1 == err) {
     if (ENOENT == errno) {
       return E_DIR_NOT_EXIST;
     } else {
       perror("stat");
       exit(1);
     }
  } else {
    if (S_ISDIR(s.st_mode)) {
      /* It is a dir*/
      return E_DIR_EXIST;
    } else {
      /* Exists but is no dir*/
      return E_EXIST_BUT_NOT_DIR;
    }
  }
}

int create_dir_path(char* dir_path) {
   int ret; 
   
   /* Check this path does not exist*/

   /* Check the input legal*/
   if (NULL == dir_path) {
     vpr_printf(TIO_MESSAGE_INFO,"dir_path is empty and nothing is created.\n");
     return 0;
   }
   ret = mkdir(dir_path, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
   switch (ret) {
   case 0:
     vpr_printf(TIO_MESSAGE_INFO,"Create directory(%s)...successfully.\n",dir_path);
     return 1;
   case -1:
     if (EEXIST == errno) {
       vpr_printf(TIO_MESSAGE_WARNING,"Directory(%s) already exists. Will overwrite SPICE netlists\n",dir_path);
       return 1;
     }
   default:
     vpr_printf(TIO_MESSAGE_ERROR,"Create directory(%s)...Failed!\n",dir_path);
     exit(1);
     return 0;
   }
}

/* Cat string2 to the end of string1 */
char* my_strcat(char* str1,
                char* str2) {
  int len1 = strlen(str1);
  int len2 = strlen(str2);
  char* ret = (char*)my_malloc(sizeof(char) * (len1 + len2 + 1));
  
  strcpy(ret,str1);
  strcat(ret,str2);

  return ret;  
}

/* Split the path and program name*/
int split_path_prog_name(char* prog_path,
                         char  split_token,
                         char** ret_path,
                         char** ret_prog_name) {
  int i;
  int split_pos = -1;
  char* local_copy = my_strdup(prog_path);
  int len = strlen(local_copy);
  char* path = NULL;
  char* prog_name = NULL;
   
  /* Spot the split token*/
  for (i = len; i > -1; i--) {
    if (split_token == local_copy[i]) {
      split_pos = i; 
      break; 
    }
  }

  /* Get the path and prog_name*/
  if (-1 == split_pos) {
    /* In this case, the prog_path actually contains only the program name*/
    path = NULL;
    prog_name = local_copy;
  } else if (len == split_pos) {
    /* In this case the progrom name is NULL... actually the prog_path is a directory*/
    path = local_copy;
    prog_name = NULL;
  } else {
    /* We have to split it!*/
    local_copy[split_pos] = '\0';
    path = my_strdup(local_copy);
    prog_name = my_strdup(local_copy + split_pos + 1);
  } 
  
  /*Copy it to the return*/
  (*ret_path) = my_strdup(path); 
  (*ret_prog_name) = my_strdup(prog_name);

  /* Free useless resources */
  my_free(local_copy);
  my_free(path);
  my_free(prog_name);

  return 1;
}

char* chomp_file_name_postfix(char* file_name) {
  char* ret = NULL;
  char* postfix = NULL;
   
  split_path_prog_name(file_name, '.', &ret, &postfix);

  my_free(postfix);

  return ret;
}

/* With a given spice_model_name, find the spice model and return its pointer
 * If we find nothing, return NULL
 */
t_spice_model* find_name_matched_spice_model(char* spice_model_name,
                                             int num_spice_model,
                                             t_spice_model* spice_models) {
  t_spice_model* ret = NULL;
  int imodel;
  int num_found = 0;

  for (imodel = 0; imodel < num_spice_model; imodel++) {
    if (0 == strcmp(spice_model_name, spice_models[imodel].name)) {
      ret = &(spice_models[imodel]);
      num_found++;
    }
  }

  assert(1 == num_found);

  return ret;
}

/* Get the default spice_model*/
t_spice_model* get_default_spice_model(enum e_spice_model_type default_spice_model_type,
                                       int num_spice_model,
                                       t_spice_model* spice_models) {
  t_spice_model* ret = NULL;
  int i;
  
  for (i = 0; i < num_spice_model; i++) {
    /* Find a MUX and it is set as default*/
    if ((default_spice_model_type == spice_models[i].type)&&(1 == spice_models[i].is_default)) {
      /* Check if we have multiple default*/
      if (NULL != ret) {
        vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,LINE[%d])Both SPICE model(%s and %s) are set as default!\n",
                   __FILE__, __LINE__, ret->name, spice_models[i].name);
        exit(1);
      } else {
        ret = &(spice_models[i]); 
      }
    }
  }
  
  return ret;
}

/* Return the SPICE model ports wanted
 * ATTENTION: we use the pointer of spice model here although we don't modify anything of spice_model 
 *            but we have return input ports, whose pointer will be lost if the input is not the pointor of spice_model
 * BECAUSE spice_model will be a local copy if it is not a pointer. And it will be set free when this function
 * finishes. So the return pointers become invalid !
 */
t_spice_model_port** find_spice_model_ports(t_spice_model* spice_model,
                                            enum e_spice_model_port_type port_type,
                                            int* port_num) {
  int iport, cur;
  t_spice_model_port** ret = NULL;

  /* Check codes*/
  assert(NULL != port_num);
  assert(NULL != spice_model);

  /* Count the number of ports that match*/
  (*port_num) = 0;
  for (iport = 0; iport < spice_model->num_port; iport++) {
    if (port_type == spice_model->ports[iport].type) {
      (*port_num)++;
    }
  }
  
  /* Initial the return pointers*/
  ret = (t_spice_model_port**)my_malloc(sizeof(t_spice_model_port*)*(*port_num));
  memset(ret, 0 , sizeof(t_spice_model_port*)*(*port_num));
  
  /* Fill the return pointers*/
  cur = 0;
  for (iport = 0; iport < spice_model->num_port; iport++) {
    if (port_type == spice_model->ports[iport].type) {
      ret[cur] = &(spice_model->ports[iport]);
      cur++;
    }
  }
  /* Check correctness*/
  assert(cur == (*port_num));
  
  return ret;
}

/* Find the transistor in the tech lib*/
t_spice_transistor_type* find_mosfet_tech_lib(t_spice_tech_lib tech_lib,
                                              e_spice_trans_type trans_type) {
  /* If we did not find return NULL*/
  t_spice_transistor_type* ret = NULL;
  int i;

  for (i = 0; i < tech_lib.num_transistor_type; i++) {
    if (trans_type == tech_lib.transistor_types[i].type) {
      ret = &(tech_lib.transistor_types[i]); 
      break;
    }
  }
  
  return ret; 
}

/* Convert a integer to a string*/
char* my_itoa(int input) {
  char* ret = NULL;
  int sign = 0;
  int len = 0;
  int temp = input;
  int cur;
  char end_of_str;

  /* Identify input number is positive or negative*/
  if (input < 0) {
    sign = 1; /* sign will be '-'*/
    len = 1;
    temp = 0 - input;
  } else if (0 == input) {
    sign = 0;
    len = 2;
    /* Alloc*/
    ret = (char*)my_malloc(sizeof(char)*len);
    /* Lets get the end_of_str, the char is dependent on OS*/
    sprintf(ret,"%s","0");
    return ret;
  }
  /* Identify the length of string*/
  while(temp > 0) {
    len++;
    temp = temp/10;
  }
  /* Total length of string should include '\0' at the end*/
  len = len + 1;
  /* Alloc*/
  ret = (char*)my_malloc(sizeof(char)*len);

  /*Fill it*/
  temp = input;
  /* Lets get the end_of_str, the char is dependent on OS*/
  sprintf(ret,"%s","-");
  end_of_str = ret[1];
  ret[len-1] = end_of_str;
  cur = len - 2;
  /* Print the number reversely*/
  while(temp > 0) {
    ret[cur] = temp%10 + '0'; /* ASIC II base is '0'*/
    cur--;
    temp = temp/10;
  }
  /* Print the sign*/
  if (1 == sign) {
    assert(0 == cur);
    ret[cur] = '-';
    temp = 0 - input;
  } else {
    assert(-1 == cur);
  }

  return ret;
}

/* With given spice_model_port, find the pb_type port with same name and type*/
t_port* find_pb_type_port_match_spice_model_port(t_pb_type* pb_type,
                                                 t_spice_model_port* spice_model_port) {
  int iport;
  t_port* ret = NULL;

  /* Search ports */
  for (iport = 0; iport < pb_type->num_ports; iport++) {
    /* Match the name and port size*/
    if ((0 == strcmp(pb_type->ports[iport].name, spice_model_port->prefix)) 
      &&(pb_type->ports[iport].num_pins == spice_model_port->size)) {
      /* Match the type*/
      switch (spice_model_port->type) {
      case SPICE_MODEL_PORT_INPUT:
        if ((IN_PORT == pb_type->ports[iport].type)
          &&(0 == pb_type->ports[iport].is_clock)) {
          if (NULL != ret) {
            vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])More than 1 pb_type(%s) port match spice_model_port(%s)!\n",
                       __FILE__, __LINE__, pb_type->name, spice_model_port->prefix);
            exit(1);
          }
          ret = &(pb_type->ports[iport]);
        }
        break;
      case SPICE_MODEL_PORT_OUTPUT:
        if (OUT_PORT == pb_type->ports[iport].type) {
          if (NULL != ret) {
            vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])More than 1 pb_type(%s) port match spice_model_port(%s)!\n",
                       __FILE__, __LINE__, pb_type->name, spice_model_port->prefix);
            exit(1);
          }
          ret = &(pb_type->ports[iport]);
        }
        break;
      case SPICE_MODEL_PORT_CLOCK:
        if ((IN_PORT == pb_type->ports[iport].type)&&(1 == pb_type->ports[iport].is_clock)) {
          if (NULL != ret) {
            vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])More than 1 pb_type(%s) port match spice_model_port(%s)!\n",
                       __FILE__, __LINE__, pb_type->name, spice_model_port->prefix);
            exit(1);
          }
          ret = &(pb_type->ports[iport]);
        }
        break;
      case SPICE_MODEL_PORT_INOUT : 
        if ((INOUT_PORT == pb_type->ports[iport].type)&&(0 == pb_type->ports[iport].is_clock)) {
          if (NULL != ret) {
            vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])More than 1 pb_type(%s) port match spice_model_port(%s)!\n",
                       __FILE__, __LINE__, pb_type->name, spice_model_port->prefix);
            exit(1);
          }
          ret = &(pb_type->ports[iport]);
        }
        break;
      case SPICE_MODEL_PORT_SRAM:
        break;
      default:
        vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid type for spice_model_port(%s)!\n",
                   __FILE__, __LINE__, spice_model_port->prefix);
        exit(1);
      }
    }
  }

  return ret;
}

char* chomp_spice_node_prefix(char* spice_node_prefix) {
  int len = 0;
  char* ret = NULL;

  if (NULL == spice_node_prefix) {
    return NULL;
  }

  len = strlen(spice_node_prefix); /* String length without the last "\0"*/
  ret = (char*)my_malloc(sizeof(char)*(len+2));
  
  /* Don't do anything when input is NULL*/
  if (NULL == spice_node_prefix) {
    my_free(ret);
    return NULL;
  }

  strcpy(ret,spice_node_prefix);
  /* If the path end up with "_" we should remove it*/
  if ('_' == ret[len-1]) {
    ret[len-1] = ret[len];
  }

  return ret;
}

char* format_spice_node_prefix(char* spice_node_prefix) {
  int len = strlen(spice_node_prefix); /* String length without the last "\0"*/
  char* ret = (char*)my_malloc(sizeof(char)*(len+2));
 
  /* Don't do anything when input is NULL*/ 
  if (NULL == spice_node_prefix) {
    my_free(ret);
    return NULL;
  }

  strcpy(ret,spice_node_prefix);
  /* If the path does not end up with "_" we should complete it*/
  if (ret[len-1] != '_') {
    strcat(ret, "_");
  }
  return ret;
}

t_port** find_pb_type_ports_match_spice_model_port_type(t_pb_type* pb_type,
                                                        enum e_spice_model_port_type port_type,
                                                        int* port_num) {
  int iport, cur;
  t_port** ret = NULL;

  /* Check codes*/
  assert(NULL != port_num);
  assert(NULL != pb_type);

  /* Count the number of ports that match*/
  (*port_num) = 0;
  for (iport = 0; iport < pb_type->num_ports; iport++) {
    switch (port_type) {
    case SPICE_MODEL_PORT_INPUT : /* TODO: support is_non_clock_global*/ 
      if ((IN_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        (*port_num)++;
      }
      break;
    case SPICE_MODEL_PORT_OUTPUT: 
      if ((OUT_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        (*port_num)++;
      }
      break;
    case SPICE_MODEL_PORT_INOUT: 
      if ((INOUT_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        (*port_num)++;
      }
      break;
    case SPICE_MODEL_PORT_CLOCK: 
      if ((IN_PORT == pb_type->ports[iport].type)
        &&(1 == pb_type->ports[iport].is_clock)) {
        (*port_num)++;
      }
      break;
    case SPICE_MODEL_PORT_SRAM:
      /* Original VPR don't support this*/
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid type for port!\n",
                 __FILE__, __LINE__);
      exit(1);
    }
  }
  
  /* Initial the return pointers*/
  ret = (t_port**)my_malloc(sizeof(t_port*)*(*port_num));
  memset(ret, 0 , sizeof(t_port*)*(*port_num));
  
  /* Fill the return pointers*/
  cur = 0;

  for (iport = 0; iport < pb_type->num_ports; iport++) {
    switch (port_type) {
    case SPICE_MODEL_PORT_INPUT : /* TODO: support is_non_clock_global*/ 
      if ((IN_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        ret[cur] = &(pb_type->ports[iport]);
        cur++;
      }
      break;
    case SPICE_MODEL_PORT_OUTPUT: 
      if ((OUT_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        ret[cur] = &(pb_type->ports[iport]);
        cur++;
      }
      break;
    case SPICE_MODEL_PORT_INOUT: 
      if ((INOUT_PORT == pb_type->ports[iport].type)
        &&(0 == pb_type->ports[iport].is_clock)) {
        ret[cur] = &(pb_type->ports[iport]);
        cur++;
      }
      break;
    case SPICE_MODEL_PORT_CLOCK: 
      if ((IN_PORT == pb_type->ports[iport].type)
        &&(1 == pb_type->ports[iport].is_clock)) {
        ret[cur] = &(pb_type->ports[iport]);
        cur++;
      }
      break;
    case SPICE_MODEL_PORT_SRAM: 
      /* Original VPR don't support this*/
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid type for port!\n",
                 __FILE__, __LINE__);
      exit(1);
    }
  }
 
  /* Check correctness*/
  assert(cur == (*port_num));
  
  return ret;
}

/* Given the co-ordinators of grid,
 * Find if there is a block mapped into this grid
 */
t_block* search_mapped_block(int x, int y, int z) {
  t_block* ret = NULL;
  int iblk = 0;
  
  /*Valid pointors*/
  assert(NULL != grid);
  assert((0 < x)||(0 == x));
  assert((x < (nx + 1))||(x == (nx + 1)));
  assert((0 < y)||(0 == y));
  assert((x < (ny + 1))||(x == (ny + 1)));

  /* Search all blocks*/
  for (iblk = 0; iblk < num_blocks; iblk++) {
    if ((x == block[iblk].x)&&(y == block[iblk].y)&&(z == block[iblk].z)) {
      /* Matched cordinators*/
      ret = &(block[iblk]);
      /* Check */
      assert(block[iblk].type == grid[x][y].type);
      assert(z < grid[x][y].type->capacity);
      assert(0 < grid[x][y].usage);
    }
  }

  return ret;
}

/* Change the decimal number to binary 
 * and return a array of integer*/
int* my_decimal2binary(int decimal,
                       int* binary_len) {
  int* ret = NULL;
  int i = 0;
  int code = decimal;

  (*binary_len) = 0;
  
  while (0 < code) {
    (*binary_len)++;
    code = code/2;
  }

  i = (*binary_len) - 1;
  while (0 < code) {
    ret[i] = code%2; 
    i--;
    code = code/2;
  }

  return ret;
}

/* Determine the level of multiplexer
 */
int determine_mux_level(int mux_size) {
  int level = 0;
 
  /* Do log2(mux_size), have a basic number*/ 
  level = (int)(log((double)mux_size)/log(2.));
  /* Fix the error, i.e. mux_size=5, level = 2, we have to complete */
  while (mux_size > pow(2.,(double)level)) {
    level++;
  }

  return level;
}

/*Determine the number inputs required at the last level*/
int mux_last_level_input_num(int num_level,
                             int mux_size) {
  int ret = 0;
  
  ret = (int)(pow(2., (double)num_level)) - mux_size;

  if (0 < ret) {
    ret = (int)(2.*(mux_size - pow(2., (double)(num_level-1))));
  } else if (0 > ret) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])num_level(%d) is wrong with mux_size(%d)!\n",
              __FILE__, __LINE__, num_level, mux_size);
    exit(1);
  } else {
    ret = mux_size;
  }

  return ret;
}

int determine_lut_path_id(int lut_size,
                          int* lut_inputs) {
  int path_id = OPEN;
  int i;
  
  for (i = 0; i < lut_size; i++) {
    switch (lut_inputs[i]) {
    case 0:
      path_id += (int)pow(2., (double)(i));
      break;
    case 1:
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s,[LINE%d])Invalid sram_bits[%d]!\n", 
                 __FILE__, __LINE__, i);
      exit(1);
    }
  }

  return path_id;
}

/* Decode the configuration to sram_bits
 * A path_id is in the range of [0..fan_in-1]
 *          sram
 *  input0 -----|
 *              |----- output
 *  input1 -----|
 * Here, we assume (fix) the mux2to1 pass input0 when sram = 1 (vdd), and pass input1 when sram = 0(gnd)
 * To generate the sram bits, we can determine the in each level of MUX,
 * the path id is on the upper path(sram = 1) or the lower path (sram = 0), by path_id > 2**mux_level
 */
int* decode_mux_sram_bits(int fan_in,
                          int mux_level,
                          int path_id) {
  int* ret = (int*)my_malloc(sizeof(int)*mux_level);
  int i = 0;
  int path_differ = 0;
  int temp = 0;
  int num_last_level_input = 0;
  int active_mux_level = 0;
  int active_path_id = 0;
  
  /* Check */
  assert((0 == path_id)||(0 < path_id));
  assert(path_id < fan_in);

  /* Determine last level input */
  num_last_level_input = mux_last_level_input_num(mux_level, fan_in);

  /* Initialize */
  for (i = 0; i < mux_level; i++) {
    ret[i] = 0;
  }
  
  /* When last level input number is less than the 2**mux_level,
   * There are some input at the level: (mux_level-1)
   */
  active_mux_level = mux_level; 
  active_path_id = path_id; 
  if (num_last_level_input < (int)pow(2.,(double)(mux_level))) {
    if (path_id > num_last_level_input) {
      active_path_id = path_id - num_last_level_input/2; 
      active_mux_level = mux_level - 1; 
    }
  } else {
    assert(num_last_level_input == (int)pow(2.,(double)(mux_level)));
  }

  temp = active_path_id;
  for (i = active_mux_level; i > 0; i--) {
    path_differ = (int)pow(2.,(double)(i-1));
    if (temp < path_differ) { 
      ret[i-1] = 1; 
    } else {
      temp = temp - path_differ;
      ret[i-1] = 0;
    }
  }

  /* Check */
  assert(0 == i);
  assert(0 == temp);
  
  return ret;
}

/**
 * Split a string with strtok
 * Store each token in a char array
 * tokens (char**):
 *  tokens[i] (char*) : pointer to a string split by delims
 */

char** my_strtok(char* str, 
                 char* delims, 
                 int* len)
{
  char** ret;
  char* result;
  int cnt=0;
  int* lens;
  char* tmp;

  if (NULL == str)
  {
    printf("Warning: NULL string found in my_strtok!\n");
    return NULL;
  }

  tmp = my_strdup(str);
  result = strtok(tmp,delims);
  /*First scan to determine the size*/
  while(result != NULL)
  {
    cnt++;
    /* strtok split until its buffer is NULL*/
    result = strtok(NULL,delims); 
  }
  //printf("1st scan cnt=%d\n",cnt);
  /* Allocate memory*/ 
  ret = (char**)my_malloc(cnt*sizeof(char*)); 
  lens = (int*)my_malloc(cnt*sizeof(int));
  /*Second to determine the size of each char*/
  cnt = 0;
  memcpy(tmp,str,strlen(str)+1);
  result = strtok(tmp,delims);  
  while(result != NULL)
  {
    lens[cnt] = strlen(result)+1;
    //printf("lens[%d]=%d .",cnt,lens[cnt]);
    cnt++;
    /* strtok split until its buffer is NULL*/
    result = strtok(NULL,delims); 
  }
  //printf("\n");
  /*Third to allocate and copy each char*/
  cnt = 0;
  memcpy(tmp,str,strlen(str)+1);
  result = strtok(tmp,delims);  
  while(result != NULL)
  {
    //printf("results[%d] = %s ",cnt,result);
    ret[cnt] = my_strdup(result);
    cnt++;
    /* strtok split until its buffer is NULL*/
    result = strtok(NULL,delims); 
  }
  //printf("\n");
  
  (*len) = cnt;

  free(tmp);

  return ret;
}

char* convert_side_index_to_string(int side) {
  switch (side) {
  case 0:
    return "top";
  case 1:
    return "right";
  case 2:
    return "bottom";
  case 3:
    return "left";
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid side index. Should be [0,3].\n",
               __FILE__, __LINE__);
    exit(1);
  }
}

void fprint_spice_head(FILE* fp,
                       char* usage) {
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s, LINE[%d]) FileHandle is NULL!\n",__FILE__,__LINE__); 
    exit(1);
  } 
  fprintf(fp,"*****************************\n");
  fprintf(fp,"*     FPGA SPICE Netlist    *\n");
  fprintf(fp,"* Description: %s *\n",usage);
  fprintf(fp,"*    Author: Xifan TANG     *\n");
  fprintf(fp,"* Organization: EPFL/IC/LSI *\n");
  fprintf(fp,"* Date: %s *\n",my_gettime());
  fprintf(fp,"*****************************\n");
  return;
}

void init_spice_net_info(t_spice_net_info* spice_net_info) {
  if (NULL == spice_net_info) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid spice_net_info!\n", __FILE__, __LINE__);
    exit(1);
  }

  spice_net_info->probability = 0.;
  spice_net_info->density = 0.;
  spice_net_info->freq = 0.;
  spice_net_info->init_val = 0;

  spice_net_info->pwl = 0.;
  spice_net_info->pwh = 0.;
  spice_net_info->slew_rise = 0.;
  
  return;
}

/* Find the spice model for Input Pad
 */
t_spice_model* find_inpad_spice_model(int num_spice_model,
                                      t_spice_model* spice_models) {
  t_spice_model* ret = NULL;
  int imodel;
  int num_found = 0;

  for (imodel = 0; imodel < num_spice_model; imodel++) {
    if (SPICE_MODEL_INPAD == spice_models[imodel].type) {
      ret = &(spice_models[imodel]);
      num_found++;
    }
  }

  assert(1 == num_found);

  return ret;
}


char* generate_string_spice_model_type(enum e_spice_model_type spice_model_type) {
  char* ret = NULL;

  switch (spice_model_type) {
  case SPICE_MODEL_WIRE:
    ret = "wire";
    break;
  case SPICE_MODEL_MUX:
    ret = "Multiplexer";
    break;
  case SPICE_MODEL_LUT:
    ret = "Look-Up Table";
    break;
  case SPICE_MODEL_FF:
    ret = "Flip-flop";
    break;
  case SPICE_MODEL_INPAD:
    ret = "Input PAD";
    break;
  case SPICE_MODEL_OUTPAD:
    ret = "Output PAD";
    break;
  case SPICE_MODEL_SRAM:
    ret = "SRAM";
    break;
  case SPICE_MODEL_HARDLOGIC:
    ret = "hard_logic";
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid spice_model_type!\n", __FILE__, __LINE__);
    exit(1);
  }

  return ret;
}

/* Deteremine the side of a io grid */
int determine_io_grid_side(int x,
                           int y) {
  /* TOP side IO of FPGA */
  if ((ny + 1) == y) {
    /* Make sure a valid x, y */
    assert((!(0 > x))&&(x < (nx + 1)));
    return BOTTOM; /* Such I/O has only Bottom side pins */
  } else if ((nx + 1) == x) { /* RIGHT side IO of FPGA */
    /* Make sure a valid x, y */
    assert((!(0 > y))&&(y < (ny + 1)));
    return LEFT; /* Such I/O has only Left side pins */
  } else if (0 == y) { /* BOTTOM side IO of FPGA */
    /* Make sure a valid x, y */
    assert((!(0 > x))&&(x < (nx + 1)));
    return TOP; /* Such I/O has only Top side pins */
  } else if (0 == x) { /* LEFT side IO of FPGA */
    /* Make sure a valid x, y */
    assert((!(0 > y))&&(y < (ny + 1)));
    return RIGHT; /* Such I/O has only Right side pins */
  } else {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])I/O Grid is in the center part of FPGA! Currently unsupported!\n",
               __FILE__, __LINE__);
    exit(1);
  }
}

void find_prev_rr_nodes_with_src(t_rr_node* src_rr_node,
                                 int* num_drive_rr_nodes,
                                 t_rr_node*** drive_rr_nodes,
                                 int** switch_indices) {
  int inode, iedge, next_node;
  int cur_index, switch_index;  

  assert(NULL != src_rr_node);
  assert(NULL != num_drive_rr_nodes);
  assert(NULL != switch_indices); 
  
  (*num_drive_rr_nodes) = 0;
  (*drive_rr_nodes) = NULL;
  (*switch_indices) = NULL;

  switch_index = -1;
  /* Determine num_drive_rr_node */
  for (inode = 0; inode < num_rr_nodes; inode++) {
    for (iedge = 0; iedge < rr_node[inode].num_edges; iedge++) {
      next_node = rr_node[inode].edges[iedge];
      if (src_rr_node == &(rr_node[next_node])) {
        /* Get the spice_model */
        if (-1 == switch_index) {
          switch_index = rr_node[inode].switches[iedge];
        } else { /* Make sure the switches are the same*/
          assert(switch_index == rr_node[inode].switches[iedge]); 
        }
        (*num_drive_rr_nodes)++;
      }
    }
  }
  /* Malloc */
  (*drive_rr_nodes) = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*(*num_drive_rr_nodes));
  (*switch_indices) = (int*)my_malloc(sizeof(int)*(*num_drive_rr_nodes)); 

  /* Find all the rr_nodes that drive current_rr_node*/
  cur_index = 0;
  for (inode = 0; inode < num_rr_nodes; inode++) {
    for (iedge = 0; iedge < rr_node[inode].num_edges; iedge++) {
      next_node = rr_node[inode].edges[iedge];
      if (src_rr_node == &(rr_node[next_node])) {
        /* Update drive_rr_nodes list */
        (*drive_rr_nodes)[cur_index] = &(rr_node[inode]);
        (*switch_indices)[cur_index] = rr_node[inode].switches[iedge];
        cur_index++;
      }
    }
  }
  assert(cur_index == (*num_drive_rr_nodes));

  return;
}


int find_path_id_prev_rr_node(int num_drive_rr_nodes,
                              t_rr_node** drive_rr_nodes,
                              t_rr_node* src_rr_node) {
  int path_id, inode;

  /* Configuration bits for this MUX*/
  path_id = -1;
  for (inode = 0; inode < num_drive_rr_nodes; inode++) {
    if (drive_rr_nodes[inode] == &(rr_node[src_rr_node->prev_node])) {
      path_id = inode;
      break;
    }
  }
  assert((-1 != path_id)&&(path_id < src_rr_node->fan_in));

  return path_id;
}

float pb_pin_density(t_rr_node* pb_rr_graph, 
                     t_pb_graph_pin* pin) {
  float density = 0.;
  int net_num;

  if (NULL == pb_rr_graph) {
    return density;
  }
  net_num = pb_rr_graph[pin->pin_count_in_cluster].net_num;

  if (net_num != OPEN) {
    density = vpack_net[net_num].spice_net_info->density;
  }

  return density;
}

float pb_pin_probability(t_rr_node* pb_rr_graph, 
                         t_pb_graph_pin* pin) {
  float probability = 0.;
  int net_num;

  if (NULL == pb_rr_graph) {
    return probability;
  }
  net_num = pb_rr_graph[pin->pin_count_in_cluster].net_num;

  if (net_num != OPEN) {
    probability = vpack_net[net_num].spice_net_info->probability;
  }

  return probability;
}

int pb_pin_init_value(t_rr_node* pb_rr_graph, 
                      t_pb_graph_pin* pin) {
  float init_val = 0;
  int net_num;

  if (NULL == pb_rr_graph) {
    return init_val;
  }
  net_num = pb_rr_graph[pin->pin_count_in_cluster].net_num;

  if (net_num != OPEN) {
    init_val = vpack_net[net_num].spice_net_info->init_val;
  }

  return init_val;
}

float get_rr_node_net_density(t_rr_node node) {
  /* If we found this net is OPEN, we assume it zero-density */
  if (OPEN == node.net_num) { 
    return 0.;
  } else {
    return clb_net[node.net_num].spice_net_info->density;
  }
}

float get_rr_node_net_probability(t_rr_node node) {
  /* If we found this net is OPEN, we assume it zero-probability */
  if (OPEN == node.net_num) { 
    return 0.;
  } else {
    return clb_net[node.net_num].spice_net_info->probability;
  }
}

int get_rr_node_net_init_value(t_rr_node node) {
  /* If we found this net is OPEN, we assume it zero-probability */
  if (OPEN == node.net_num) { 
    return 0;
  } else {
    return clb_net[node.net_num].spice_net_info->init_val;
  }
}

int find_parent_pb_type_child_index(t_pb_type* parent_pb_type,
                                    int mode_index,
                                    t_pb_type* child_pb_type) {
  int i;

  assert(NULL != parent_pb_type);
  assert(NULL != child_pb_type);
  assert((!(0 > mode_index))&&(mode_index < parent_pb_type->num_modes));

  for (i = 0; i < parent_pb_type->modes[mode_index].num_pb_type_children; i++) {
    if (child_pb_type == &(parent_pb_type->modes[mode_index].pb_type_children[i])) {
      assert(0 == strcmp(child_pb_type->name, parent_pb_type->modes[mode_index].pb_type_children[i].name));
      return i;
    }
  }
  
  return -1;
}

/* Rule in generating a unique name: 
 * name of current pb =  <parent_pb_name_tag>_<cur_pb_graph_node>[index]
 */
void gen_spice_name_tag_pb_rec(t_pb* cur_pb,
                               char* prefix) {
  char* prefix_rec = NULL; 
  int ipb, jpb, mode_index; 

  mode_index = cur_pb->mode;

  /* Free previous name_tag if there is */
  /* my_free(cur_pb->spice_name_tag); */

  /* Generate the name_tag */
  if ((0 < cur_pb->pb_graph_node->pb_type->num_modes)
    &&(NULL == cur_pb->pb_graph_node->pb_type->spice_model_name)) {
    prefix_rec = (char*)my_malloc(sizeof(char)*(strlen(prefix) + 1 + strlen(cur_pb->pb_graph_node->pb_type->name) + 1
                                              + strlen(my_itoa(cur_pb->pb_graph_node->placement_index)) + 7 + strlen(cur_pb->pb_graph_node->pb_type->modes[mode_index].name) + 2 ));
    sprintf(prefix_rec, "%s_%s[%d]_mode[%s]", 
            prefix, cur_pb->pb_graph_node->pb_type->name, cur_pb->pb_graph_node->placement_index, cur_pb->pb_graph_node->pb_type->modes[mode_index].name);
    cur_pb->spice_name_tag = my_strdup(prefix_rec);
  } else {
    assert((0 == cur_pb->pb_graph_node->pb_type->num_modes)
          ||(NULL != cur_pb->pb_graph_node->pb_type->spice_model_name));
    prefix_rec = (char*)my_malloc(sizeof(char)*(strlen(prefix) + 1 + strlen(cur_pb->pb_graph_node->pb_type->name) + 1
                                              + strlen(my_itoa(cur_pb->pb_graph_node->placement_index)) + 2 ));
    sprintf(prefix_rec, "%s_%s[%d]", 
            prefix, cur_pb->pb_graph_node->pb_type->name, cur_pb->pb_graph_node->placement_index);
    cur_pb->spice_name_tag = my_strdup(prefix_rec);
  }

  /* When reach the leaf, we directly return */
  /* Recursive until reach the leaf */
  if ((0 == cur_pb->pb_graph_node->pb_type->num_modes)
     ||(NULL == cur_pb->child_pbs)) {
    return;
  }
  for (ipb = 0; ipb < cur_pb->pb_graph_node->pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb->pb_graph_node->pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Refer to pack/output_clustering.c [LINE 392] */
      //if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) { 
        /* Try to simplify the name tag... to avoid exceeding the length of SPICE name (up to 1024 chars) */
        /* gen_spice_name_tag_pb_rec(&(cur_pb->child_pbs[ipb][jpb]),prefix); */
        gen_spice_name_tag_pb_rec(&(cur_pb->child_pbs[ipb][jpb]),prefix_rec); 
      //}
    }
  }
 
  my_free(prefix_rec);
 
  return;
}


/* Generate a unique name tag for each pb, 
 * to identify it in both SPICE netlist and Power Modeling.
 */
void gen_spice_name_tags_all_pbs() {
  int iblk;
  char* prefix = NULL;

  for (iblk = 0; iblk < num_blocks; iblk++) {
    prefix = (char*)my_malloc(sizeof(char)*(5 + strlen(my_itoa(block[iblk].x)) + 2 + strlen(my_itoa(block[iblk].y)) + 2));
    sprintf(prefix, "grid[%d][%d]", block[iblk].x, block[iblk].y);
    gen_spice_name_tag_pb_rec(block[iblk].pb, prefix);
    my_free(prefix);
  }

  return;
}

/* Make sure the edge has only one input pin and output pin*/
void check_pb_graph_edge(t_pb_graph_edge pb_graph_edge) {
  assert(1 == pb_graph_edge.num_input_pins);
  assert(1 == pb_graph_edge.num_output_pins);

  return;
}

/* Check all the edges for a given pb_graph_pin*/
void check_pb_graph_pin_edges(t_pb_graph_pin pb_graph_pin) {
  int iedge;
 
  for (iedge = 0; iedge < pb_graph_pin.num_input_edges; iedge++) {
    check_pb_graph_edge(*(pb_graph_pin.input_edges[iedge]));
  }
  
  for (iedge = 0; iedge < pb_graph_pin.num_output_edges; iedge++) {
    check_pb_graph_edge(*(pb_graph_pin.output_edges[iedge]));
  }
 
  return;
}

void backup_one_pb_rr_node_pack_prev_node_edge(t_rr_node* pb_rr_node) {

  pb_rr_node->prev_node_in_pack = pb_rr_node->prev_node; 
  pb_rr_node->prev_edge_in_pack = pb_rr_node->prev_edge; 
  pb_rr_node->net_num_in_pack = pb_rr_node->net_num; 
  pb_rr_node->prev_node = OPEN; 
  pb_rr_node->prev_edge = OPEN; 

  return;
}

/* During routing stage, VPR swap logic equivalent pins
 * which potentially changes the packing results (prev_node, prev_edge) in local routing
 * The following functions are to update the local routing results to match them with routing results
 */
void update_one_grid_pack_prev_node_edge(int x, int y) {
  int iblk, blk_id, ipin, iedge, inode;
  int pin_global_rr_node_id, vpack_net_id, class_id;
  t_type_ptr type = NULL;
  t_pb* pb = NULL;
  t_rr_node* local_rr_graph = NULL;

  /* Assert */
  assert((!(x < 0))&&(x < (nx + 1)));  
  assert((!(y < 0))&&(y < (ny + 1)));  

  type = grid[x][y].type;
  /* Bypass IO_TYPE*/
  if ((EMPTY_TYPE == type)||(IO_TYPE == type)) {
    return;
  }   
  for (iblk = 0; iblk < grid[x][y].usage; iblk++) {
    blk_id = grid[x][y].blocks[iblk];
    assert(block[blk_id].x == x);
    assert(block[blk_id].y == y);
    pb = block[blk_id].pb;
    assert(NULL != pb);
    local_rr_graph = pb->rr_graph; 
    /* Foreach local rr_node*/
    for (ipin = 0; ipin < type->num_pins; ipin++) {
      class_id = type->pin_class[ipin];
      if (DRIVER == type->class_inf[class_id].type) {
        pin_global_rr_node_id = get_rr_node_index(x, y, OPIN, ipin, rr_node_indices);
      } else if (RECEIVER == type->class_inf[class_id].type) {
        pin_global_rr_node_id = get_rr_node_index(x, y, IPIN, ipin, rr_node_indices);
      } else {
        continue; /* OPEN PIN */
      }
      /* Get the index of Vpack net from global rr_node net_num (clb_net index)*/
      if (OPEN == rr_node[pin_global_rr_node_id].net_num) {
        continue;
      }
      /* back annotate pb ! */
      rr_node[pin_global_rr_node_id].pb = pb;
      vpack_net_id = clb_to_vpack_net_mapping[rr_node[pin_global_rr_node_id].net_num];
      assert(ipin == local_rr_graph[ipin].pb_graph_pin->pin_count_in_cluster);
      /* Update net_num */
      local_rr_graph[ipin].net_num_in_pack = local_rr_graph[ipin].net_num;
      local_rr_graph[ipin].net_num = vpack_net_id;
      /* TODO: this is not so efficient... */
      for (iedge = 0; iedge < local_rr_graph[ipin].pb_graph_pin->num_output_edges; iedge++) {
        check_pb_graph_edge(*(local_rr_graph[ipin].pb_graph_pin->output_edges[iedge]));
        inode = local_rr_graph[ipin].pb_graph_pin->output_edges[iedge]->output_pins[0]->pin_count_in_cluster;
        /* Update prev_node, prev_edge if needed*/
        if (vpack_net_id == local_rr_graph[inode].net_num) {
          /* Backup prev_node, prev_edge */ 
          backup_one_pb_rr_node_pack_prev_node_edge(&(local_rr_graph[inode]));
          local_rr_graph[inode].prev_node = ipin;
          local_rr_graph[inode].prev_edge = iedge;
        }
      }
    }
  }
 
  return;
}

void update_grid_pbs_post_route_rr_graph() {
  int ix, iy;
  t_type_ptr type = NULL;

  for (ix = 0; ix < (nx + 1); ix++) {
    for (iy = 0; iy < (ny + 1); iy++) {
      type = grid[ix][iy].type;
      if (NULL != type) {
        /* Backup the packing prev_node and prev_edge */
        update_one_grid_pack_prev_node_edge(ix, iy);
      }
    }
  }

  return;
}

int find_pb_mapped_logical_block_rec(t_pb* cur_pb,
                                     t_spice_model* pb_spice_model, 
                                     char* pb_spice_name_tag) {
  int logical_block_index = OPEN;
  int mode_index, ipb, jpb;

  assert(NULL != cur_pb);

  if ((pb_spice_model == cur_pb->pb_graph_node->pb_type->spice_model)
    &&(0 == strcmp(cur_pb->spice_name_tag, pb_spice_name_tag))) {
    /* Special for LUT... They have sub modes!!!*/
    if (SPICE_MODEL_LUT == pb_spice_model->type) {
      mode_index = cur_pb->mode;
      assert(NULL != cur_pb->child_pbs);
      return cur_pb->child_pbs[0][0].logical_block; 
    }
    assert(pb_spice_model == logical_block[cur_pb->logical_block].mapped_spice_model);
    return cur_pb->logical_block;
  }
  
  /* Go recursively ... */
  mode_index = cur_pb->mode;
  if (0 == cur_pb->pb_graph_node->pb_type->num_modes) {
    return logical_block_index;
  }
  for (ipb = 0; ipb < cur_pb->pb_graph_node->pb_type->modes[mode_index].num_pb_type_children; ipb++) {
    for (jpb = 0; jpb < cur_pb->pb_graph_node->pb_type->modes[mode_index].pb_type_children[ipb].num_pb; jpb++) {
      /* Refer to pack/output_clustering.c [LINE 392] */
      if ((NULL != cur_pb->child_pbs[ipb])&&(NULL != cur_pb->child_pbs[ipb][jpb].name)) {
        logical_block_index = 
        find_pb_mapped_logical_block_rec(&(cur_pb->child_pbs[ipb][jpb]), pb_spice_model, pb_spice_name_tag);
        if (OPEN != logical_block_index) {
          return logical_block_index;
        }
      }
    }
  }
  
  return logical_block_index;
}

int find_grid_mapped_logical_block(int x, int y,
                                   t_spice_model* pb_spice_model,
                                   char* pb_spice_name_tag) {
  int logical_block_index = OPEN;
  int iblk;

  /* Find the grid usage */
  if (0 == grid[x][y].usage) { 
    return logical_block_index;
  } else {
    assert(0 < grid[x][y].usage);
    /* search each block */
    for (iblk = 0; iblk < grid[x][y].usage; iblk++) {
      /* Get the pb */
      logical_block_index = find_pb_mapped_logical_block_rec(block[grid[x][y].blocks[iblk]].pb,
                                                             pb_spice_model, pb_spice_name_tag);
      if (OPEN != logical_block_index) {
        return logical_block_index;
      }
    }
  }
  
  return logical_block_index;
}

void stats_pb_graph_node_port_pin_numbers(t_pb_graph_node* cur_pb_graph_node,
                                          int* num_inputs,
                                          int* num_outputs,
                                          int* num_clock_pins) {
  int iport;

  assert(NULL != cur_pb_graph_node);

  (*num_inputs) = 0;
  for (iport = 0; iport < cur_pb_graph_node->num_input_ports; iport++) {
    (*num_inputs) += cur_pb_graph_node->num_input_pins[iport];
  }
  (*num_outputs) = 0;
  for (iport = 0; iport < cur_pb_graph_node->num_output_ports; iport++) {
    (*num_outputs) += cur_pb_graph_node->num_output_pins[iport];
  }
  (*num_clock_pins) = 0;
  for (iport = 0; iport < cur_pb_graph_node->num_clock_ports; iport++) {
    (*num_clock_pins) += cur_pb_graph_node->num_clock_pins[iport];
  }

  return;
}

