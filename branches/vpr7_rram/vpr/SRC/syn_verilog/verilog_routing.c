/***********************************/
/*      SPICE Modeling for VPR     */
/*       Xifan TANG, EPFL/LSI      */
/***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>
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
#include "spice_utils.h"

/* Include Verilog support headers*/
#include "verilog_global.h"
#include "verilog_utils.h"
#include "verilog_lut.h"
#include "verilog_primitives.h"
#include "verilog_routing.h"


void dump_verilog_routing_chan_subckt(FILE* fp,
                                int x,
                                int y,
                                t_rr_type chan_type, 
                                int chan_width,
                                t_ivec*** LL_rr_node_indices,
                                int num_segment,
                                t_segment_inf* segments) {
  int itrack, inode, iseg, cost_index;
  char* chan_prefix = NULL;
  t_rr_node** chan_rr_nodes = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*chan_width);

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 
  assert((CHANX == chan_type)||(CHANY == chan_type));

  /* Initial chan_prefix*/
  switch (chan_type) {
  case CHANX:
    chan_prefix = "chanx";
    fprintf(fp, "//----- Subckt for Channel X [%d][%d] -----\n", x, y);
    break;
  case CHANY:
    chan_prefix = "chany";
    fprintf(fp, "//----- Subckt for Channel Y [%d][%d] -----\n", x, y);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid Channel type! Should be CHANX or CHANY.\n",
               __FILE__, __LINE__);
    exit(1);
  }
  
  /* Collect rr_nodes for Tracks for chanx[ix][iy] */
  for (itrack = 0; itrack < chan_width; itrack++) {
    inode = get_rr_node_index(x, y, chan_type, itrack, LL_rr_node_indices);
    chan_rr_nodes[itrack] = &(rr_node[inode]);
  }

  /* Chan subckt definition */
  fprintf(fp, "module %s_%d__%d_ ( \n", chan_prefix, x, y);
  /* Inputs and outputs,
   * Rules for CHANX:
   * print left-hand ports(in) first, then right-hand ports(out)
   * Rules for CHANX:
   * print bottom ports(in) first, then top ports(out)
   */
  for (itrack = 0; itrack < chan_width; itrack++) {
    switch (chan_rr_nodes[itrack]->direction) {
    case INC_DIRECTION:
      fprintf(fp, "  input in%d, // track %d input \n", itrack, itrack);
      break;
    case DEC_DIRECTION:
      fprintf(fp, "  output out%d, // track %d output \n", itrack, itrack);
      break;
    case BI_DIRECTION:
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of rr_node chany[%d][%d]_in/out[%d]!\n",
                 __FILE__, __LINE__, x, y + 1, itrack);
      exit(1);
    }
  }
  for (itrack = 0; itrack < chan_width; itrack++) {
    switch (chan_rr_nodes[itrack]->direction) {
    case INC_DIRECTION:
      fprintf(fp, "  output out%d, // track %d output\n", itrack, itrack);
      break;
    case DEC_DIRECTION:
      fprintf(fp, "  input in%d, // track %d input \n", itrack, itrack);
      break;
    case BI_DIRECTION:
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of rr_node chany[%d][%d]_in/out[%d]!\n",
                 __FILE__, __LINE__, x, y + 1, itrack);
      exit(1);
    }
  }
  /* Middle point output for connection box inputs */
  for (itrack = 0; itrack < chan_width; itrack++) {
    fprintf(fp, "  output mid_out%d", itrack);
    if (itrack < (chan_width - 1)) {
      fprintf(fp, ",");
    } else {
    }
    fprintf(fp, " // Middle output %d to logic blocks \n", itrack);
  }
  fprintf(fp, "  );\n");

  /* Print segments models*/
  for (itrack = 0; itrack < chan_width; itrack++) {
    cost_index = chan_rr_nodes[itrack]->cost_index;
    iseg = rr_indexed_data[cost_index].seg_index; 
    /* Check */
    assert((!(iseg < 0))&&(iseg < num_segment));
    /* short connecting inputs and outputs: 
     * length of metal wire and parasitics are handled by semi-custom flow
     */
    fprintf(fp, "assign out%d = in%d; \n", itrack, itrack); 
    fprintf(fp, "assign mid_out%d = in%d; \n", itrack, itrack);
  }

  fprintf(fp, "endmodule\n");

  /* Free */
  my_free(chan_rr_nodes);
  
  return;
}

t_rr_node** verilog_get_grid_side_pin_rr_nodes(int* num_pin_rr_nodes,
                                       t_rr_type pin_type,
                                       int x,
                                       int y,
                                       int side,
                                       t_ivec*** LL_rr_node_indices) {
  int height, ipin, class_id, inode;
  t_type_ptr type = NULL;
  t_rr_node** ret = NULL;
  enum e_pin_type pin_class_type;
  int cur;
  
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 
  type = grid[x][y].type;
  assert(NULL != type);
  /* Assign the type of PIN*/ 
  switch (pin_type) {
  case IPIN:
  /* case SINK: */
    pin_class_type = RECEIVER; /* This is the end of a route path*/ 
    break;
  /*case SOURCE:*/
  case OPIN:
    pin_class_type = DRIVER; /* This is the start of a route path */ 
    break;
  /* SINK and SOURCE are hypothesis nodes */
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid pin_type!\n", __FILE__, __LINE__);
    exit(1); 
  }

  /* Output the pins on the side*/
  (*num_pin_rr_nodes) = 0;
  height = grid[x][y].offset;
  for (ipin = 0; ipin < type->num_pins; ipin++) {
    class_id = type->pin_class[ipin];
    if ((1 == type->pinloc[height][side][ipin])&&(pin_class_type == type->class_inf[class_id].type)) {
      (*num_pin_rr_nodes)++;
    }
  } 
  /* Malloc */
  ret = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*(*num_pin_rr_nodes)); 

  /* Fill the return array*/
  cur = 0;
  height = grid[x][y].offset;
  for (ipin = 0; ipin < type->num_pins; ipin++) {
    class_id = type->pin_class[ipin];
    if ((1 == type->pinloc[height][side][ipin])&&(pin_class_type == type->class_inf[class_id].type)) {
      inode = get_rr_node_index(x, y, pin_type, ipin, LL_rr_node_indices);
      ret[cur] = &(rr_node[inode]); 
      cur++;
    }
  } 
  assert(cur == (*num_pin_rr_nodes));
  
  return ret;
}

void dump_verilog_grid_side_in_with_given_index(FILE* fp,
                                          int pin_index,
                                          int side,
                                          int x,
                                          int y) {
  int height, class_id;
  t_type_ptr type = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 
  type = grid[x][y].type;
  assert(NULL != type);

  assert((!(0 > pin_index))&&(pin_index < type->num_pins));
  assert((!(0 > side))&&(!(side > 3)));

  /* Output the pins on the side*/ 
  height = grid[x][y].offset;
  class_id = type->pin_class[pin_index];
  if ((1 == type->pinloc[height][side][pin_index])) {
    fprintf(fp, "grid_%d__%d__pin_%d__%d__%d_ ", x, y, height, side, pin_index);
  } else {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Fail to print a grid pin (x=%d, y=%d, height=%d, side=%d, index=%d)",
              __FILE__, __LINE__, x, y, height, side, pin_index);
    exit(1);
  } 

  return;
}

void dump_verilog_grid_side_pins(FILE* fp,
                           t_rr_type pin_type,
                           int x,
                           int y,
                           int side,
                           boolean dump_port_type) {
  int height, ipin, class_id;
  t_type_ptr type = NULL;
  enum e_pin_type pin_class_type;
  char* verilog_port_type = NULL;
  
  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 
  type = grid[x][y].type;
  assert(NULL != type);
 
  /* Assign the type of PIN*/ 
  switch (pin_type) {
  case IPIN:
  /* case SINK: */
    pin_class_type = RECEIVER; /* This is the end of a route path*/ 
    verilog_port_type = "output";
    break;
  /* case SOURCE: */
  case OPIN:
    pin_class_type = DRIVER; /* This is the start of a route path */ 
    verilog_port_type = "input";
    break;
  /* SINK and SOURCE are hypothesis nodes */
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid pin_type!\n", __FILE__, __LINE__);
    exit(1); 
  }

  /* Output the pins on the side*/ 
  height = grid[x][y].offset;
  for (ipin = 0; ipin < type->num_pins; ipin++) {
    class_id = type->pin_class[ipin];
    if ((1 == type->pinloc[height][side][ipin])&&(pin_class_type == type->class_inf[class_id].type)) {
      if (TRUE == dump_port_type) {
        fprintf(fp, "%s ", verilog_port_type);
      }
      fprintf(fp, " grid_%d__%d__pin_%d__%d__%d_", x, y, height, side, ipin);
      if (TRUE == dump_port_type) {
        fprintf(fp, ",\n");
      } else {
        fprintf(fp, ",\n");
      }
    }
  } 
  
  return;
}

/* Determine the channel coordinates in switch box subckt */
void verilog_determine_src_chan_coordinate_switch_box(t_rr_node* src_rr_node,
                                              t_rr_node* des_rr_node,
                                              int side,
                                              int switch_box_x,
                                              int switch_box_y,
                                              int* src_chan_x,
                                              int* src_chan_y,
                                              char** src_chan_port_name) {
  /* Check */ 
  assert((!(0 > side))&&(side < 4));
  assert((CHANX == src_rr_node->type)||(CHANY == src_rr_node->type));
  assert((CHANX == des_rr_node->type)||(CHANY == des_rr_node->type));
  assert((!(0 > switch_box_x))&&(!(switch_box_x > (nx + 1)))); 
  assert((!(0 > switch_box_y))&&(!(switch_box_y > (ny + 1)))); 
 
  /* Initialize*/
  (*src_chan_x) = 0;
  (*src_chan_y) = 0;
  (*src_chan_port_name) = NULL;

  switch (side) {
  case 0: /*TOP*/
    /* The destination rr_node only have one condition!!! */
    assert((INC_DIRECTION == des_rr_node->direction)&&(CHANY == des_rr_node->type));
    /* Following cases:
     *               |
     *             / | \
     */
    if ((INC_DIRECTION == src_rr_node->direction)&&(CHANY == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x;
      (*src_chan_y) = switch_box_y;
      (*src_chan_port_name) = "in";
    } else if ((INC_DIRECTION == src_rr_node->direction)&&(CHANX == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x;
      (*src_chan_y) = switch_box_y;
      (*src_chan_port_name) = "in";
    } else if ((DEC_DIRECTION == src_rr_node->direction)&&(CHANX == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x + 1;
      (*src_chan_y) = switch_box_y;
      (*src_chan_port_name) = "in";
    } else {
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid source channel!\n", __FILE__, __LINE__);
      exit(1);
    }
    break; 
  case 1: /*RIGHT*/
    /* The destination rr_node only have one condition!!! */
    assert((INC_DIRECTION == des_rr_node->direction)&&(CHANX == des_rr_node->type));
    /* Following cases:
     *          \               
     *       ---  ----  
     *          /
     */
    if ((DEC_DIRECTION == src_rr_node->direction)&&(CHANY == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x;
      (*src_chan_y) = switch_box_y + 1;
      (*src_chan_port_name) = "in";
    } else if ((INC_DIRECTION == src_rr_node->direction)&&(CHANX == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x;
      (*src_chan_y) = switch_box_y;
      (*src_chan_port_name) = "in";
    } else if ((INC_DIRECTION == src_rr_node->direction)&&(CHANY == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x;
      (*src_chan_y) = switch_box_y;
      (*src_chan_port_name) = "in";
    } else {
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid source channel!\n", __FILE__, __LINE__);
      exit(1);
    }
    break; 
  case 2: /*BOTTOM*/
    /* The destination rr_node only have one condition!!! */
    assert((DEC_DIRECTION == des_rr_node->direction)&&(CHANY == des_rr_node->type));
    /* Following cases:
     *          |               
     *        \   /  
     *          |
     */
    if ((DEC_DIRECTION == src_rr_node->direction)&&(CHANY == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x;
      (*src_chan_y) = switch_box_y + 1;
      (*src_chan_port_name) = "in";
    } else if ((INC_DIRECTION == src_rr_node->direction)&&(CHANX == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x;
      (*src_chan_y) = switch_box_y;
      (*src_chan_port_name) = "in";
    } else if ((DEC_DIRECTION == src_rr_node->direction)&&(CHANX == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x + 1;
      (*src_chan_y) = switch_box_y;
      (*src_chan_port_name) = "in";
    } else {
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid source channel!\n", __FILE__, __LINE__);
      exit(1);
    }
    break; 
  case 3: /*LEFT*/
    /* The destination rr_node only have one condition!!! */
    assert((DEC_DIRECTION == des_rr_node->direction)&&(CHANX == des_rr_node->type));
    /* Following cases:
     *           /               
     *       ---  ----  
     *           \
     */
    if ((DEC_DIRECTION == src_rr_node->direction)&&(CHANX == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x + 1;
      (*src_chan_y) = switch_box_y;
      (*src_chan_port_name) = "in";
    } else if ((INC_DIRECTION == src_rr_node->direction)&&(CHANY == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x;
      (*src_chan_y) = switch_box_y;
      (*src_chan_port_name) = "in";
    } else if ((DEC_DIRECTION == src_rr_node->direction)&&(CHANY == src_rr_node->type)) {
      (*src_chan_x) = switch_box_x;
      (*src_chan_y) = switch_box_y + 1;
      (*src_chan_port_name) = "in";
    } else {
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid source channel!\n", __FILE__, __LINE__);
      exit(1);
    }
    break; 
  default: 
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid side!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Make sure the source rr_node (channel) is in the range*/
  assert((!((*src_chan_x) < src_rr_node->xlow))&&(!((*src_chan_x) > src_rr_node->xhigh)));
  if (!((!((*src_chan_y) < src_rr_node->ylow))&&(!((*src_chan_y) > src_rr_node->yhigh)))) {
  assert((!((*src_chan_y) < src_rr_node->ylow))&&(!((*src_chan_y) > src_rr_node->yhigh)));
  }

  return; 
}

void dump_verilog_switch_box_chan_port(FILE* fp,
                                 int switch_box_x, 
                                 int switch_box_y, 
                                 int chan_side,
                                 t_rr_node* cur_rr_node) {
  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  switch (chan_side) {
  case 0: /*TOP*/
    /* The destination rr_node only have one condition!!! */
    assert((INC_DIRECTION == cur_rr_node->direction)&&(CHANY == cur_rr_node->type));
    fprintf(fp, "chany_%d__%d__out_%d_ ", switch_box_x, switch_box_y + 1, cur_rr_node->ptc_num);
    break;
  case 1: /*RIGHT*/
    /* The destination rr_node only have one condition!!! */
    assert((INC_DIRECTION == cur_rr_node->direction)&&(CHANX == cur_rr_node->type));
    fprintf(fp, "chanx_%d__%d__out_%d_ ", switch_box_x + 1, switch_box_y, cur_rr_node->ptc_num);
    break;
  case 2: /*BOTTOM*/
    /* The destination rr_node only have one condition!!! */
    assert((DEC_DIRECTION == cur_rr_node->direction)&&(CHANY == cur_rr_node->type));
    fprintf(fp, "chany_%d__%d__out_%d_ ", switch_box_x, switch_box_y, cur_rr_node->ptc_num);
    break;
  case 3: /*LEFT*/
    /* The destination rr_node only have one condition!!! */
    assert((DEC_DIRECTION == cur_rr_node->direction)&&(CHANX == cur_rr_node->type));
    fprintf(fp, "chanx_%d__%d__out_%d_ ", switch_box_x, switch_box_y, cur_rr_node->ptc_num);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid side!\n", __FILE__, __LINE__);
    exit(1);
  }
  return;
}

/* Print a short interconneciton in switch box
 * There are two cases should be noticed.
 * 1. The actual fan-in of cur_rr_node is 0. In this case,
      the cur_rr_node need to be short connected to itself which is on the opposite side of this switch
 * 2. The actual fan-in of cur_rr_node is 0. In this case,
 *    The cur_rr_node need to connected to the drive_rr_node
 */
void dump_verilog_switch_box_short_interc(FILE* fp, 
                                    int switch_box_x, 
                                    int switch_box_y, 
                                    int chan_side,
                                    t_rr_node* cur_rr_node,
                                    int actual_fan_in,
                                    t_rr_node* drive_rr_node) {
  int side; 
  int track_index;
  int grid_x, grid_y, pin_index, height, class_id, pin_written_times;
  t_type_ptr type = NULL;
  int src_chan_x, src_chan_y;
  char* src_chan_port_name = NULL;
  char* chan_name = NULL;
  char* des_chan_port_name = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert((!(0 > switch_box_x))&&(!(switch_box_x > (nx + 1)))); 
  assert((!(0 > switch_box_y))&&(!(switch_box_y > (ny + 1)))); 
  assert((0 == actual_fan_in)||(1 == actual_fan_in));

  switch(cur_rr_node->type) {
  case CHANX:
    chan_name = "chanx";
    break;
  case CHANY:
    chan_name = "chany";
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid cur_rr_node_type!\n", __FILE__, __LINE__);
    exit(1);
  }

  switch (cur_rr_node->direction) {
  case INC_DIRECTION:
    if ((TOP == chan_side)||(RIGHT == chan_side)) {
      des_chan_port_name = "out"; 
    } else if ((BOTTOM == chan_side)||(LEFT == chan_side)) {
      des_chan_port_name = "in"; 
    } else {
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid chan_side(%d)!\n",
                  __FILE__, __LINE__, chan_side);
      exit(1);
    }
    break;
  case DEC_DIRECTION:
    if ((TOP == chan_side)||(RIGHT == chan_side)) {
      des_chan_port_name = "in"; 
    } else if ((BOTTOM == chan_side)||(LEFT == chan_side)) {
      des_chan_port_name = "out"; 
    } else {
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid chan_side(%d)!\n",
                  __FILE__, __LINE__, chan_side);
      exit(1);
    }
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid cur_rr_node directionality!\n", __FILE__, __LINE__);
    exit(1);
  }

  fprintf(fp, "//----- Short connection %s[%d][%d]_%s[%d] -----\n", 
          chan_name, switch_box_x, switch_box_y, des_chan_port_name, cur_rr_node->ptc_num);
  fprintf(fp, "assign "); 

  /* Output port */
  dump_verilog_switch_box_chan_port(fp, switch_box_x, switch_box_y, chan_side, cur_rr_node);
  fprintf(fp, "= "); 


  /* Check the driver*/
  if (0 == actual_fan_in) {
    assert(drive_rr_node == cur_rr_node);
  } else {
    /* drive_rr_node = &(rr_node[cur_rr_node->prev_node]); */
    assert(1 == rr_node_drive_switch_box(drive_rr_node, cur_rr_node, switch_box_x, switch_box_y, chan_side));
  }
  switch (drive_rr_node->type) {
  /* case SOURCE: */
  case OPIN:
    /* Indicate a CLB Outpin*/
    /* Get grid information*/
    grid_x = drive_rr_node->xlow; 
    grid_y = drive_rr_node->ylow; /*Plus the offset*/
    assert((switch_box_x == grid_x)||((switch_box_x+1) == grid_x));
    assert((switch_box_y == grid_y)||((switch_box_y+1) == grid_y));
    height = grid[grid_x][grid_y].offset;
    grid_y = grid_y + height; /* May Cause Prob.*/
    type = grid[grid_x][grid_y].type;
    assert(NULL != type);
    /* Get pin information*/
    pin_index = drive_rr_node->ptc_num;
    class_id = type->pin_class[pin_index];
    assert(DRIVER == type->class_inf[class_id].type);
    pin_written_times = 0;
    side = determine_io_grid_side(grid_x, grid_y);
    if (1 == type->pinloc[height][side][pin_index]) {
      fprintf(fp, "grid_%d__%d__pin_%d__%d__%d_ ", 
              grid_x, grid_y, height, side, pin_index);
      pin_written_times++;
    }
    /* Make sure this pin is printed only once!!! (TODO: make sure for IO PAD, this remains ok)*/
    assert(1 == pin_written_times);
    break;
  case CHANX:
    verilog_determine_src_chan_coordinate_switch_box(drive_rr_node, cur_rr_node, chan_side,
                                             switch_box_x, switch_box_y, &src_chan_x, &src_chan_y, &src_chan_port_name);
    /* For channels, ptc_num is the track_index*/
    track_index = drive_rr_node->ptc_num; 
    fprintf(fp, "chanx_%d__%d__%s_%d_ ", src_chan_x, src_chan_y, src_chan_port_name, track_index);
    //my_free(src_chan_port_name);
    break;
  case CHANY:
    verilog_determine_src_chan_coordinate_switch_box(drive_rr_node, cur_rr_node, chan_side,
                                             switch_box_x, switch_box_y, &src_chan_x, &src_chan_y, &src_chan_port_name);
    /* For channels, ptc_num is the track_index*/
    track_index = drive_rr_node->ptc_num; 
    fprintf(fp, "chany_%d__%d__%s_%d_ ", src_chan_x, src_chan_y, src_chan_port_name, track_index);
    //my_free(src_chan_port_name);
    break;
  /* SOURCE is invalid as well */
  default: /* IPIN, SINK are invalid*/
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid rr_node type! Should be [OPIN|CHANX|CHANY].\n",
               __FILE__, __LINE__);
    exit(1);
  }

  /* END */
  fprintf(fp, ";\n");

  return;
}

/* Print the SPICE netlist of multiplexer that drive this rr_node */
void dump_verilog_switch_box_mux(FILE* fp, 
                           int switch_box_x, 
                           int switch_box_y, 
                           int chan_side,
                           t_rr_node* cur_rr_node,
                           int mux_size,
                           t_rr_node** drive_rr_nodes,
                           int switch_index) {
  int inode, side;
  int track_index;
  int grid_x, grid_y, pin_index, height, class_id, pin_written_times;
  t_type_ptr type = NULL;
  t_spice_model* verilog_model = NULL;
  int src_chan_x, src_chan_y;
  char* src_chan_port_name = NULL;
  int mux_level, path_id, cur_num_sram, ilevel;
  int num_mux_sram_bits = 0;
  int* mux_sram_bits = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert((!(0 > switch_box_x))&&(!(switch_box_x > (nx + 1)))); 
  assert((!(0 > switch_box_y))&&(!(switch_box_y > (ny + 1)))); 

  /* Check current rr_node is CHANX or CHANY*/
  assert((CHANX == cur_rr_node->type)||(CHANY == cur_rr_node->type));
  
  /* Allocate drive_rr_nodes according to the fan-in*/
  assert((2 == mux_size)||(2 < mux_size));

  /* Get verilog model*/
  verilog_model = switch_inf[switch_index].spice_model;
  /* Now it is the time print the SPICE netlist of MUX*/
  fprintf(fp, "%s_size%d %s_size%d_%d_ (", 
          verilog_model->prefix, mux_size,
          verilog_model->prefix, mux_size, verilog_model->cnt);
  verilog_model->cnt++;
  /* Input ports*/
  for (inode = 0; inode < mux_size; inode++) {
    switch (drive_rr_nodes[inode]->type) {
    /* case SOURCE: */
    case OPIN:
      /* Indicate a CLB Outpin*/
      /* Get grid information*/
      grid_x = drive_rr_nodes[inode]->xlow; 
      grid_y = drive_rr_nodes[inode]->ylow; /*Plus the offset*/
      assert((switch_box_x == grid_x)||((switch_box_x+1) == grid_x));
      assert((switch_box_y == grid_y)||((switch_box_y+1) == grid_y));
      height = grid[grid_x][grid_y].offset;
      grid_y = grid_y + height; /* May Cause Prob.*/
      type = grid[grid_x][grid_y].type;
      assert(NULL != type);
      /* Get pin information*/
      pin_index = drive_rr_nodes[inode]->ptc_num;
      class_id = type->pin_class[pin_index];
      assert(DRIVER == type->class_inf[class_id].type);
      pin_written_times = 0;
      /* See the channel type and then determine the side*/
      switch (cur_rr_node->type) {
      case CHANX:
        if (switch_box_y == grid_y) {
          side = TOP;
        } else if ((switch_box_y+1) == grid_y) {
          side = BOTTOM;
        } else {
          vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid grid_y!\n", __FILE__, __LINE__);
          exit(1);
        }
        break;
      case CHANY:
        if (switch_box_x == grid_x) {
          side = RIGHT;
        } else if ((switch_box_x+1) == grid_x) {
          side = LEFT;
        } else {
          vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid grid_y!\n", __FILE__, __LINE__);
          exit(1);
        }
        break;
      default:
        vpr_printf(TIO_MESSAGE_ERROR,"(File:%s, [LINE%d])Invalid cur_rr_node type!\n", __FILE__, __LINE__);
        exit(1);
      }
      /* Find the pin to be printed*/
      if (1 == type->pinloc[height][side][pin_index]) {
        fprintf(fp, "grid_%d__%d__pin_%d__%d__%d_, ", grid_x, grid_y, height, side, pin_index);
        pin_written_times++;
      }
      /* Make sure this pin is printed only once!!! (TODO: make sure for IO PAD, this remains ok)*/
      assert(1 == pin_written_times);
      break;
    case CHANX:
      verilog_determine_src_chan_coordinate_switch_box(drive_rr_nodes[inode], cur_rr_node, chan_side,
                                               switch_box_x, switch_box_y, &src_chan_x, &src_chan_y, &src_chan_port_name);
      /* For channels, ptc_num is the track_index*/
      track_index = drive_rr_nodes[inode]->ptc_num; 
      fprintf(fp, "chanx_%d__%d__%s_%d_, ", src_chan_x, src_chan_y, src_chan_port_name, track_index);
      //my_free(src_chan_port_name);
      break;
    case CHANY:
      verilog_determine_src_chan_coordinate_switch_box(drive_rr_nodes[inode], cur_rr_node, chan_side,
                                               switch_box_x, switch_box_y, &src_chan_x, &src_chan_y, &src_chan_port_name);
      /* For channels, ptc_num is the track_index*/
      track_index = drive_rr_nodes[inode]->ptc_num; 
      fprintf(fp, "chany_%d__%d__%s_%d_, ", src_chan_x, src_chan_y, src_chan_port_name, track_index);
      //my_free(src_chan_port_name);
      break;
    default: /* IPIN, SINK are invalid*/
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid rr_node type! Should be [OPIN|CHANX|CHANY].\n",
                 __FILE__, __LINE__);
      exit(1);
    }
  }

  /* Output port */
  dump_verilog_switch_box_chan_port(fp, switch_box_x, switch_box_y, chan_side, cur_rr_node);
  /* Add a comma because dump_verilog_switch_box_chan_port does not add so  */
  fprintf(fp, ", ");

  /* Configuration bits for this MUX*/
  path_id = -1;
  for (inode = 0; inode < mux_size; inode++) {
    if (drive_rr_nodes[inode] == &(rr_node[cur_rr_node->prev_node])) {
      path_id = inode;
      break;
    }
  }

  if (!((-1 != path_id)&&(path_id < mux_size))) {
  assert((-1 != path_id)&&(path_id < mux_size));
  }

  /* Depend on both technology and structure of this MUX*/
  switch (verilog_model->design_tech) {
  case SPICE_MODEL_DESIGN_CMOS:
    decode_cmos_mux_sram_bits(verilog_model, mux_size, path_id, &num_mux_sram_bits, &mux_sram_bits, &mux_level);
    break;
  case SPICE_MODEL_DESIGN_RRAM:
    decode_verilog_rram_mux(verilog_model, mux_size, path_id, &num_mux_sram_bits, &mux_sram_bits, &mux_level);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid design technology for verilog model (%s)!\n",
               __FILE__, __LINE__, verilog_model->name);
    exit(1);
  }

  /* Print SRAMs that configure this MUX */
  /* TODO: What about RRAM-based MUX? */
  cur_num_sram = sram_verilog_model->cnt;
  for (ilevel = 0; ilevel < num_mux_sram_bits; ilevel++) {
    fprintf(fp,"%s_out_%d_, ", sram_verilog_model->prefix, cur_num_sram);
    cur_num_sram++;
  }
  cur_num_sram = sram_verilog_model->cnt;
  for (ilevel = 0; ilevel < num_mux_sram_bits; ilevel++) {
    fprintf(fp,"%s_outb_%d_, ", sram_verilog_model->prefix, cur_num_sram);
    cur_num_sram++;
  }
  /* End */
  fprintf(fp, ");\n");
  
  /* Print the encoding in SPICE netlist for debugging */
  switch (verilog_model->design_tech) {
  case SPICE_MODEL_DESIGN_CMOS:
    fprintf(fp, "//----- SRAM bits for MUX[%d], level=%d, select_path_id=%d. -----\n", 
            verilog_model->cnt, mux_level, path_id);
    fprintf(fp, "//-----");
    cur_num_sram = sram_verilog_model->cnt;
    for (ilevel = 0; ilevel < num_mux_sram_bits; ilevel++) {
      fprintf(fp, "%d", mux_sram_bits[ilevel]);
      /* Store the configuraion bit to linked-list */
      conf_bits_head = add_conf_bit_info_to_llist(conf_bits_head, cur_num_sram,
                                                  mux_sram_bits[ilevel], 0, 0, verilog_model);
      cur_num_sram++;
    }
    fprintf(fp, "-----\n");
    break;
  case SPICE_MODEL_DESIGN_RRAM:
    fprintf(fp, "//----- BL/WL bits for 4T1R MUX[%d], level=%d, select_path_id=%d. -----\n", 
            verilog_model->cnt, mux_level, path_id);
    fprintf(fp, "//---- BL: ");
    for (ilevel = 0; ilevel < num_mux_sram_bits/2; ilevel++) {
      fprintf(fp, "%d", mux_sram_bits[ilevel]);
    }
    fprintf(fp, "-----\n");
    fprintf(fp, "//---- WL: ");
    for (ilevel = 0; ilevel < num_mux_sram_bits/2; ilevel++) {
      fprintf(fp, "%d", mux_sram_bits[ilevel+num_mux_sram_bits/2]);
    }
    fprintf(fp, "-----\n");
    /* Store the configuraion bit to linked-list */
    cur_num_sram = sram_verilog_model->cnt;
    for (ilevel = 0; ilevel < num_mux_sram_bits/2; ilevel++) {
      conf_bits_head = add_conf_bit_info_to_llist(conf_bits_head, cur_num_sram,
                                                  0, mux_sram_bits[ilevel], mux_sram_bits[ilevel+num_mux_sram_bits/2], 
                                                  verilog_model);
      cur_num_sram++;
    }
    fprintf(fp, "-----\n");
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid design technology for verilog model (%s)!\n",
               __FILE__, __LINE__, verilog_model->name);
  }
  
  /* update sram counter */
  sram_verilog_model->cnt = cur_num_sram;

  /* Free */
  my_free(mux_sram_bits);

  return;
}

/* Count the number of configuration bits of a rr_node*/
int count_verilog_switch_box_interc_conf_bits(int switch_box_x, int switch_box_y, int chan_side, 
                                              t_rr_node* cur_rr_node) {
  int num_conf_bits = 0;
  int switch_idx = 0;
  int num_drive_rr_nodes = 0;
  
  if (NULL == cur_rr_node) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])NULL cur_rr_node!\n",
               __FILE__, __LINE__);
    exit(1);    
    return num_conf_bits;
  }

  /* Determine if the interc lies inside a channel wire, that is interc between segments */
  if (1 == is_sb_interc_between_segments(switch_box_x, switch_box_y, cur_rr_node, chan_side)) {
    num_drive_rr_nodes = 0;
  } else {
    num_drive_rr_nodes = cur_rr_node->num_drive_rr_nodes;
  }

  /* fan_in >= 2 implies a MUX and requires configuration bits */
  if (2 > num_drive_rr_nodes) {
    return num_conf_bits;
  } else {
    switch_idx = cur_rr_node->drive_switches[0];
    assert(-1 < switch_idx);
    assert(SPICE_MODEL_MUX == switch_inf[switch_idx].spice_model->type);
    num_conf_bits = count_num_conf_bits_one_spice_model(switch_inf[switch_idx].spice_model, num_drive_rr_nodes);
    return num_conf_bits;
  }
}

void dump_verilog_switch_box_interc(FILE* fp, 
                              int switch_box_x, 
                              int switch_box_y, 
                              int chan_side,
                              t_rr_node* cur_rr_node) {
  int num_drive_rr_nodes = 0;  
  t_rr_node** drive_rr_nodes = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert((!(0 > switch_box_x))&&(!(switch_box_x > (nx + 1)))); 
  assert((!(0 > switch_box_y))&&(!(switch_box_y > (ny + 1)))); 
  /*
  find_drive_rr_nodes_switch_box(switch_box_x, switch_box_y, cur_rr_node, chan_side, 0, 
                                 &num_drive_rr_nodes, &drive_rr_nodes, &switch_index);
  */

  /* Determine if the interc lies inside a channel wire, that is interc between segments */
  if (1 == is_sb_interc_between_segments(switch_box_x, switch_box_y, cur_rr_node, chan_side)) {
    num_drive_rr_nodes = 0;
    drive_rr_nodes = NULL;
  } else {
    num_drive_rr_nodes = cur_rr_node->num_drive_rr_nodes;
    drive_rr_nodes = cur_rr_node->drive_rr_nodes;
  }

  if (0 == num_drive_rr_nodes) {
    /* Print a special direct connection*/
    dump_verilog_switch_box_short_interc(fp, switch_box_x, switch_box_y, chan_side, cur_rr_node, 
                                   num_drive_rr_nodes, cur_rr_node);
  } else if (1 == num_drive_rr_nodes) {
    /* Print a direct connection*/
    dump_verilog_switch_box_short_interc(fp, switch_box_x, switch_box_y, chan_side, cur_rr_node, 
                                   num_drive_rr_nodes, drive_rr_nodes[0]);
  } else if (1 < num_drive_rr_nodes) {
    /* Print the multiplexer, fan_in >= 2 */
    dump_verilog_switch_box_mux(fp, switch_box_x, switch_box_y, chan_side, cur_rr_node, 
                          num_drive_rr_nodes, drive_rr_nodes, 
                          cur_rr_node->drive_switches[0]);
  } /*Nothing should be done else*/ 

  /* Free */

  return;
}

/* Task: Print the subckt of a Switch Box.
 * A Switch Box subckt consists of following ports:
 * 1. Channel Y [x][y] inputs 
 * 2. Channel X [x+1][y] inputs
 * 3. Channel Y [x][y-1] outputs
 * 4. Channel X [x][y] outputs
 * 5. Grid[x][y+1] Right side outputs pins
 * 6. Grid[x+1][y+1] Left side output pins
 * 7. Grid[x+1][y+1] Bottom side output pins
 * 8. Grid[x+1][y] Top side output pins
 * 9. Grid[x+1][y] Left side output pins
 * 10. Grid[x][y] Right side output pins
 * 11. Grid[x][y] Top side output pins
 * 12. Grid[x][y+1] Bottom side output pins
 *
 *    --------------          --------------
 *    |            |          |            |
 *    |    Grid    |  ChanY   |    Grid    |
 *    |  [x][y+1]  | [x][y+1] | [x+1][y+1] |
 *    |            |          |            |
 *    --------------          --------------
 *                  ----------
 *       ChanX      | Switch |     ChanX 
 *       [x][y]     |   Box  |    [x+1][y]
 *                  | [x][y] |
 *                  ----------
 *    --------------          --------------
 *    |            |          |            |
 *    |    Grid    |  ChanY   |    Grid    |
 *    |   [x][y]   |  [x][y]  |  [x+1][y]  |
 *    |            |          |            |
 *    --------------          --------------
 */
void dump_verilog_routing_switch_box_subckt(FILE* fp, 
                                      int x, 
                                      int y, 
                                      t_ivec*** LL_rr_node_indices) {
  int itrack, inode, side, ix, iy;
  t_rr_node*** chan_rr_nodes = (t_rr_node***)my_malloc(sizeof(t_rr_node**)*4); /* 4 sides*/
  int* chan_width = (int*)my_malloc(sizeof(int)*4); /* 4 sides */

  int num_conf_bits = 0;
  int esti_sram_cnt = 0;
 
  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 

  /* Find all rr_nodes of channels */
  for (side = 0; side < 4; side++) {
    switch (side) {
    case 0:
      /* For the bording, we should take special care */
      if (y == ny) {
        chan_width[side] = 0;
        chan_rr_nodes[side] = NULL;
        break;
      }
      /* Routing channels*/
      ix = x; 
      iy = y + 1;
      /* Channel width */
      chan_width[side] = chan_width_y[ix];
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      chan_rr_nodes[side] = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*chan_width[side]);
      /* Collect rr_nodes for Tracks for top: chany[x][y+1] */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANY, itrack, LL_rr_node_indices);
        chan_rr_nodes[0][itrack] = &(rr_node[inode]);
      }
      break;
    case 1:
      /* For the bording, we should take speical care */
      if (x == nx) {
        chan_width[side] = 0;
        chan_rr_nodes[side] = NULL;
        break;
      }
      /* Routing channels*/
      ix = x + 1; 
      iy = y;
      /* Channel width */
      chan_width[side] = chan_width_x[iy];
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      chan_rr_nodes[side] = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*chan_width[side]);
      /* Collect rr_nodes for Tracks for right: chanX[x+1][y] */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANX, itrack, LL_rr_node_indices);
        chan_rr_nodes[1][itrack] = &(rr_node[inode]);
      }
      break;
    case 2:
      /* For the bording, we should take speical care */
      if (y == 0) {
        chan_width[side] = 0;
        chan_rr_nodes[side] = NULL;
        break;
      }
      /* Routing channels*/
      ix = x; 
      iy = y;
      /* Channel width */
      chan_width[side] = chan_width_y[ix];
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      chan_rr_nodes[side] = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*chan_width[side]);
      /* Collect rr_nodes for Tracks for bottom: chany[x][y] */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANY, itrack, LL_rr_node_indices);
        chan_rr_nodes[2][itrack] = &(rr_node[inode]);
      }
      break;
    case 3:
      /* For the bording, we should take speical care */
      if (x == 0) {
        chan_width[side] = 0;
        chan_rr_nodes[side] = NULL;
        break;
      }
      /* Routing channels*/
      ix = x; 
      iy = y;
      /* Channel width */
      chan_width[side] = chan_width_x[iy];
      /* Side: TOP => 0, RIGHT => 1, BOTTOM => 2, LEFT => 3 */
      chan_rr_nodes[side] = (t_rr_node**)my_malloc(sizeof(t_rr_node*)*chan_width[side]);
      /* Collect rr_nodes for Tracks for left: chanx[x][y] */
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        inode = get_rr_node_index(ix, iy, CHANX, itrack, LL_rr_node_indices);
        chan_rr_nodes[3][itrack] = &(rr_node[inode]);
      }
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid side index!\n", __FILE__, __LINE__);
      exit(1);
    }
  }

  /* Count the number of configuration bits to be consumed by this Switch block */
  for (side = 0; side < 4; side++) {
    switch (side) {
    case 0:
      /* For the bording, we should take speical care */
      if (y == ny) {
        break;
      }
      /* Start from the TOP side*/
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        assert(CHANY == chan_rr_nodes[side][itrack]->type);
        /* We care INC_DIRECTION tracks at this side*/
        if (INC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          num_conf_bits += count_verilog_switch_box_interc_conf_bits(x, y, side, chan_rr_nodes[side][itrack]);
        } 
      }
      break;
    case 1:
      /* For the bording, we should take speical care */
      if (x == nx) {
        break;
      }
      /* RIGHT side*/
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        assert(CHANX == chan_rr_nodes[side][itrack]->type);
        /* We care INC_DIRECTION tracks at this side*/
        if (INC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          num_conf_bits += count_verilog_switch_box_interc_conf_bits(x, y, side, chan_rr_nodes[side][itrack]);
        } 
      }
      break;
    case 2:
      /* For the bording, we should take speical care */
      if (y == 0) {
        break;
      }
      /* BOTTOM side*/
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        assert(CHANY == chan_rr_nodes[side][itrack]->type);
        /* We care DEC_DIRECTION tracks at this side*/
        if (DEC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          num_conf_bits += count_verilog_switch_box_interc_conf_bits(x, y, side, chan_rr_nodes[side][itrack]);
        } 
      }
      break;
    case 3:
      /* For the bording, we should take speical care */
      if (x == 0) {
        break;
      }
      /* LEFT side*/
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        assert(CHANX == chan_rr_nodes[side][itrack]->type);
        /* We care DEC_DIRECTION tracks at this side*/
        if (DEC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          num_conf_bits += count_verilog_switch_box_interc_conf_bits(x, y, side, chan_rr_nodes[side][itrack]);
        } 
      }
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid side index!\n", __FILE__, __LINE__);
      exit(1);
    }
  }
  /* Estimate the sram_verilog_model->cnt */
  esti_sram_cnt = sram_verilog_model->cnt + num_conf_bits;

  /* Print the definition of subckt*/
  fprintf(fp, "//----- Switch Box[%d][%d] Sub-Circuit -----\n", x, y);
  fprintf(fp, "module sb_%d__%d_ ( \n", x, y);
  fprintf(fp, "//----- Inputs/outputs of Channel Y [%d][%d] -----\n", x, y+1);
  /* 1. Channel Y [x][y+1] inputs */
  for (itrack = 0; itrack < chan_width[0]; itrack++) {
    switch (chan_rr_nodes[0][itrack]->direction) {
    case INC_DIRECTION:
      fprintf(fp, "  output chany_%d__%d__out_%d_,\n", x, y + 1, itrack);
      break;
    case DEC_DIRECTION:
      fprintf(fp, "  input chany_%d__%d__in_%d_,\n", x, y + 1, itrack);
      break;
    case BI_DIRECTION:
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of rr_node chany[%d][%d]_in/out[%d]!\n",
                 __FILE__, __LINE__, x, y + 1, itrack);
      exit(1);
    }
  }
  fprintf(fp, "//----- Inputs/outputs of Channel X [%d][%d] -----\n", x + 1, y);
  /* 2. Channel X [x+1][y] inputs */
  for (itrack = 0; itrack < chan_width[1]; itrack++) {
    switch (chan_rr_nodes[1][itrack]->direction) {
    case INC_DIRECTION:
      fprintf(fp, "  output chanx_%d__%d__out_%d_,\n", x + 1, y, itrack);
      break;
    case DEC_DIRECTION:
      fprintf(fp, "  input chanx_%d__%d__in_%d_,\n", x + 1, y, itrack);
      break;
    case BI_DIRECTION:
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of rr_node chanx[%d][%d]_in/out[%d]!\n",
                 __FILE__, __LINE__, x + 1, y, itrack);
      exit(1);
    }
  }
  fprintf(fp, "//----- Inputs/outputs of Channel Y [%d][%d] -----\n", x, y);
  /* 3. Channel Y [x][y] outputs */
  for (itrack = 0; itrack < chan_width[2]; itrack++) {
    switch (chan_rr_nodes[2][itrack]->direction) {
    case INC_DIRECTION:
      fprintf(fp, "  input chany_%d__%d__in_%d_,\n", x, y, itrack);
      break;
    case DEC_DIRECTION:
      fprintf(fp, "  output chany_%d__%d__out_%d_,\n", x, y, itrack);
      break;
    case BI_DIRECTION:
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of rr_node chany[%d][%d]_in/out[%d]!\n",
                 __FILE__, __LINE__, x, y, itrack);
      exit(1);
    }
  }
  fprintf(fp, "//----- Inputs/outputs of Channel X [%d][%d] -----\n", x, y);
  /* 4. Channel X [x][y] outputs */
  for (itrack = 0; itrack < chan_width[3]; itrack++) {
    switch (chan_rr_nodes[3][itrack]->direction) {
    case INC_DIRECTION:
      fprintf(fp, "  input chanx_%d__%d__in_%d_,\n", x, y, itrack);
      break;
    case DEC_DIRECTION:
      fprintf(fp, "  output chanx_%d__%d__out_%d_,\n", x, y, itrack);
      break;
    case BI_DIRECTION:
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File: %s [LINE%d]) Invalid direction of rr_node chanx[%d][%d]_in/out[%d]!\n",
                 __FILE__, __LINE__, x, y, itrack);
      exit(1);
    }
  }

  /* Considering the border */
  if (ny != y) {
    /* 5. Grid[x][y+1] Right side outputs pins */
    dump_verilog_grid_side_pins(fp, OPIN, x, y+1, 1, TRUE);
  }
  if (0 != x) {
    /* 6. Grid[x][y+1] Bottom side outputs pins */
    dump_verilog_grid_side_pins(fp, OPIN, x, y+1, 2, TRUE);
  }

  if (ny != y) {
    /* 7. Grid[x+1][y+1] Left side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x+1, y+1, 3, TRUE);
  }
  if (nx != x) {
    /* 8. Grid[x+1][y+1] Bottom side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x+1, y+1, 2, TRUE);
  }

  if (nx != x) {
    /* 9. Grid[x+1][y] Top side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x+1, y, 0, TRUE);
  }
  if (0 != y) {
    /* 10. Grid[x+1][y] Left side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x+1, y, 3, TRUE);
  }

  if (0 != y) {
    /* 11. Grid[x][y] Right side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x, y, 1, TRUE);
  } 
  if (0 != x) {
    /* 12. Grid[x][y] Top side output pins */
    dump_verilog_grid_side_pins(fp, OPIN, x, y, 0, TRUE);
  }
  /* Put down inpad and outpad ports */
  assert(!(0 > (inpad_verilog_model->sb_index_high[x][y] - inpad_verilog_model->sb_index_low[x][y])));
  if (0 < (inpad_verilog_model->sb_index_high[x][y] - inpad_verilog_model->sb_index_low[x][y])) {
    fprintf(fp, "  input [%d:%d] gfpga_input_%s, \n", 
            inpad_verilog_model->sb_index_high[x][y] - 1, 
            inpad_verilog_model->sb_index_low[x][y],
            inpad_verilog_model->prefix);
  }
  assert(!(0 > (outpad_verilog_model->sb_index_high[x][y] - outpad_verilog_model->sb_index_low[x][y])));
  if (0 < (outpad_verilog_model->sb_index_high[x][y] - outpad_verilog_model->sb_index_low[x][y])) {
    /* inverted output of each configuration bit */
    fprintf(fp, "  output [%d:%d] gfpga_output_%s, \n", 
            outpad_verilog_model->sb_index_high[x][y] - 1, 
            outpad_verilog_model->sb_index_low[x][y],
            outpad_verilog_model->prefix);
  }
  /* Put down configuration port */
  /* output of each configuration bit */
  /*assert(num_conf_bits == (sram_verilog_model->sb_index_high[x][y] - sram_verilog_model->sb_index_low[x][y])); */
  sram_verilog_model->sb_index_low[x][y] = sram_verilog_model->cnt;
  sram_verilog_model->sb_index_high[x][y] = esti_sram_cnt;

  if (0 < num_conf_bits) {
    fprintf(fp, "  input [%d:%d] %s_out, \n", 
            sram_verilog_model->sb_index_high[x][y] - 1, 
            sram_verilog_model->sb_index_low[x][y],
            sram_verilog_model->prefix); 
    /* inverted output of each configuration bit */
    fprintf(fp, "  input [%d:%d] %s_outb \n", 
            sram_verilog_model->sb_index_high[x][y] - 1, 
            sram_verilog_model->sb_index_low[x][y],
            sram_verilog_model->prefix); 
  }
  fprintf(fp, "); \n");

  /* Put down all the multiplexers */
  for (side = 0; side < 4; side++) {
    switch (side) {
    case 0:
      /* For the bording, we should take speical care */
      if (y == ny) {
        break;
      }
      /* Start from the TOP side*/
      fprintf(fp, "//----- TOP side Multiplexers -----\n");
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        assert(CHANY == chan_rr_nodes[side][itrack]->type);
        /* We care INC_DIRECTION tracks at this side*/
        if (INC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          dump_verilog_switch_box_interc(fp, x, y, side, chan_rr_nodes[side][itrack]);
        } 
      }
      break;
    case 1:
      /* For the bording, we should take speical care */
      if (x == nx) {
        break;
      }
      /* RIGHT side*/
      fprintf(fp, "//----- RIGHT side Multiplexers -----\n");
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        assert(CHANX == chan_rr_nodes[side][itrack]->type);
        /* We care INC_DIRECTION tracks at this side*/
        if (INC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          dump_verilog_switch_box_interc(fp, x, y, side, chan_rr_nodes[side][itrack]);
        } 
      }
      break;
    case 2:
      /* For the bording, we should take speical care */
      if (y == 0) {
        break;
      }
      /* BOTTOM side*/
      fprintf(fp, "//----- BOTTOM side Multiplexers -----\n");
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        assert(CHANY == chan_rr_nodes[side][itrack]->type);
        /* We care DEC_DIRECTION tracks at this side*/
        if (DEC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          dump_verilog_switch_box_interc(fp, x, y, side, chan_rr_nodes[side][itrack]);
        } 
      }
      break;
    case 3:
      /* For the bording, we should take speical care */
      if (x == 0) {
        break;
      }
      /* LEFT side*/
      fprintf(fp, "//----- LEFT side Multiplexers -----\n");
      for (itrack = 0; itrack < chan_width[side]; itrack++) {
        assert(CHANX == chan_rr_nodes[side][itrack]->type);
        /* We care DEC_DIRECTION tracks at this side*/
        if (DEC_DIRECTION == chan_rr_nodes[side][itrack]->direction) {
          dump_verilog_switch_box_interc(fp, x, y, side, chan_rr_nodes[side][itrack]);
        } 
      }
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid side index!\n", __FILE__, __LINE__);
      exit(1);
    }
  }
 
  fprintf(fp, "endmodule\n");

  /* Check */
  if (esti_sram_cnt != sram_verilog_model->cnt) {
  assert(esti_sram_cnt == sram_verilog_model->cnt);
  }

  /* record number of configuration bits in global array */
  num_conf_bits_sb[x][y] = num_conf_bits;

  /* Free chan_rr_nodes */
  my_free(chan_width);
  for (side = 0; side < 4; side++) {
    my_free(chan_rr_nodes[side]);
  }
  my_free(chan_rr_nodes);

  return;
}

/* Count the number of configuration bits of a rr_node*/
int count_verilog_connection_box_interc_conf_bits(t_rr_node* cur_rr_node) {
  int num_conf_bits = 0;
  int switch_idx = 0;
  int num_drive_rr_nodes = cur_rr_node->num_drive_rr_nodes;
  
  if (NULL == cur_rr_node) {
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])NULL cur_rr_node!\n",
               __FILE__, __LINE__);
    exit(1);    
    return num_conf_bits;
  }

  /* fan_in >= 2 implies a MUX and requires configuration bits */
  if (2 > num_drive_rr_nodes) {
    return num_conf_bits;
  } else {
    switch_idx = cur_rr_node->drive_switches[0];
    assert(-1 < switch_idx);
    assert(SPICE_MODEL_MUX == switch_inf[switch_idx].spice_model->type);
    num_conf_bits = count_num_conf_bits_one_spice_model(switch_inf[switch_idx].spice_model, num_drive_rr_nodes);
    return num_conf_bits;
  }
}

int count_verilog_connection_box_conf_bits(int num_ipin_rr_nodes,
                                           t_rr_node** ipin_rr_node) {
  int num_conf_bits = 0;
  int inode;

  for (inode = 0; inode < num_ipin_rr_nodes; inode++) {
    num_conf_bits += count_verilog_connection_box_interc_conf_bits(ipin_rr_node[inode]);
  }     

  return num_conf_bits;
}


/* SRC rr_node is the IPIN of a grid.*/
void dump_verilog_connection_box_short_interc(FILE* fp,
                                        t_rr_type chan_type,
                                        int cb_x,
                                        int cb_y,
                                        t_rr_node* src_rr_node) {
  t_rr_node* drive_rr_node = NULL;
  int iedge, check_flag;
  int xlow, ylow, height, side;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert((!(0 > cb_x))&&(!(cb_x > (nx + 1)))); 
  assert((!(0 > cb_y))&&(!(cb_y > (ny + 1)))); 
  assert(1 == src_rr_node->fan_in);

  /* Check the driver*/
  drive_rr_node = &(rr_node[src_rr_node->prev_node]); 
  assert((CHANX == drive_rr_node->type)||(CHANY == drive_rr_node->type));
  check_flag = 0;
  for (iedge = 0; iedge < drive_rr_node->num_edges; iedge++) {
    if (src_rr_node == &(rr_node[drive_rr_node->edges[iedge]])) {
      check_flag++;
    }
  }
  assert(1 == check_flag);

  xlow = src_rr_node->xlow;
  ylow = src_rr_node->ylow;
  height = grid[xlow][ylow].offset;

  /* Call the zero-resistance model */
  switch(chan_type) {
  case CHANX:
    fprintf(fp, "//----- short connection cbx[%d][%d]_grid[%d][%d]_pin[%d] -----\n", cb_x, cb_y, xlow, ylow + height, src_rr_node->ptc_num);
    break;
  case CHANY:
    fprintf(fp, "//----- short connection cby[%d][%d]_grid[%d][%d]_pin[%d] ------\n", cb_x, cb_y, xlow, ylow + height, src_rr_node->ptc_num);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
    exit(1);
  }
  fprintf(fp, "assign ");

  /* output porti -- > connect to the output at middle point of a channel */
  switch(drive_rr_node->type) {
  case CHANX:
    fprintf(fp, "chanx_%d__%d__midout_%d_ ", cb_x, cb_y, drive_rr_node->ptc_num);
    break;
  case CHANY:
    fprintf(fp, "chany_%d__%d__midout_%d_ ", cb_x, cb_y, drive_rr_node->ptc_num);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of drive_rr_node!\n", __FILE__, __LINE__);
    exit(1);
  }
  fprintf(fp, "= ");

  /* Input port*/
  switch(src_rr_node->type) {
  case IPIN:
  /* case SINK: */
    switch (drive_rr_node->type) {
    case CHANX: 
      assert(cb_x == xlow);
      if (cb_y == (ylow + height)) {
        side  = TOP;
        dump_verilog_grid_side_in_with_given_index(fp, src_rr_node->ptc_num, side, cb_x, cb_y);
      } else if ((cb_y + 1) == (ylow + height)) {
        side  = BOTTOM;
        dump_verilog_grid_side_in_with_given_index(fp, src_rr_node->ptc_num, side, cb_x, cb_y);
      } else {
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid coordinator of cb_y(=%d) and ylow+offset(=%d)!\n", __FILE__, __LINE__, cb_y, ylow+height);
        exit(1);
      }
      break;
    case CHANY: 
      assert(cb_y == (ylow + height));
      if (cb_x == xlow) {
        side  = RIGHT;
        dump_verilog_grid_side_in_with_given_index(fp, src_rr_node->ptc_num, side, cb_x, cb_y);
      } else if ((cb_x + 1) == xlow) {
        side  = LEFT;
        dump_verilog_grid_side_in_with_given_index(fp, src_rr_node->ptc_num, side, cb_x, cb_y);
      } else {
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid coordinator of cb_x(=%d) and xlow(=%d)!\n", __FILE__, __LINE__, cb_x, xlow);
        exit(1);
      }
      break;
    default: 
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of drive_rr_node!\n", __FILE__, __LINE__);
      exit(1);
    }
    break;
  /* SINK is a hypothesis node */
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of src_rr_node!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* End */
  fprintf(fp, "\n");


  return;
}

void dump_verilog_connection_box_mux(FILE* fp,
                               t_rr_type chan_type,
                               int cb_x,
                               int cb_y,
                               t_rr_node* src_rr_node) {
  int mux_size, cur_num_sram, ilevel;
  t_rr_node** drive_rr_nodes = NULL;
  int inode, mux_level, path_id, switch_index;
  t_spice_model* mux_verilog_model = NULL;
  int num_mux_sram_bits = 0;
  int* mux_sram_bits = NULL;
  t_rr_type drive_rr_node_type = NUM_RR_TYPES;
  int xlow, ylow, offset, side;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > cb_x))&&(!(cb_x > (nx + 1)))); 
  assert((!(0 > cb_y))&&(!(cb_y > (ny + 1)))); 

  /* Find drive_rr_nodes*/
  mux_size = src_rr_node->num_drive_rr_nodes;
  drive_rr_nodes = src_rr_node->drive_rr_nodes; 

  /* Configuration bits for MUX*/
  path_id = -1;
  for (inode = 0; inode < mux_size; inode++) {
    if (drive_rr_nodes[inode] == &(rr_node[src_rr_node->prev_node])) {
      path_id = inode;
      break;
    }
  }
  assert((-1 != path_id)&&(path_id < mux_size));

  switch_index = src_rr_node->drive_switches[path_id];

  mux_verilog_model = switch_inf[switch_index].spice_model;

  /* Call the MUX SPICE model */
  fprintf(fp, "%s_size%d %s_size%d_%d_ (", 
          mux_verilog_model->name, mux_size, 
          mux_verilog_model->prefix, mux_size, mux_verilog_model->cnt);
  mux_verilog_model->cnt++;
  /* Check drive_rr_nodes type, should be the same*/
  for (inode = 0; inode < mux_size; inode++) {
    if (NUM_RR_TYPES == drive_rr_node_type) { 
      drive_rr_node_type = drive_rr_nodes[inode]->type;
    } else {
      assert(drive_rr_node_type == drive_rr_nodes[inode]->type);
      assert((CHANX == drive_rr_nodes[inode]->type)||(CHANY == drive_rr_nodes[inode]->type));
    }
  } 
  /* input port*/
  for (inode = 0; inode < mux_size; inode++) {
    switch(drive_rr_nodes[inode]->type) {
    case CHANX:
      fprintf(fp, "chanx_%d__%d__midout_%d_, ", cb_x, cb_y, drive_rr_nodes[inode]->ptc_num);
      break;
    case CHANY:
      fprintf(fp, "chany_%d__%d__midout_%d_, ", cb_x, cb_y, drive_rr_nodes[inode]->ptc_num);
      break;
    default: 
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of drive_rr_node!\n", __FILE__, __LINE__);
      exit(1);
    }
  }
  /* output port*/
  xlow = src_rr_node->xlow;
  ylow = src_rr_node->ylow;
  offset = grid[xlow][ylow].offset;
  switch(src_rr_node->type) {
  case IPIN:
  /* case SINK: */
    switch (drive_rr_node_type) {
    case CHANX:  
      assert(cb_x == xlow);
      if (cb_y == (ylow + offset)) { /* CHANX is on the TOP side of grid */
        side  = TOP;
        dump_verilog_grid_side_in_with_given_index(fp, src_rr_node->ptc_num, side, cb_x, cb_y);
      } else if ((cb_y + 1) == (ylow + offset)) {
        side  = BOTTOM;
        dump_verilog_grid_side_in_with_given_index(fp, src_rr_node->ptc_num, side, cb_x, cb_y + 1);
      } else {
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid coordinator of cb_y(=%d) and ylow+offset(=%d)!\n", __FILE__, __LINE__, cb_y, ylow+offset);
        exit(1);
      }
      break;
    case CHANY: 
      assert(cb_y == (ylow + offset));
      if (cb_x == xlow) {
        side  = RIGHT;
        dump_verilog_grid_side_in_with_given_index(fp, src_rr_node->ptc_num, side, cb_x, cb_y);
      } else if ((cb_x + 1) == xlow) {
        side  = LEFT;
        dump_verilog_grid_side_in_with_given_index(fp, src_rr_node->ptc_num, side, cb_x + 1, cb_y);
      } else {
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid coordinator of cb_x(=%d) and xlow(=%d)!\n", __FILE__, __LINE__, cb_x, xlow);
        exit(1);
      }
      break;
    default: 
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of drive_rr_node!\n", __FILE__, __LINE__);
      exit(1);
    }
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of src_rr_node!\n", __FILE__, __LINE__);
    exit(1);
  }
  fprintf(fp, ", "); 

  switch (mux_verilog_model->design_tech) {
  case SPICE_MODEL_DESIGN_CMOS:
    decode_cmos_mux_sram_bits(mux_verilog_model, mux_size, path_id, &num_mux_sram_bits, &mux_sram_bits, &mux_level);
    break;
  case SPICE_MODEL_DESIGN_RRAM:
    decode_verilog_rram_mux(mux_verilog_model, mux_size, path_id, &num_mux_sram_bits, &mux_sram_bits, &mux_level);
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid design technology for verilog model (%s)!\n",
               __FILE__, __LINE__, mux_verilog_model->name);
  }
 
  /* Print SRAMs that configure this MUX */
  /* TODO: What about RRAM-based MUX? */
  cur_num_sram = sram_verilog_model->cnt;
  for (ilevel = 0; ilevel < num_mux_sram_bits; ilevel++) {
    fprintf(fp,"%s_out[%d], ", sram_verilog_model->prefix, cur_num_sram);
    cur_num_sram++;
  }
  cur_num_sram = sram_verilog_model->cnt;
  for (ilevel = 0; ilevel < num_mux_sram_bits; ilevel++) {
    fprintf(fp,"%s_outb[%d], ", sram_verilog_model->prefix, cur_num_sram);
    cur_num_sram++;
  }

  /* End with svdd and sgnd, subckt name*/
  fprintf(fp, ");\n");

  /* Print the encoding in SPICE netlist for debugging */
  switch (mux_verilog_model->design_tech) {
  case SPICE_MODEL_DESIGN_CMOS:
    fprintf(fp, "//----- SRAM bits for MUX[%d], level=%d, select_path_id=%d. -----\n", 
            mux_verilog_model->cnt, mux_level, path_id);
    fprintf(fp, "//-----");
    cur_num_sram = sram_verilog_model->cnt;
    for (ilevel = 0; ilevel < num_mux_sram_bits; ilevel++) {
      fprintf(fp, "%d", mux_sram_bits[ilevel]);
      /* Store the configuraion bit to linked-list */
      conf_bits_head = add_conf_bit_info_to_llist(conf_bits_head, cur_num_sram,
                                                  mux_sram_bits[ilevel], 0, 0, mux_verilog_model);
      cur_num_sram++;
    }
    fprintf(fp, "-----\n");
    break;
  case SPICE_MODEL_DESIGN_RRAM:
    fprintf(fp, "//----- BL/WL bits for 4T1R MUX[%d], level=%d, select_path_id=%d. -----\n", 
            mux_verilog_model->cnt, mux_level, path_id);
    fprintf(fp, "//---- BL: ");
    for (ilevel = 0; ilevel < num_mux_sram_bits/2; ilevel++) {
      fprintf(fp, "%d", mux_sram_bits[ilevel]);
    }
    fprintf(fp, "-----\n");
    fprintf(fp, "//---- WL: ");
    for (ilevel = 0; ilevel < num_mux_sram_bits/2; ilevel++) {
      fprintf(fp, "%d", mux_sram_bits[ilevel+num_mux_sram_bits/2]);
    }
    fprintf(fp, "-----\n");
    /* Store the configuraion bit to linked-list */
    cur_num_sram = sram_verilog_model->cnt;
    for (ilevel = 0; ilevel < num_mux_sram_bits/2; ilevel++) {
      conf_bits_head = add_conf_bit_info_to_llist(conf_bits_head, cur_num_sram,
                                                  0, mux_sram_bits[ilevel], mux_sram_bits[ilevel+num_mux_sram_bits/2], 
                                                  mux_verilog_model);
      cur_num_sram++;
    }
    fprintf(fp, "-----\n");
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid design technology for verilog model (%s)!\n",
               __FILE__, __LINE__, mux_verilog_model->name);
  }

  /* update SRAM counters */
  sram_verilog_model->cnt = cur_num_sram;

  /* Free */
  my_free(mux_sram_bits);

  return;
}

void dump_verilog_connection_box_interc(FILE* fp,
                                  t_rr_type chan_type,
                                  int cb_x,
                                  int cb_y,
                                  t_rr_node* src_rr_node) {
  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert((!(0 > cb_x))&&(!(cb_x > (nx + 1)))); 
  assert((!(0 > cb_y))&&(!(cb_y > (ny + 1)))); 

  if (1 == src_rr_node->fan_in) {
    /* Print a direct connection*/
    dump_verilog_connection_box_short_interc(fp, chan_type, cb_x, cb_y, src_rr_node);
  } else if (1 < src_rr_node->fan_in) {
    /* Print the multiplexer, fan_in >= 2 */
    dump_verilog_connection_box_mux(fp, chan_type, cb_x, cb_y, src_rr_node);
  } /*Nothing should be done else*/ 
   
  return;
}

/* Print connection boxes
 * Print the sub-circuit of a connection Box (Type: [CHANX|CHANY])
 * Actually it is very similiar to switch box but
 * the difference is connection boxes connect Grid INPUT Pins to channels
 * TODO: merge direct connections into CB 
 *    --------------             --------------
 *    |            |             |            |
 *    |    Grid    |   ChanY     |    Grid    |
 *    |  [x][y+1]  |   [x][y]    | [x+1][y+1] |
 *    |            | Connection  |            |
 *    -------------- Box_Y[x][y] --------------
 *                   ----------
 *       ChanX       | Switch |        ChanX 
 *       [x][y]      |   Box  |       [x+1][y]
 *     Connection    | [x][y] |      Connection 
 *    Box_X[x][y]    ----------     Box_X[x+1][y]
 *    --------------             --------------
 *    |            |             |            |
 *    |    Grid    |  ChanY      |    Grid    |
 *    |   [x][y]   | [x][y-1]    |  [x+1][y]  |
 *    |            | Connection  |            |
 *    --------------Box_Y[x][y-1]--------------
 */
void dump_verilog_routing_connection_box_subckt(FILE* fp,
                                          t_rr_type chan_type,
                                          int x,
                                          int y,
                                          int chan_width,
                                          t_ivec*** LL_rr_node_indices) {
  int itrack, inode, side;
  int side_cnt = 0;
  int num_ipin_rr_node = 0;
  t_rr_node** ipin_rr_nodes = NULL;
  int num_temp_rr_node = 0;
  t_rr_node** temp_rr_nodes = NULL;
  
  int num_conf_bits = 0;
  int esti_sram_cnt = 0;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert((!(0 > x))&&(!(x > (nx + 1)))); 
  assert((!(0 > y))&&(!(y > (ny + 1)))); 
  
  /* Print the definition of subckt*/
  fprintf(fp, "module ");
  /* Identify the type of connection box */
  switch(chan_type) {
  case CHANX:
    fprintf(fp, "cbx_%d__%d_ ", x, y);
    break;
  case CHANY:
    fprintf(fp, "cby_%d__%d_ ", x, y);
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
    exit(1);
  }
 
  fprintf(fp, "(\n");
  /* Print the ports of channels*/
  /*connect to the mid point of a track*/
  for (itrack = 0; itrack < chan_width; itrack++) {
    switch(chan_type) { 
    case CHANX:
      fprintf(fp, "input chanx_%d__%d__midout_%d_, \n", x, y, itrack);
      break;
    case CHANY:
      fprintf(fp, "input chany_%d__%d__midout_%d_, \n ", x, y, itrack);
      break;
    default: 
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
      exit(1);
    }
  }
  /* Print the ports of grids*/
  side_cnt = 0;
  num_ipin_rr_node = 0;  
  for (side = 0; side < 4; side++) {
    switch (side) {
    case 0: /* TOP */
      switch(chan_type) { 
      case CHANX:
        /* BOTTOM INPUT Pins of Grid[x][y+1] */
        dump_verilog_grid_side_pins(fp, IPIN, x, y + 1, 2, TRUE);
        /* Collect IPIN rr_nodes*/ 
        temp_rr_nodes = verilog_get_grid_side_pin_rr_nodes(&num_temp_rr_node, IPIN, x, y + 1, 2, LL_rr_node_indices);
        /* Update the ipin_rr_nodes, if ipin_rr_nodes is NULL, realloc will do a pure malloc */
        ipin_rr_nodes = (t_rr_node**)realloc(ipin_rr_nodes, sizeof(t_rr_node*)*(num_ipin_rr_node + num_temp_rr_node));
        for (inode = num_ipin_rr_node; inode < (num_ipin_rr_node + num_temp_rr_node); inode++) {
          ipin_rr_nodes[inode] = temp_rr_nodes[inode - num_ipin_rr_node];
        } 
        /* Count in the new members*/
        num_ipin_rr_node += num_temp_rr_node; 
        /* Free the temp_ipin_rr_node */
        my_free(temp_rr_nodes);
        num_temp_rr_node = 0;
        side_cnt++;
        break; 
      case CHANY:
        /* Nothing should be done */
        break;
      default: 
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
        exit(1);
      }
      break;
    case 1: /* RIGHT */
      switch(chan_type) { 
      case CHANX:
        /* Nothing should be done */
        break; 
      case CHANY:
        /* LEFT INPUT Pins of Grid[x+1][y] */
        dump_verilog_grid_side_pins(fp, IPIN, x + 1, y, 3, TRUE);
        /* Collect IPIN rr_nodes*/ 
        temp_rr_nodes = verilog_get_grid_side_pin_rr_nodes(&num_temp_rr_node, IPIN, x + 1, y, 3, LL_rr_node_indices);
        /* Update the ipin_rr_nodes, if ipin_rr_nodes is NULL, realloc will do a pure malloc */
        ipin_rr_nodes = (t_rr_node**)realloc(ipin_rr_nodes, sizeof(t_rr_node*)*(num_ipin_rr_node + num_temp_rr_node));
        for (inode = num_ipin_rr_node; inode < (num_ipin_rr_node + num_temp_rr_node); inode++) {
          ipin_rr_nodes[inode] = temp_rr_nodes[inode - num_ipin_rr_node];
        } 
        /* Count in the new members*/
        num_ipin_rr_node += num_temp_rr_node; 
        /* Free the temp_ipin_rr_node */
        my_free(temp_rr_nodes);
        num_temp_rr_node = 0;
        side_cnt++;
        break;
      default: 
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
        exit(1);
      }
      break;
    case 2: /* BOTTOM */
      switch(chan_type) { 
      case CHANX:
        /* TOP INPUT Pins of Grid[x][y] */
        dump_verilog_grid_side_pins(fp, IPIN, x, y, 0, TRUE);
        /* Collect IPIN rr_nodes*/ 
        temp_rr_nodes = verilog_get_grid_side_pin_rr_nodes(&num_temp_rr_node, IPIN, x, y, 0, LL_rr_node_indices);
        /* Update the ipin_rr_nodes, if ipin_rr_nodes is NULL, realloc will do a pure malloc */
        ipin_rr_nodes = (t_rr_node**)realloc(ipin_rr_nodes, sizeof(t_rr_node*)*(num_ipin_rr_node + num_temp_rr_node));
        for (inode = num_ipin_rr_node; inode < (num_ipin_rr_node + num_temp_rr_node); inode++) {
          ipin_rr_nodes[inode] = temp_rr_nodes[inode - num_ipin_rr_node];
        } 
        /* Count in the new members*/
        num_ipin_rr_node += num_temp_rr_node; 
        /* Free the temp_ipin_rr_node */
        my_free(temp_rr_nodes);
        num_temp_rr_node = 0;
        side_cnt++;
        break; 
      case CHANY:
        /* Nothing should be done */
        break;
      default: 
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
        exit(1);
      }
      break;
    case 3: /* LEFT */
      switch(chan_type) { 
      case CHANX:
        /* Nothing should be done */
        break; 
      case CHANY:
        /* RIGHT INPUT Pins of Grid[x][y] */
        dump_verilog_grid_side_pins(fp, IPIN, x, y, 1, TRUE);
        /* Collect IPIN rr_nodes*/ 
        temp_rr_nodes = verilog_get_grid_side_pin_rr_nodes(&num_temp_rr_node, IPIN, x, y, 1, LL_rr_node_indices);
        /* Update the ipin_rr_nodes, if ipin_rr_nodes is NULL, realloc will do a pure malloc */
        ipin_rr_nodes = (t_rr_node**)realloc(ipin_rr_nodes, sizeof(t_rr_node*)*(num_ipin_rr_node + num_temp_rr_node));
        for (inode = num_ipin_rr_node; inode < (num_ipin_rr_node + num_temp_rr_node); inode++) {
          ipin_rr_nodes[inode] = temp_rr_nodes[inode - num_ipin_rr_node];
        } 
        /* Count in the new members*/
        num_ipin_rr_node += num_temp_rr_node; 
        /* Free the temp_ipin_rr_node */
        my_free(temp_rr_nodes);
        num_temp_rr_node = 0;
        side_cnt++;
        break;
      default: 
        vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
        exit(1);
      }
      break;
    default:
      vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid side index!\n", __FILE__, __LINE__);
      exit(1);
    }
    fprintf(fp, "\n");
  }
  /* Check */
  assert(2 == side_cnt);

  /* Count the number of configuration bits */
  num_conf_bits = count_verilog_connection_box_conf_bits(num_ipin_rr_node, ipin_rr_nodes);
  esti_sram_cnt = sram_verilog_model->cnt + num_conf_bits;
  /* Print inpad, outpad ports and sram ports */
  switch(chan_type) { 
  case CHANX:
    /* inpad */
    /*
    assert(!(0 > inpad_verilog_model->cbx_index_high[x][y] - inpad_verilog_model->cbx_index_low[x][y]));
    if (0 < inpad_verilog_model->cbx_index_high[x][y] - inpad_verilog_model->cbx_index_low[x][y]) {
      fprintf(fp, "  gfpga_input_%s[%d:%d], \n", 
              inpad_verilog_model->prefix, 
              inpad_verilog_model->cbx_index_low[x][y],
              inpad_verilog_model->cbx_index_high[x][y] - 1);
    } */
    /* outpad */
    /*
    assert(!(0 > outpad_verilog_model->cbx_index_high[x][y] - outpad_verilog_model->cbx_index_low[x][y]));
    if (0 < outpad_verilog_model->cbx_index_high[x][y] - outpad_verilog_model->cbx_index_low[x][y]) {
      fprintf(fp, "  gfpga_output_%s[%d:%d], \n", 
              outpad_verilog_model->prefix, 
              outpad_verilog_model->cbx_index_low[x][y],
              outpad_verilog_model->cbx_index_high[x][y] - 1);
    } */
    /* Configuration ports */
    sram_verilog_model->cbx_index_high[x][y] = esti_sram_cnt;
    assert(!(0 > sram_verilog_model->cbx_index_high[x][y] - sram_verilog_model->cbx_index_low[x][y]));
    assert(num_conf_bits == (sram_verilog_model->cbx_index_high[x][y] - sram_verilog_model->cbx_index_low[x][y]));
    if (0 < sram_verilog_model->cbx_index_high[x][y] - sram_verilog_model->cbx_index_low[x][y]) {
      fprintf(fp, " input [%d:%d] %s_out, \n", 
              sram_verilog_model->cbx_index_high[x][y] - 1,
              sram_verilog_model->cbx_index_low[x][y],
              sram_verilog_model->prefix);
      /* inverted output of each configuration bit */
      fprintf(fp, " input [%d:%d] %s_outb \n", 
              sram_verilog_model->cbx_index_high[x][y] - 1,
              sram_verilog_model->cbx_index_low[x][y],
              sram_verilog_model->prefix); 
        
    }
    break;
  case CHANY:
    /* inpad */
    /*
    assert(!(0 > inpad_verilog_model->cby_index_high[x][y] - inpad_verilog_model->cby_index_low[x][y]));
    if (0 < inpad_verilog_model->cby_index_high[x][y] - inpad_verilog_model->cby_index_low[x][y]) {
      fprintf(fp, "  gfpga_input_%s[%d:%d], \n", 
              inpad_verilog_model->prefix, 
              inpad_verilog_model->cby_index_low[x][y],
              inpad_verilog_model->cby_index_high[x][y] - 1);
    } */
    /* outpad */
    /*
    assert(!(0 > outpad_verilog_model->cby_index_high[x][y] - outpad_verilog_model->cby_index_low[x][y]));
    if (0 < outpad_verilog_model->cby_index_high[x][y] - outpad_verilog_model->cby_index_low[x][y]) {
      fprintf(fp, "  gfpga_output_%s[%d:%d], \n", 
              outpad_verilog_model->prefix, 
              outpad_verilog_model->cby_index_low[x][y],
              outpad_verilog_model->cby_index_high[x][y] - 1);
    } */
    /* Configuration ports */
    sram_verilog_model->cby_index_high[x][y] = esti_sram_cnt;
    assert(!(0 > sram_verilog_model->cby_index_high[x][y] - sram_verilog_model->cby_index_low[x][y]));
    assert(num_conf_bits == (sram_verilog_model->cby_index_high[x][y] - sram_verilog_model->cby_index_low[x][y]));
    if (0 < sram_verilog_model->cby_index_high[x][y] - sram_verilog_model->cby_index_low[x][y]) {
      fprintf(fp, " input [%d:%d] %s_out, \n", 
              sram_verilog_model->cby_index_high[x][y] - 1,
              sram_verilog_model->cby_index_low[x][y],
              sram_verilog_model->prefix);
      /* inverted output of each configuration bit */
      fprintf(fp, " input [%d:%d] %s_outb \n", 
              sram_verilog_model->cby_index_high[x][y] - 1,
              sram_verilog_model->cby_index_low[x][y],
              sram_verilog_model->prefix); 
        
    }
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* subckt definition ends with svdd and sgnd*/
  fprintf(fp, ");\n");


  /* Print multiplexers or direct interconnect*/
  for (inode = 0; inode < num_ipin_rr_node; inode++) {
    dump_verilog_connection_box_interc(fp, ipin_rr_nodes[inode]->type, x, y, ipin_rr_nodes[inode]);
  } 

  fprintf(fp, "endmodule\n");

  /* Check */
  assert(esti_sram_cnt == sram_verilog_model->cnt);

  /* record number of configuration bits in global array */
  switch(chan_type) { 
  case CHANX:
    num_conf_bits_cbx[x][y] = num_conf_bits;
    break;
  case CHANY:
    num_conf_bits_cby[x][y] = num_conf_bits;
    break;
  default: 
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid type of channel!\n", __FILE__, __LINE__);
    exit(1);
  }

  /* Free */
  my_free(ipin_rr_nodes);
 
  return;
}


/* Top Function*/
/* Build the routing resource SPICE sub-circuits*/
void dump_verilog_routing_resources(char* subckt_dir,
                                    t_arch arch,
                                    t_det_routing_arch* routing_arch,
                                    t_ivec*** LL_rr_node_indices) {
  FILE* fp = NULL;
  char* verilog_name = my_strcat(subckt_dir, routing_verilog_file_name);
  int ix, iy; 
  int chan_width;
 
  assert(UNI_DIRECTIONAL == routing_arch->directionality);

  /* Create FILE */
  fp = fopen(verilog_name, "w");
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(FILE:%s,LINE[%d])Failure in create verilog netlist %s",__FILE__, __LINE__, routing_verilog_file_name); 
    exit(1);
  } 
  dump_verilog_file_header(fp,"Routing Resources");
  
  /* Two major tasks: 
   * 1. Generate sub-circuits for Routing Channels 
   * 2. Generate sub-circuits for Switch Boxes
   */ 
  /* Now: First task: Routing channels
   * Sub-circuits are named as chanx[ix][iy] or chany[ix][iy] for horizontal or vertical channels
   * each channels consist of a number of routing tracks. (Actually they are metal wires)
   * We only support single-driver routing architecture. 
   * The direction is defined as INC_DIRECTION ------> and DEC_DIRECTION <-------- for chanx
   * The direction is defined as INC_DIRECTION /|\ and DEC_DIRECTION | for chany
   *                                            |                    |
   *                                            |                    |
   *                                            |                   \|/
   * For INC_DIRECTION chanx, the inputs are at the left of channels, the outputs are at the right of channels
   * For DEC_DIRECTION chanx, the inputs are at the right of channels, the outputs are at the left of channels
   * For INC_DIRECTION chany, the inputs are at the bottom of channels, the outputs are at the top of channels
   * For DEC_DIRECTION chany, the inputs are at the top of channels, the outputs are at the bottom of channels
   */
  /* X - channels [1...nx][0..ny]*/
  vpr_printf(TIO_MESSAGE_INFO, "Writing X-direction Channels...\n");
  for (iy = 0; iy < (ny + 1); iy++) {
    for (ix = 1; ix < (nx + 1); ix++) {
      chan_width = chan_width_x[iy];
      dump_verilog_routing_chan_subckt(fp, ix, iy, CHANX, chan_width, LL_rr_node_indices, arch.num_segments, arch.Segments);
    }
  }
  /* Y - channels [1...ny][0..nx]*/
  vpr_printf(TIO_MESSAGE_INFO, "Writing Y-direction Channels...\n");
  for (ix = 0; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      chan_width = chan_width_y[ix];
      dump_verilog_routing_chan_subckt(fp, ix, iy, CHANY, chan_width, LL_rr_node_indices, arch.num_segments, arch.Segments);
    }
  }

  /* Switch Boxes*/
  for (ix = 0; ix < (nx + 1); ix++) {
    for (iy = 0; iy < (ny + 1); iy++) {
      vpr_printf(TIO_MESSAGE_INFO, "Writing Switch Boxes[%d][%d]...\n", ix, iy);
      update_spice_models_routing_index_low(ix, iy, arch.spice->num_spice_model, arch.spice->spice_models);
      dump_verilog_routing_switch_box_subckt(fp, ix, iy, LL_rr_node_indices);
      update_spice_models_routing_index_high(ix, iy, arch.spice->num_spice_model, arch.spice->spice_models);
    }
  }

  /* Connection Boxes */
  /* X - channels [1...nx][0..ny]*/
  for (iy = 0; iy < (ny + 1); iy++) {
    for (ix = 1; ix < (nx + 1); ix++) {
      vpr_printf(TIO_MESSAGE_INFO, "Writing X-direction Connection Boxes[%d][%d]...\n", ix, iy);
      chan_width = chan_width_x[iy];
      update_spice_models_routing_index_low(ix, iy, arch.spice->num_spice_model, arch.spice->spice_models);
      dump_verilog_routing_connection_box_subckt(fp, CHANX, ix, iy, chan_width, LL_rr_node_indices);
      update_spice_models_routing_index_high(ix, iy, arch.spice->num_spice_model, arch.spice->spice_models);
    }
  }
  /* Y - channels [1...ny][0..nx]*/
  for (ix = 0; ix < (nx + 1); ix++) {
    for (iy = 1; iy < (ny + 1); iy++) {
      vpr_printf(TIO_MESSAGE_INFO, "Writing Y-direction Connection Boxes[%d][%d]...\n", ix, iy);
      chan_width = chan_width_y[ix];
      update_spice_models_routing_index_low(ix, iy, arch.spice->num_spice_model, arch.spice->spice_models);
      dump_verilog_routing_connection_box_subckt(fp, CHANY, ix, iy, chan_width, LL_rr_node_indices);
      update_spice_models_routing_index_high(ix, iy, arch.spice->num_spice_model, arch.spice->spice_models);
    }
  }
  
  /* Close the file*/
  fclose(fp);
  
  return;
}
