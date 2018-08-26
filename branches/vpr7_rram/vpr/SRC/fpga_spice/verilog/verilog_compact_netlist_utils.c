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
#include <sys/stat.h>
#include <unistd.h>

/* Include vpr structs*/
#include "util.h"
#include "physical_types.h"
#include "vpr_types.h"
#include "globals.h"
#include "rr_graph.h"
#include "vpr_utils.h"

/* Include SPICE support headers*/
#include "linkedlist.h"
#include "fpga_spice_utils.h"
#include "fpga_spice_mux_utils.h"
#include "fpga_spice_pbtypes_utils.h"
#include "fpga_spice_bitstream_utils.h"
#include "spice_mux.h"
#include "fpga_spice_globals.h"

/* Include Synthesizable Verilog headers */
#include "verilog_global.h"
#include "verilog_utils.h"
#include "verilog_primitives.h"
#include "verilog_pbtypes.h"
#include "verilog_top_netlist.h"

/* Create a fake (x, y) coorindate for a I/O block on the border side */
void verilog_compact_generate_fake_xy_for_io_border_side(int border_side,  
                                                         int* ix, int* iy) {
  switch (border_side) {
  case 0: /* TOP*/
    (*ix) = 1;
    (*iy) = ny + 1;
    break;
  case 1: /*RIGHT */
    (*ix) = nx + 1;
    (*iy) = 1;
    break;
  case 2: /*BOTTOM */
    (*ix) = 1;
    (*iy) = 0;
    break;
  case 3: /* LEFT */
    (*ix) = 0;
    (*iy) = 1;
    break;
  default:
    vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d]) Invalid border_side(%d) for I/O grid!\n",
               __FILE__, __LINE__, border_side);
    exit(1);
  }

  return;
}

/* We print all the pins of a type descriptor in the following sequence 
 * TOP, RIGHT, BOTTOM, LEFT
 */
void dump_compact_verilog_grid_pins(FILE* fp,
                                    t_type_ptr grid_type_descriptor,
                                    boolean dump_port_type,
                                    boolean dump_last_comma) {
  int iheight, side, ipin, class_id; 
  int side_pin_index;
  int num_dumped_port = 0;
  int first_dump = 1;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert(NULL != grid_type_descriptor);
  assert(0 < grid_type_descriptor->capacity);

  for (side = 0; side < 4; side++) {
    /* Count the number of pins */
    side_pin_index = 0;
    //for (iz = 0; iz < grid_type_descriptor->capacity; iz++) {
      for (iheight = 0; iheight < grid_type_descriptor->height; iheight++) {
        for (ipin = 0; ipin < grid_type_descriptor->num_pins; ipin++) {
          if (1 == grid_type_descriptor->pinloc[iheight][side][ipin]) {
            /* Add comma if needed */
            if (1 == first_dump) {
              first_dump = 0;
            } else { 
              if (TRUE == dump_port_type) {
                fprintf(fp, ",\n");
              } else {
                fprintf(fp, ",\n");
             }
            }
            if (TRUE == dump_port_type) {
              /* Determine this pin is an input or output */
              class_id = grid_type_descriptor->pin_class[ipin];
              switch (grid_type_descriptor->class_inf[class_id].type) {
              case RECEIVER:
                fprintf(fp, "input ");
                break;
              case DRIVER:
                fprintf(fp, "output ");
                break;
              default:
                vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid pin_class_type!\n",
                           __FILE__, __LINE__);
                exit(1);
              }
            }
            /* This pin appear at this side! */
            fprintf(fp, " %s_height_%d__pin_%d_", 
                      convert_side_index_to_string(side), iheight, ipin);
            /* Update counter */
            num_dumped_port++;
            side_pin_index++;
          }
        }
      }  
    //}
  }

  if ((0 < num_dumped_port)&&(TRUE == dump_last_comma)) {
    if (TRUE == dump_port_type) {
      fprintf(fp, ",\n");
    } else {
      fprintf(fp, ",\n");
    }
  }

  return;
} 

/* Special for I/O grid, we need only part of the ports
 * i.e., grid[0][0..ny] only need the right side ports.
 */
/* We print all the pins of a type descriptor in the following sequence 
 * TOP, RIGHT, BOTTOM, LEFT
 */
void dump_compact_verilog_io_grid_pins(FILE* fp,
                                       t_type_ptr grid_type_descriptor,
                                       int border_side,
                                       boolean dump_port_type,
                                       boolean dump_last_comma) {
  int iheight, ipin; 
  int side_pin_index;
  int class_id = -1;
  int num_dumped_port = 0;
  int first_dump = 1;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }

  /* Check */
  assert( (-1 < border_side) && (border_side < 4));
  /* Make sure this is IO */
  assert(NULL != grid_type_descriptor);
  assert(IO_TYPE == grid_type_descriptor);

  /* Count the number of pins */
  side_pin_index = 0;
  //for (iz = 0; iz < capacity; iz++) {
    for (iheight = 0; iheight < grid_type_descriptor->height; iheight++) {
      for (ipin = 0; ipin < grid_type_descriptor->num_pins; ipin++) {
        if (1 == grid_type_descriptor->pinloc[iheight][border_side][ipin]) {
          /* Add comma if needed */
          if (1 == first_dump) {
            first_dump = 0;
          } else { 
            if (TRUE == dump_port_type) {
              fprintf(fp, ",\n");
            } else {
              fprintf(fp, ",\n");
            }
          }
          /* Determine this pin is an input or output */
          if (TRUE == dump_port_type) {
            class_id = grid_type_descriptor->pin_class[ipin];
            switch (grid_type_descriptor->class_inf[class_id].type) {
            case RECEIVER:
              fprintf(fp, "input ");
              break;
            case DRIVER:
              fprintf(fp, "output ");
              break;
            default:
              vpr_printf(TIO_MESSAGE_ERROR, "(File:%s, [LINE%d])Invalid pin_class_type!\n",
                         __FILE__, __LINE__);
              exit(1);
            }
          }
          /* This pin appear at this side! */
          fprintf(fp, " %s_height_%d__pin_%d_", 
                  convert_side_index_to_string(border_side), iheight, ipin);
          /* Update counter */
          num_dumped_port++;
          side_pin_index++;
        }
      }  
    }
  //}
  
  if ((0 < num_dumped_port)&&(TRUE == dump_last_comma)) {
    if (TRUE == dump_port_type) {
      fprintf(fp, ",\n");
    } else {
      fprintf(fp, ",\n");
    }
  }

  return;
} 

/* Physical mode subckt name */
char* compact_verilog_get_grid_phy_block_subckt_name(t_type_ptr grid_type_descriptor,
                                                     int z,
                                                     char* subckt_prefix) {
  char* ret = NULL;
  char* formatted_subckt_prefix = format_verilog_node_prefix(subckt_prefix);
  int phy_mode_index = 0;

  /* Check */
  assert(NULL != grid_type_descriptor);

  /* This a NULL logic block... Find the idle mode*/
  phy_mode_index = find_pb_type_physical_mode_index(*(grid_type_descriptor->pb_type)); 
  assert(-1 < phy_mode_index);

  ret = (char*)my_malloc(sizeof(char)* 
             (strlen(formatted_subckt_prefix) + strlen(grid_type_descriptor->name) + 1
             + strlen(my_itoa(z)) + 7 + strlen(grid_type_descriptor->pb_type->modes[phy_mode_index].name) + 1 + 1)); 
  sprintf(ret, "%s%s_%d__mode_%s_", formatted_subckt_prefix,
          grid_type_descriptor->name, z, grid_type_descriptor->pb_type->modes[phy_mode_index].name);

  return ret;
}                        

/* Print the pins of grid subblocks */
void dump_compact_verilog_io_grid_block_subckt_pins(FILE* fp,
                                                    t_type_ptr grid_type_descriptor,
                                                    int border_side,
                                                    int z) {
  int iport, ipin, dump_pin_cnt;
  int grid_pin_index, pin_height, side_pin_index;
  t_pb_graph_node* top_pb_graph_node = NULL;

  /* Check the file handler*/ 
  if (NULL == fp) {
    vpr_printf(TIO_MESSAGE_ERROR,"(File:%s,[LINE%d])Invalid file handler.\n", 
               __FILE__, __LINE__); 
    exit(1);
  }
  /* Check */
  assert(NULL != grid_type_descriptor);
  top_pb_graph_node = grid_type_descriptor->pb_graph_head;
  assert(NULL != top_pb_graph_node); 

  /* Make sure this is IO */
  assert(IO_TYPE == grid_type_descriptor);

  /* identify the location of IO grid and 
   * decide which side of ports we need
   */

  dump_pin_cnt = 0;

  for (iport = 0; iport < top_pb_graph_node->num_input_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_input_pins[iport]; ipin++) {
      grid_pin_index = top_pb_graph_node->input_pins[iport][ipin].pin_count_in_cluster 
                     + z * grid_type_descriptor->num_pins / grid_type_descriptor->capacity;
      /* num_pins/capacity = the number of pins that each type_descriptor has.
       * Capacity defines the number of type_descriptors in each grid
       * so the pin index at grid level = pin_index_in_type_descriptor 
       *                                + type_descriptor_index_in_capacity * num_pins_per_type_descriptor
       */
      pin_height = grid_type_descriptor->pin_height[grid_pin_index];
      if (1 == grid_type_descriptor->pinloc[pin_height][border_side][grid_pin_index]) {
        /* This pin appear at this side! */
        if (0 < dump_pin_cnt) {
          fprintf(fp, ",\n");
        }
        fprintf(fp, "%s_height_%d__pin_%d_", 
                convert_side_index_to_string(border_side), pin_height, grid_pin_index);
        side_pin_index++;
        dump_pin_cnt++;
      }
    }
  }

  for (iport = 0; iport < top_pb_graph_node->num_output_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_output_pins[iport]; ipin++) {
      grid_pin_index = top_pb_graph_node->output_pins[iport][ipin].pin_count_in_cluster 
                     + z * grid_type_descriptor->num_pins / grid_type_descriptor->capacity;
      /* num_pins/capacity = the number of pins that each type_descriptor has.
       * Capacity defines the number of type_descriptors in each grid
       * so the pin index at grid level = pin_index_in_type_descriptor 
       *                                + type_descriptor_index_in_capacity * num_pins_per_type_descriptor
       */
      pin_height = grid_type_descriptor->pin_height[grid_pin_index];
      if (1 == grid_type_descriptor->pinloc[pin_height][border_side][grid_pin_index]) {
        /* This pin appear at this side! */
        if (0 < dump_pin_cnt) {
          fprintf(fp, ",\n");
        }
        fprintf(fp, "%s_height_%d__pin_%d_", 
                convert_side_index_to_string(border_side), pin_height, grid_pin_index);
        side_pin_index++;
        dump_pin_cnt++;
      }
    }
  }

  for (iport = 0; iport < top_pb_graph_node->num_clock_ports; iport++) {
    for (ipin = 0; ipin < top_pb_graph_node->num_clock_pins[iport]; ipin++) {
      grid_pin_index = top_pb_graph_node->clock_pins[iport][ipin].pin_count_in_cluster 
                     + z * grid_type_descriptor->num_pins / grid_type_descriptor->capacity;
      /* num_pins/capacity = the number of pins that each type_descriptor has.
       * Capacity defines the number of type_descriptors in each grid
       * so the pin index at grid level = pin_index_in_type_descriptor 
       *                                + type_descriptor_index_in_capacity * num_pins_per_type_descriptor
       */
      pin_height = grid_type_descriptor->pin_height[grid_pin_index];
      if (1 == grid_type_descriptor->pinloc[pin_height][border_side][grid_pin_index]) {
        /* This pin appear at this side! */
        if (0 < dump_pin_cnt) {
          fprintf(fp, ",\n");
        }
        fprintf(fp, "%s_height_%d__pin_%d_", 
                convert_side_index_to_string(border_side), pin_height, grid_pin_index);
        side_pin_index++;
        dump_pin_cnt++;
      }
    }
  }

  return;
}


