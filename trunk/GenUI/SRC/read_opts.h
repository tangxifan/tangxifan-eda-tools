/*Subroutines in read_opts.c*/
int read_options(IN int argc,
                 IN char ** argv);

int process_arg_opt(IN char** argv,
                    INOUT int* iarg,
                    IN char* curarg);

int show_opt_list();

int process_int_arg(IN char* arg,
                    OUT int* val);

int process_float_arg(IN char* arg,
                    OUT float* val);
