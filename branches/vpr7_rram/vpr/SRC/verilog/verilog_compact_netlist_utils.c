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
#include "verilog_lut.h"
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
    for (iheight = 0; iheight < type_descriptor->height; iheight++) {
      for (ipin = 0; ipin < type_descriptor->num_pins; ipin++) {
        if (1 == type_descriptor->pinloc[iheight][border_side][ipin]) {
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
            class_id = type_descriptor->pin_class[ipin];
            switch (type_descriptor->class_inf[class_id].type) {
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


