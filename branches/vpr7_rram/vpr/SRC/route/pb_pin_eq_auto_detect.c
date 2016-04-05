/* Xifan TANG: Auto detect the pin equivalence of type_descriptor
 * by re-construct the class of pins in type_descriptor
 */
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "vpr_types.h"
#include "physical_types.h"
#include "globals.h"
#include "vpr_utils.h"

/***** Subroutines *****/
int one_type_pin_equivalence_auto_detect(t_type_descriptor* cur_type);

/*  Top-level function */
void types_pin_equivalence_auto_detect() {
  int itype = 0;

  for (itype = 0; itype < num_types; itype++) {
    /* Look into this type */
    if (1 == one_type_pin_equivalence_auto_detect(&type_descriptors[itype])) {
      vpr_printf(TIO_MESSAGE_INFO, "Pin equivalent auto-detection: type %d (name=%s) pin classes are re-built.\n",
                 itype, type_descriptors[itype].name);
    }
  }


  return;
}

int one_type_pin_equivalence_auto_detect(t_type_descriptor* cur_type) {
  t_pb_graph_node* pb_graph_head;
  int iport, ipin, iedge;
  int ret = 0;

  if (NULL == cur_type) {
    return ret;
  }

  /* Check each port if their auto_detect option is on*/
  if (TRUE == cur_type->output_ports_eq_auto_detect) {
    /* Re-built the DRIVER classes */
    pb_graph_head = cur_type->pb_graph_head; 
    /* Trace back to the inputs of these output pins, 
     * If two output pins has the same inputs, they are equivalent and belong to the same class
     */

    ret = 1;
  }

  if (TRUE == cur_type->input_ports_eq_auto_detect) {
    /* Re-built the RECEIVER classes */
    pb_graph_head = cur_type->pb_graph_head; 
    /* Forward to the outputs of these input pins, 
     * If two input pins has the same outputs, they are equivalent and belong to the same class
     */

    ret = 1;
  }

  return ret;
}
