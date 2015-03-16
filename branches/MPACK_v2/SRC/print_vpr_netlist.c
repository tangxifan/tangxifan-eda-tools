#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "mpack_types.h"
#include "mpack_conf.h"
#include "blearch_parser.h"
#include "mpack_util.h"
#include "print_vpr_netlist.h"

int print_block_inputs(FILE* fnet,
                       t_ble_info* ble_info,
                       int verbose)
{
  int i;

  /*Print Block Inputs*/
  fprintf(fnet,"    <inputs>\n");
  fprintf(fnet,"      <port name=\"I\">");
  for (i = 0; i<ble_info->input_num; i++) {
    if (NULL != ble_info->input_lgknds[i]) {
      fprintf(fnet,"%s ",ble_info->input_lgknds[i]->inputs[ble_info->input_lgknds_input_idx[i]]->net_name);
    }
    else {
      fprintf(fnet,"open ");
    }
  }
  fprintf(fnet,"</port>\n");
  fprintf(fnet,"    </inputs>\n");

  return 1;
}

int print_block_mux_input(FILE* fnet,
                          t_ble_arch* ble_arch,
                          t_ble_info* ble_info,
                          t_bleport* port,
                          int verbose)
{
  int mux_path;
  
  /*Determine the Path that MUX is open*/
  if (BLE_ND == port->type) {
    mux_path = ble_info->blend_muxes[port->blend_idx][port->idx];
  }
  else if (BLE_OUT == port->type) {
    mux_path = ble_info->output_muxes[port->idx];
  }
  else {
    printf("Error:(print_block_mux_input)Invalid port type!\n");
    exit(1);
  }

  /*Print the mux ports driver*/
  if (BLE_ND == port->cands[mux_path]->type) {
    fprintf(fnet,"cell%d.O",port->cands[mux_path]->blend_idx);
  }
  else if (BLE_OUT == port->cands[mux_path]->type) {
    fprintf(fnet,"follower.O[%d]",port->cands[mux_path]->idx);
  }
  else if (BLE_IN == port->cands[mux_path]->type) {
	fprintf(fnet,"%s.I[%d]",ble_arch->name,port->cands[mux_path]->idx);
  }
 
  return 1;   
}

int print_block_outputs(FILE* fnet,
                        t_ble_arch* ble_arch,
                        t_ble_info* ble_info,
                        int verbose)
{
  int i;

  /*Print Block Outputs*/
  fprintf(fnet,"    <outputs>\n");
  fprintf(fnet,"      <port name=\"O\">");
  for (i = 0; i<ble_info->output_num; i++) {
    /* In what case, the block output will be set as "open"?
     * 1. Output Un-used.(None logic node is mapped) 
     * 2. Output net is fully absorbed by the block.It has no connection with nodes outside block
     */
    if ((NULL != ble_info->output_lgknds[i])&&(0 == check_ble_absorb_lgknd_output(ble_info,ble_info->output_lgknds[i],verbose))) {
      fprintf(fnet,"follower.O[%d]->follower_output ",i);
    }
    else {
      fprintf(fnet,"open ");
    }
  }
  fprintf(fnet,"</port>\n");
  fprintf(fnet,"    </outputs>\n");

  return 1;
}

/* Print the Netlist of Block cells,
 * Including input connections, and outputs
 * NOTICE: outputs should include Net name
 */
int print_block_cells(FILE* fnet,
                      t_ble_arch* ble_arch,
                      t_ble_info* ble_info,
                      int verbose)
{
  int i,j;

  for (i = 0; i<ble_info->blend_num; i++) {
    /*Print open for unused block*/
    if (NULL == ble_info->blend_lgknds[i]) {
      fprintf(fnet,"    <block name=\"open\" instance=\"cell%d[0]\"/>\n",i);
    }
    else {
      fprintf(fnet,"    <block name=\"%s\" instance=\"cell%d[0]\">\n",ble_info->blend_lgknds[i]->net_name,i);
      /*Print inputs*/
      fprintf(fnet,"      <inputs>\n");
      fprintf(fnet,"        <port name=\"I\">\n");
      fprintf(fnet,"          ");
      for (j = 0; j<ble_arch->blends[i].input_num; j++) {
        if (0 == ble_info->blend_port_used[i][j]) {
          fprintf(fnet,"open ");
        }
        else {
          print_block_mux_input(fnet,ble_arch,ble_info,&ble_arch->blends[i].inputs[j],verbose);
          fprintf(fnet,"->cell%d_I%d_mux ",i,j);
        }
      }
      fprintf(fnet,"\n");
      fprintf(fnet,"        </port>\n");
      fprintf(fnet,"      </inputs>\n");
      /*Print outputs*/
      fprintf(fnet,"      <outputs>\n");
      fprintf(fnet,"        <port name=\"O\">");
      fprintf(fnet,"%s",ble_info->blend_lgknds[i]->net_name);
      fprintf(fnet,"</port>\n");
      fprintf(fnet,"      </outputs>\n");
      fprintf(fnet,"      <clocks>\n");
      fprintf(fnet,"      </clocks>\n");
      fprintf(fnet,"    </block>\n");
    }
  } 

  return 1;
}

/*Print latches inside Block*/
int print_block_latches(FILE* fnet,
                        t_ble_arch* ble_arch,
                        t_ble_info* ble_info,
                        int verbose)
{
  int i, ff_cur;

  ff_cur = 0;
 
  /*Print Follower Blocks*/
  //fprintf(fnet,"      <block name=\"open\" instance=\"follower[0]\" mode=\"follower\">\n");
  fprintf(fnet,"      <block name=\"follower%d\" instance=\"follower[0]\" mode=\"follower\">\n",ble_info->idx);

  /*Print Follwer Input port*/
  fprintf(fnet,"        <inputs>\n");
  fprintf(fnet,"          <port name=\"I\">\n");
  fprintf(fnet,"            ");

  for (i = 0; i<ble_arch->output_num; i++) {
    if (NULL == ble_info->output_lgknds[i]) {
      fprintf(fnet,"open ");
    }
    else {
      if ((0 == check_bleport_conkt(ble_arch,ble_info,&ble_arch->outputs[i],verbose)) 
      &&(1 == check_ble_absorb_lgknd_output(ble_info,ble_info->output_lgknds[i],verbose))) {
      //&&(ND_LATCH == ble_info->output_lgknds[i]->type))) {
      // For combinational fanouts, absorbed could be through local multiplexer
        fprintf(fnet,"open ");
      }
      else { 
        print_block_mux_input(fnet,ble_arch,ble_info,&ble_arch->outputs[i],verbose);
        fprintf(fnet,"->comb_out%d_mux ",i);
      }
    }
  }
  fprintf(fnet,"\n");

  fprintf(fnet,"          </port>\n");
  fprintf(fnet,"        </inputs>\n");

  /*Print Follwer Output port*/
  fprintf(fnet,"        <outputs>\n");
  fprintf(fnet,"          <port name=\"O\">\n");
  fprintf(fnet,"            ");

  for (i = 0; i<ble_arch->output_num; i++) {
    /*Print open blocks if they are unused*/
    if (NULL == ble_info->output_lgknds[i]) {
      fprintf(fnet,"open ");       
    }
    else {
      if ((0 == check_bleport_conkt(ble_arch,ble_info,&ble_arch->outputs[i],verbose))
      &&(1 == check_ble_absorb_lgknd_output(ble_info,ble_info->output_lgknds[i],verbose))) {
      //&&(ND_LATCH == ble_info->output_lgknds[i]->type)) { 
      // For combinational fanouts, absorbed could be through local multiplexer
        fprintf(fnet,"open ");
      }
      else {
        /*Check outputs*/
        if (1 == ble_arch->outputs[i].is_comb) { 
          /*Comb-only Outputs*/
          fprintf(fnet,"follower.I[%d]->follower_in[%d]_mux ",i,i);
        }
        else {
          if (ND_LATCH == ble_info->output_lgknds[i]->type) {
            fprintf(fnet,"ff[%d].Q->ff[%d]_mux ",ff_cur,ff_cur);
          }
          else {
            fprintf(fnet,"follower.I[%d]->ff[%d]_mux ",i,ff_cur);
          }
        }
      }
    }
    if (0 == ble_arch->outputs[i].is_comb) {
      ff_cur++;
    }
  }
  fprintf(fnet,"\n");

  fprintf(fnet,"          </port>\n");
  fprintf(fnet,"        </outputs>\n");

  /*Print clocks*/
  fprintf(fnet,"       <clocks>\n");
  for (i = 0; i<ble_info->output_num; i++) {
    if ((NULL != ble_info->output_lgknds[i])&&(ND_LATCH == ble_info->output_lgknds[i]->type)) {
     fprintf(fnet,"         <port name=\"clk\">%s.clk[0]->%s_clks</port>\n",ble_arch->name,ble_arch->name);
     break;
    }
  }
  fprintf(fnet,"       </clocks>\n");

  fprintf(fnet,"\n");

  /*Print Sub Blocks: DFFs*/
  ff_cur = 0;
  for (i = 0; i<ble_arch->output_num; i++) {
    /*Check latch-capable outputs only*/
    if (0 == ble_arch->outputs[i].is_comb) {
      if ((NULL == ble_info->output_lgknds[i])||(ND_LATCH != ble_info->output_lgknds[i]->type)) {
        /*Print open blocks if they are unused or used as combinational*/
        fprintf(fnet,"       <block name=\"open\" instance=\"ff[%d]\"/>\n",ff_cur);
      }
      else if (ND_LATCH == ble_info->output_lgknds[i]->type) {
        fprintf(fnet,"       <block name=\"%s\" instance=\"ff[%d]\">\n",ble_info->output_lgknds[i]->net_name,ff_cur);
        /*Print Inputs*/
        fprintf(fnet,"         <inputs>\n");
        fprintf(fnet,"           <port name=\"D\">");
        fprintf(fnet,"follower.I[%d]->follower_in[%d]_mux",i,i);
        fprintf(fnet,"</port>\n");
        fprintf(fnet,"         </inputs>\n");
        /*Print outputs*/
        fprintf(fnet,"         <outputs>\n");
        fprintf(fnet,"           <port name=\"Q\">");
        fprintf(fnet,"%s",ble_info->output_lgknds[i]->net_name);
        fprintf(fnet,"</port>\n");
        fprintf(fnet,"         </outputs>\n");
        /*Print clocks*/
        fprintf(fnet,"         <clocks>\n");
        fprintf(fnet,"           <port name=\"clk\">follower.clk[0]->follower_clks</port>\n");
        fprintf(fnet,"         </clocks>\n");
        fprintf(fnet,"       </block>\n");
      }
      ff_cur++;
    }
  }
  fprintf(fnet,"     </block>\n");

  return 1;
}

int print_vpr7_netlist(char* net_file,
                       t_ble_arch* ble_arch,
                       t_ble_info* ble_info,
                       t_lgkntwk* lgkntwk,
                       int verbose)
{
  int i,block_cur;
  t_ble_info* head = ble_info->next; //Skip the scout
  FILE* fnet;
  
  /*Open FILE*/
  printf("Output VPR7 Netlist(%s)...\n",net_file);
  fnet = my_fopen(net_file,"w");
  /*Print File head*/
  fputs("<!--VPR7 Netlist XML " __DATE__ "-->\n",fnet); 
  fprintf(fnet,"<!--Author: Xifan TANG-->\n");
  /*Print Top Block, Primary inputs, Primary Outputs*/ 
  fprintf(fnet,"<block name=\"%s\" instance=\"FPGA_packed_netlist[0]\">\n",net_file); 

  /*Primary Inputs printing*/
  fprintf(fnet,"  <inputs>\n");
  fprintf(fnet,"    ");
  
  /*Print Clocks if we have latch*/ 
  if (0 < lgkntwk->latch_num) {
    fprintf(fnet,"clk ");
  }
  for (i = 0; i<lgkntwk->pi_num; i++) {
    fprintf(fnet,"%s ",lgkntwk->pi_ptrs[i]->net_name);  
  }
  fprintf(fnet,"\n");
  fprintf(fnet,"  </inputs>\n");
  /*Primary Outputs printing*/
  fprintf(fnet,"  <outputs>\n");
  fprintf(fnet,"    ");
  for (i = 0; i<lgkntwk->po_num; i++) {
    fprintf(fnet,"out:%s ",lgkntwk->po_ptrs[i]->net_name);  
  }
  fprintf(fnet,"\n");
  fprintf(fnet,"  </outputs>\n");
  /*Global clock printing*/
  fprintf(fnet,"  <clocks>\n");
  fprintf(fnet,"    ");
  if (0 < lgkntwk->latch_num) {
    fprintf(fnet,"clk"); 
  }
  fprintf(fnet,"\n");
  fprintf(fnet,"  </clocks>\n");

  /*Print Blocks*/
  block_cur = 0;
  while(head) {
    fprintf(fnet,"  <block name=\"block[%d]\" instance=\"%s[%d]\" mode=\"%s\">\n",head->idx,ble_arch->name,head->idx,ble_arch->name);

    print_block_inputs(fnet,head,verbose);
    print_block_outputs(fnet,ble_arch,head,verbose);

    /*Print Clocks, if we have latch...*/
    fprintf(fnet,"    <clocks>\n");
    for (i = 0; i<head->output_num; i++) {
      if ((NULL != head->output_lgknds[i])&&(ND_LATCH == head->output_lgknds[i]->type)) {
         fprintf(fnet,"       <port name=\"clk\">clk</port>\n");
         break;
      }
    }
    fprintf(fnet,"    </clocks>\n");
    
    /*Print Cells*/
    print_block_cells(fnet,ble_arch,head,verbose);
    
    /*Print Latches*/
    print_block_latches(fnet,ble_arch,head,verbose);

    fprintf(fnet,"  </block>\n");
     
    /*Move on to next BLE*/ 
    head = head->next;
    /*Count the number of block*/
    block_cur++;
  }

  
  /*Print Input pads*/
  for (i = 0; i<lgkntwk->pi_num; i++) {
	fprintf(fnet,"  <block name=\"%s\" instance=\"io[%d]\" mode=\"inpad\">\n",lgkntwk->pi_ptrs[i]->net_name,block_cur);
	fprintf(fnet,"	  <inputs>\n");
	fprintf(fnet,"	    <port name=\"outpad\">open </port>\n");
	fprintf(fnet,"	  </inputs>\n");
    fprintf(fnet,"	  <outputs>\n");
	fprintf(fnet,"		<port name=\"inpad\">inpad[0].inpad[0]->inpad  </port>\n");
	fprintf(fnet,"	  </outputs>\n");
	fprintf(fnet,"	  <clocks>\n");
	fprintf(fnet,"		<port name=\"clock\">open </port>\n");
	fprintf(fnet,"    </clocks>\n");
	fprintf(fnet,"	  <block name=\"%s\" instance=\"inpad[0]\">\n",lgkntwk->pi_ptrs[i]->net_name);
	fprintf(fnet,"		<inputs>\n");
	fprintf(fnet,"		</inputs>\n");
	fprintf(fnet,"		<outputs>\n");
	fprintf(fnet,"		  <port name=\"inpad\">%s </port>\n",lgkntwk->pi_ptrs[i]->net_name);
	fprintf(fnet,"		</outputs>\n");
	fprintf(fnet,"		<clocks>\n");
	fprintf(fnet,"		</clocks>\n");
	fprintf(fnet,"	  </block>\n");
	fprintf(fnet,"  </block>\n");

    block_cur++;
  }

  /*Print Output pads*/
  for (i = 0; i<lgkntwk->po_num; i++) {
	fprintf(fnet,"  <block name=\"out:%s\" instance=\"io[%d]\" mode=\"outpad\">\n",lgkntwk->po_ptrs[i]->net_name,block_cur);
	fprintf(fnet,"	  <inputs>\n");
	fprintf(fnet,"		<port name=\"outpad\">%s </port>\n",lgkntwk->po_ptrs[i]->inputs[0]->net_name);
	fprintf(fnet,"	  </inputs>\n");
	fprintf(fnet,"	  <outputs>\n");
	fprintf(fnet,"		<port name=\"inpad\">open </port>\n");
	fprintf(fnet,"	  </outputs>\n");
	fprintf(fnet,"	  <clocks>\n");
	fprintf(fnet,"		<port name=\"clock\">open </port>\n");
	fprintf(fnet,"	  </clocks>\n");
	fprintf(fnet," 	  <block name=\"out:%s\" instance=\"outpad[0]\">\n",lgkntwk->po_ptrs[i]->net_name);
	fprintf(fnet,"	 	<inputs>\n");
	fprintf(fnet,"	  	  <port name=\"outpad\">io.outpad[0]->outpad  </port>\n");
	fprintf(fnet,"		</inputs>\n");
	fprintf(fnet,"		<outputs>\n");
	fprintf(fnet,"		</outputs>\n");
	fprintf(fnet,"	  	<clocks>\n");
	fprintf(fnet,"	  	</clocks>\n");
	fprintf(fnet,"	  </block>\n");
	fprintf(fnet,"  </block>\n");

    block_cur++;
  }
  
  /*We should print io:clk if we have latch...*/
  if (0 < lgkntwk->latch_num) {
	fprintf(fnet,"  <block name=\"clk\" instance=\"io[%d]\" mode=\"inpad\">\n",block_cur);
	fprintf(fnet,"	  <inputs>\n");
	fprintf(fnet,"	    <port name=\"outpad\">open </port>\n");
	fprintf(fnet,"	  </inputs>\n");
    fprintf(fnet,"	  <outputs>\n");
	fprintf(fnet,"		<port name=\"inpad\">inpad[0].inpad[0]->inpad  </port>\n");
	fprintf(fnet,"	  </outputs>\n");
	fprintf(fnet,"	  <clocks>\n");
	fprintf(fnet,"		<port name=\"clock\">open </port>\n");
	fprintf(fnet,"    </clocks>\n");
	fprintf(fnet,"	  <block name=\"clk\" instance=\"inpad[0]\">\n");
	fprintf(fnet,"		<inputs>\n");
	fprintf(fnet,"		</inputs>\n");
	fprintf(fnet,"		<outputs>\n");
	fprintf(fnet,"		  <port name=\"inpad\">clk </port>\n");
	fprintf(fnet,"		</outputs>\n");
	fprintf(fnet,"		<clocks>\n");
	fprintf(fnet,"		</clocks>\n");
	fprintf(fnet,"	  </block>\n");
	fprintf(fnet,"  </block>\n");

    block_cur++;
  }

  fprintf(fnet,"</block>\n");

  fprintf(fnet,"\n");
  
 
  return 1;
}
