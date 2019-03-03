/**
 VPR is a CAD tool used to conduct FPGA architecture exploration.  It takes, as input, a technology-mapped netlist and a description of the FPGA architecture being investigated.  
 VPR then generates a packed, placed, and routed FPGA (in .net, .place, and .route files respectively) that implements the input netlist.
 
 This file is where VPR starts execution.

 Key files in VPR:
 1.  libarchfpga/physical_types.h - Data structures that define the properties of the FPGA architecture
 2.  vpr_types.h - Very major file that defines the core data structures used in VPR.  This includes detailed architecture information, user netlist data structures, and data structures that describe the mapping between those two.
 3.  globals.h - Defines the global variables used by VPR.
 */


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "vpr_api.h"

#include "interactive_shell.h" 

int main(int argc, char ** argv) {
  
  /* Parse the options and decide which interface to go */
  /* Interface 1: run through -c, --command */ 
  /* Interface 2: run through -f, --file */
  /* Interface 3: run through -i, --interactive */

  /* Start the interactive shell */
  vpr_launch_interactive_shell();
}


