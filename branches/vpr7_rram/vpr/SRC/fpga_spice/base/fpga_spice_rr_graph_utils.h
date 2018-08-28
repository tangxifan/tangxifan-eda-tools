
void alloc_and_load_rr_graph_rr_node(INOUTP t_rr_graph* local_rr_graph,
                                     IN int local_num_rr_nodes);

void alloc_and_load_rr_graph_switch_inf(INOUTP t_rr_graph* local_rr_graph,
                                        IN int num_switch_inf,
                                        INP t_switch_inf* switch_inf);

void alloc_and_load_rr_graph_route_structs(t_rr_graph* local_rr_graph);

void free_rr_graph_rr_nodes(t_rr_graph* local_rr_graph);

void free_rr_graph_switch_inf(INOUTP t_rr_graph* local_rr_graph);

void free_rr_graph_route_structs(t_rr_graph* local_rr_graph);

void free_rr_graph(t_rr_graph* local_rr_graph);

void free_rr_graph_heap_data(t_rr_graph* local_rr_graph,

t_heap * get_rr_graph_heap_head(t_rr_graph* local_rr_graph);

t_linked_f_pointer* alloc_rr_graph_linked_f_pointer(t_rr_graph* local_rr_graph);

void add_to_rr_graph_mod_list(t_rr_graph* local_rr_graph,
                              float *fptr);

t_trace* alloc_rr_graph_trace_data(t_rr_graph* local_rr_graph);

void empty_rr_graph_heap(t_rr_graph* local_rr_graph);

void reset_rr_graph_rr_node_route_structs(t_rr_graph* local_rr_graph);

t_trace* update_rr_graph_traceback(t_rr_graph* local_rr_graph,
                                   t_heap *hptr, int inet);

void reset_rr_graph_path_costs(t_rr_graph* local_rr_graph);

float get_rr_graph_rr_cong_cost(t_rr_graph* local_rr_graph,
                                int rr_node_index);

t_heap * alloc_rr_graph_heap_data(t_rr_graph* local_rr_graph);

void add_heap_node_to_rr_graph_heap(t_rr_graph* local_rr_graph,
                                    t_heap *hptr);

void add_node_to_rr_graph_heap(t_rr_graph* local_rr_graph,
                               int inode, float cost, int prev_node, int prev_edge,
                               float backward_path_cost, float R_upstream);

void mark_rr_graph_ends(t_rr_graph* local_rr_graph, 
                        int inet);

void invalidate_rr_graph_heap_entries(t_rr_graph* local_rr_graph, 
                                      int sink_node, int ipin_node);

float get_rr_graph_rr_node_pack_intrinsic_cost(t_rr_graph* local_rr_graph,
                                               int inode);
