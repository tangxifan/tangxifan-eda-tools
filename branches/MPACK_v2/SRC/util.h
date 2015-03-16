/**
 * Filename : util.h
 * Author : Xifan TANG, EPFL
 * Description : Define most useful types for general purpose
 */

/*DEBUG MODE*/
#define DEBUG 0

#define IN
#define OUT
#define INOUT
 
#define BUFSIZE 4096

#define abs(x) (((x)>0)?(x):-(x))
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)>(b)?(b):(a))

/*Boolean enum, C does not support*/
typedef enum e_bool bool;
enum e_bool
{TRUE,FALSE};

/*General Purpose Linked List*/
typedef struct s_llist t_llist;
struct s_llist
{
  void* dptr;
  t_llist* next;
};

/*****Subroutines*****/
void* my_malloc(size_t size);

void* my_calloc(size_t num, size_t size);

void* my_realloc(void* org, size_t size);

char* my_strdup(const char* str);

int my_freadln(char* buf, int max_size,
               FILE* fp, char* filename,
               int* line_cnt);

char* full_freadlns(char* buf, int max_size,
                    FILE* fp, char* filename,
                    int* line_cnt, int* feof);

FILE* my_fopen(char* fname, char* flag);

char** my_strtok(char* str,
                 char* delims,
                 int* len);

void free_tokens(char** tokens,
                 int tokens_len);

float my_atof(IN char* arg);

int my_atoi(IN char* arg);

t_llist* create_llist(int len,
                      size_t size_node);

t_llist* insert_llist_node(t_llist* cur,
                           size_t size_node);

void remove_llist_node(t_llist* cur);

t_llist* cat_llist(t_llist* head1,
                   t_llist* head2);

int free_int_ptr_ary(int len_ary,
                     int** ary);

