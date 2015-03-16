/**
 * Filename : util.c
 * Author : Xifan TANG, EPFL
 * Description : Define most useful functions for general purpose
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

void* 
my_malloc(size_t size)
{
  void* ret;

  /*
  if (0 == size)
  {
    printf("Warning: Intend to malloc 0 size memory!\n");
    return NULL;
  }
  */
   
  if ((ret = malloc(size)) == NULL)
  {
    printf("Error: Unable to malloc memeory.Aborting!\n");
    exit(1);
  }
  return (ret);
}

void* 
my_calloc(size_t num,
          size_t size)
{
  void* ret;

  /*
  if (0 == size)
  {
    printf("Warning: Intend to malloc 0 size memory!\n");
    return NULL;
  }
  */
   
  if ((ret = calloc(num,size)) == NULL)
  {
    printf("Error: Unable to calloc memeory.Aborting!\n");
    exit(1);
  }
  return (ret);
}

void* 
my_realloc(void* org, size_t size)
{
  void* ret;
   
  if ((ret = realloc(org,size)) == NULL)
  {
    printf("Error: Unable to realloc memeory.Aborting!\n");
    exit(1);
  }
  return (ret);
}

/*Allocate new char array for a string and Copy it to the new char array*/
char* 
my_strdup(const char* str)
{
  int len;
  char* des;

  len = 1 + strlen(str);
  des = (char*)my_malloc(len*sizeof(char));
  memcpy(des,str,len);

  return des;
}

/*
 * Read lines from a blif file
 * Process comment lines (started with "#")
 * Process contine (End with "\")
 * Return:
 *   0 : NULL
 *   1 : Normal. Line ends with '\n'
 *   2 : Continue. Line ends with '\' 
 */
int   my_freadln(char* buf,
                 int max_size,
                 FILE* fp,
                 char* filename,
                 int* line_cnt)
{
  char* ret;
  int ibuf;
  int actual_buf;
  int continue_buf;
  /*Buffer for continue line*/
  //char* nxtbuf;

  if (feof(fp))
  {
    printf("End of file(%s) reached!\n",filename);
    return -1;
  }

  /*Get a line in buffer*/
  ret = fgets(buf,max_size,fp);
  
  //if (DEBUG)
  //{printf("Get line: %s",buf);}
 
  /* Check line is empty or EOF*/
  if (NULL == ret)
  {
    /*Incremental in line counter*/
    (*line_cnt)++;
    return 0;
  }

  
  /* Replace the '\n' with '\0'
   *Check if there is overflow, the end should be newline
   * Line:       ...  ...\n...     ... ...
   *                      |            |
   * Position:       actual_buf    max_size
   */
  for (ibuf=0; ibuf<max_size; ibuf++)
  {
    if ('\n' == buf[ibuf])
    {
      actual_buf = ibuf;
      buf[ibuf] = '\0';
      break;
    }
    if ('\0' == buf[ibuf])
    {
      printf("Error: Fail to put line into buffer!\n");
      printf("Error: Line(%d) in file(%s) exceeds buffer size(%d)!\n", *line_cnt,filename,max_size);
      exit(1);
    }
  }
  
  //if (DEBUG)
  //{printf("Check Overflow:actual_buf: %d\n",actual_buf);}
  /*
   * Cut off comments, redefine the buf_size in actual_buf 
   * Line:       ...   ...#...     ...\n
   *                      |            |
   * Position:   actual_buf(new)    actual_buf
   */ 
  //actual_buf = max_size;
  for (ibuf=0; ibuf<actual_buf; ibuf++)
  {
    /* if found comment, split with '\0' */
    if ('#' == buf[ibuf])
    {
      actual_buf = ibuf;
      buf[ibuf] = '\0';
      break;
    }
  } 
  if (0 == actual_buf)
  {
    /*Incremental in line counter*/
    (*line_cnt)++;
    return 0;
  }
  
  //if (DEBUG)
  //{printf("Comment Truncated, actual_buf: %d\n",actual_buf);}
  /*
   * Continue lines, redefine actual_buf
   * Line:       ...  ...\...   ...#...     ...\n
   *                     |         |            |
   * Position:     continue_buf actual_buf    max_size
   */
  (*line_cnt)++;
  continue_buf = -1;
  for(ibuf=actual_buf-1; ibuf>-1; ibuf--)
  {
    /*Continue line exist!*/ 
    /*TODO: could be more flexible for continue line,
     *      Truncate all the space between '\' and '\0'
     */
    if (('\\' == buf[ibuf])&&(ibuf == actual_buf-1))
    {
      continue_buf = ibuf;
      if (DEBUG)
      {printf("DEBUG: continue line found in line(%d)!\n", *line_cnt);}
      /* Check if there are chars after continue line token
       * Replace '\' with '\0'
       */
      if ((actual_buf-1) != continue_buf)
      {
        printf("Warning: chars in line(%d) after continue line('\\') ignored!\n",*line_cnt);
      }  
      /*Update position of actual_buf*/
      actual_buf = continue_buf;
      buf[continue_buf] = '\0'; 
      return 2;
    }
  }

  //if (DEBUG)
  //{printf("ContinueLine checked, actual_buf: %d\n",actual_buf);}
  
  return 1;
}

/*
 * Full_freadlns: Full read line from file.
 * Return string which has been truncated or cat.
 */
char*  full_freadlns(char* buf,
                    int max_size,
                    FILE* fp,
                    char* filename,
                    int* line_cnt,
                    int* feof)
{
  char* ret;
  char* tmp_ret;
  int line_len = 0;
  int rdln; 

  *feof = 0; 

  ret = NULL;
  
  while(1)
  {
    memset(buf,0,BUFSIZE*sizeof(char));
    rdln = my_freadln(buf,max_size,fp,filename,line_cnt);
    //printf("my_freadln finish,rdln=%d,buf=%s.\n",rdln,buf);
    
    /*Check EOF first*/
    if (-1 == rdln)
    {
      (*feof) = 1;     
      return NULL;
    }
    //printf("EOF finished\n");

    /*Don't copy the NULL line*/
    if (0 != rdln)
    {
      /*
       * Once find a continue token.
       * Continue finding until a new line
       * Draw back: Always reallocate memory for continued line.
       */
      //printf("Process not NULL line(rdln=%d)!\n",rdln);
      if (NULL == ret)
      {
        //printf("ret is null,copy buf(%s) to ret.\n",buf);
        ret = my_strdup(buf);
        line_len = line_len + strlen(ret) + 1;
        //printf("Copy over. ret is (%s).\n",ret);
      }
      else
      {
        //printf("ret isn't null,cat buf(%s) to ret(%s).\n",buf,ret);
        line_len = line_len + strlen(buf)+1; 
        tmp_ret = (char*)my_realloc(ret,line_len*sizeof(char));
        //printf("Realloc tmp_ret over,");
        //printf("line_len:%d, tmp_ret:%s\n",line_len,tmp_ret);
        tmp_ret = strcat(tmp_ret,buf);
        ret = tmp_ret;
        //printf("Strcat over,ret:%s\n",ret);
      }
      if (1 == rdln)
      {return ret;}  
    } 
  }
    
  return ret; 
}

/*Open file with error checking*/
FILE* my_fopen(char* fname,
               char* flag)
{
  FILE *fp;
  if (NULL == (fp = fopen(fname,flag)))
  {
    printf("Error: Fail to open %s!\n", fname);
    exit(1);
  }
  return fp;
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

/* Free a char** array*/
void free_tokens(char** tokens,
                 int tokens_len)
{
  int itok;
  
  for (itok=0; itok<tokens_len; itok++)
  {
    free(tokens[itok]);
  }
  
  free(tokens);

}

/**
 * Read the integer in option list 
 * Do simple check and convert it from char to 
 * integer
 */
int my_atoi(char* arg)
{
  int ret;

  /*Check if the pointor of arg is NULL */
  if (NULL == arg)
  {
    printf("Error: Input is NULL when processing integer!\n");
    exit(1);
  } 

  ret = atoi(arg);

  return ret;
}

/**
 * Read the float in option list 
 * Do simple check and convert it from char to 
 * float
 */
float my_atof(char* arg)
{
  float ret;

  /*Check if the pointor of arg is NULL */
  if (NULL == arg)
  {
    printf("Error: Input is NULL when processing float!\n");
    exit(1);
  }

  ret = atof(arg);

  return ret;
}

/**
 * Create, insert, delete, cat 
 * General purpose linked list
 */

/**
 * Create fixed length of Linked list
 * Node struct must have a pointor named after "next"!
 * The head of this linked list is a scout. Formal start is the second element. 
 */
t_llist* create_llist(int len,
                      size_t size_node)
{
  t_llist* head;
  t_llist* tmp_head;
  int ind;

  /* Create a scout, this is a blank node*/
  head = (t_llist*)my_malloc(size_node);
  head->next = NULL;
  
  for (ind=0; ind<len; ind++)
  {
    /* Create a node ahead of the current*/
    tmp_head = (t_llist*)my_malloc(size_node); 
    tmp_head->next = head;
    head = tmp_head;
  }
  
  return head; 
}

/**
 * Insert a node inside the linked list
 * Cur is pointer which a new node will be insertedafter.
 */
t_llist* insert_llist_node(t_llist* cur,
                           size_t size_node)
{
  t_llist* cur_next; 
  /* Store the current next*/  
  cur_next = cur->next;
  /* Allocate new node*/
  cur->next = (t_llist*)my_malloc(size_node);
  /* Configure the new node*/
  cur->next->next = cur_next;

  return cur_next;
}

/**
 * Romove a node from linked list
 * cur is the node whose next node is to be removed
 */
void remove_llist_node(t_llist* cur)
{ 
  t_llist* rm_node = cur->next;
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
t_llist* cat_llists(t_llist* head1,
                    t_llist* head2)
{
  t_llist* tmp = head1;

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

int free_int_ptr_ary(int len_ary,
                     int** ary)
{
  int iary;

  if (NULL == ary) {
    return 0;
  } 

  for (iary=0; iary<len_ary; iary++) {
    free(ary[iary]);
  }

  free(ary);

  return 1;
}
