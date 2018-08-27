/* Define the basic data structures used for FPGA-SPICE */

/* Key data structure for router: routing resource graph 
 * This data structure store the key parameters that
 * models a routing resource graph used by router.
 * 1. number of routing resource nodes in the graph
 * 2. all the routing resource nodes
 * 3. router information for each routing resource node 
 */

typedef struct fpga_spice_rr_graph t_rr_graph;
struct fpga_spice_rr_graph {
  int num_rr_nodes;
  t_rr_node* rr_nodes;
  t_rr_node_route_inf* rr_node_route_inf;
};
