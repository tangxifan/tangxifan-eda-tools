// MCluster Packer (MPack)
// Creator : Pierre - Emmanuel Gaillardon
// Accessibility and debug tools library

#ifndef ACCESS_H_
#define ACCESS_H_

// MATRIX ACCESSIBILITY FUNCTIONS
int* matrix_access(int ** matrix, int depth_indix, int width_indix);
int matrix_add_line_column(int *** matrix, int msize, int indix);
int matrix_remove_line_column(int *** matrix, int msize, int idx);
int ** delete_full_connectivity_matrix(int ** fconm, int size);
int ** alloc_and_fill_full_connectivity_matrix(int ** fadjm, int size);

// TABLE ACCESSIBILITY FUNCTIONS
void table_add_case(int **tab, int size, int idx);
void table_add_pchar(char ***tab, int size, int idx);
int table_remove_case(int **tab, int size, int idx);
int table_remove_pchar(char ***tab, int size, int idx);
// Clean and realloc unused case in a table
int clean_table(int **tab, int size);
int max_in_table(int * tab, int size);


// DEBUG FUNCTIONS

void print_matrix(int ** matrix, int nb_line, int nb_column);
void print_table(int * table, int size);
void print_char_table(char ** table, int size);

#endif
