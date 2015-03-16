#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "util.h"
#include "mpack_types.h"
#include "global.h"
#include "mpack_conf.h"
#include "blearch_parser.h"
#include "mpack_util.h"
#include "check_ble_arch.h"
#include "print_vpr_arch.h"

/*Print mux inputs list*/
int print_vpr7_arch_mux_inputs(t_ble_arch* ble_arch,
                               t_bleport* port,
                               FILE* farch,
                               int verbose)
{
  int icnd;
 
  /*Print inputs*/
  for (icnd=0; icnd<port->cand_num; icnd++) {
    if (BLE_IN == port->cands[icnd]->type) {
      fprintf(farch,"%s.I[%d] ",ble_arch->name,port->cands[icnd]->idx);
    }
    else if (BLE_ND == port->cands[icnd]->type) {
      fprintf(farch,"cell%d.O ",port->cands[icnd]->blend_idx);
    }
    else if (BLE_OUT == port->cands[icnd]->type) {
      fprintf(farch,"follower.O[%d] ",port->cands[icnd]->idx);
    }
    else {
      printf("Error: (output_vpr7_arch_print_mux_inputs)Invaild Port Type!\n");
    }
  }

  return 1;
}

/*Print interconnections for a port inside CLB*/
int print_vpr7_arch_comb_part_inputs(t_ble_arch* ble_arch,
                                     t_bleport* port,
                                     FILE* farch,
                                     t_delay_info delay_info,
                                     int verbose)
{
  int im,ip;
 
  for (im = 0; im<ble_arch->blend_num; im++) {
    for (ip = 0; ip<ble_arch->blends[im].input_num; ip++) {
      if (1 == ble_arch->blends[im].inputs[ip].cand_num) {
       /*Print head*/
       fprintf(farch,"         <direct name=\"cell%d_I%d_mux\" input=\"",im,ip);
  
       print_vpr7_arch_mux_inputs(ble_arch,&ble_arch->blends[im].inputs[ip],farch,verbose);
       fprintf(farch,"\" output=\"cell%d.I[%d]\"/>\n",im,ip);
      }
    }
  }

  for (im = 0; im<ble_arch->blend_num; im++) {
    for (ip = 0; ip<ble_arch->blends[im].input_num; ip++) {
      if (1 < ble_arch->blends[im].inputs[ip].cand_num) {
        fprintf(farch,"        <complete name=\"cell%d_I%d_mux\" input=\"",im,ip);
  
        print_vpr7_arch_mux_inputs(ble_arch,&ble_arch->blends[im].inputs[ip],farch,verbose);
        fprintf(farch,"\" output=\"cell%d.I[%d]\">\n",im,ip);
        /*Print delay information*/
        fprintf(farch,"          <delay_constant max=\"%.4g\" in_port=\"",delay_info.inner_cluster_delay); 
        print_vpr7_arch_mux_inputs(ble_arch,&ble_arch->blends[im].inputs[ip],farch,verbose);
        fprintf(farch,"\" out_port=\"cell%d.I[%d]\"/>\n",im,ip);
        /*Print Tail*/
        fprintf(farch,"        </complete>\n");
      }
    }
  }

  return 1;
}

/*This function print the output MUXes for combinational cells*/
int print_vpr7_arch_comb_part_outputs(t_ble_arch* ble_arch,
                                      FILE* farch,
                                      int verbose)
{
  int i;

  for (i = 0; i<ble_arch->output_num; i++) {
    fprintf(farch,"        <complete name=\"comb_out%d_mux\" input=\"",i);
    print_vpr7_arch_mux_inputs(ble_arch,&ble_arch->outputs[i],farch,verbose);
    fprintf(farch,"\" output=\"follower.I[%d]\">\n",i);
    if (1 < ble_arch->outputs[i].cand_num) {
      /*Print delay information*/
      fprintf(farch,"          <delay_constant max=\"2.5e-11\" in_port=\""); 
      print_vpr7_arch_mux_inputs(ble_arch,&ble_arch->outputs[i],farch,verbose);
      fprintf(farch,"\" out_port=\"follower.I[%d]\"/>\n",i);
    }
    /*Print Tail*/
    fprintf(farch,"        </complete>\n");
  }
  
  return 1;
}

/*Print the interconnect for followers*/
int print_vpr7_arch_follower_interconnect(t_ble_arch* ble_arch,
                                          FILE* farch,
                                          int verbose)
{
  int im;
  int ff_cur = 0;
  int ff_num = sum_ff_num(ble_arch,verbose);

  /*Print Interconnections*/
  fprintf(farch,"        <interconnect>\n");
  fprintf(farch,"          <complete name=\"follower_clks\" input=\"follower.clk\" output=\"ff[%d:0].clk\"/>\n",ff_num-1);
  /*Print Direct for combinational followers and DFFs.D*/
  for (im = 0; im<ble_arch->output_num; im++) {
    if (1 == ble_arch->outputs[im].is_comb) {
      fprintf(farch,"          <direct name=\"follower_in[%d]_mux\" input=\"follower.I[%d]\" output=\"follower.O[%d]\"/>\n",im,im,im);
    }
    else {
      fprintf(farch,"          <direct name=\"follower_in[%d]_mux\" input=\"follower.I[%d]\" output=\"ff[%d].D\"/>\n",im,im,ff_cur);
      ff_cur++;
    }
  }

  /*Print MUXes for combinational followers*/
  ff_cur = 0;
  for (im = 0; im<ble_arch->output_num; im++) {
    if (0 == ble_arch->outputs[im].is_comb) {
      fprintf(farch,"          <mux name=\"ff[%d]_mux\" input=\"follower.I[%d] ff[%d].Q\" output=\"follower.O[%d]\">\n",ff_cur,im,ff_cur,im);
      /*Print delay information*/
      fprintf(farch,"            <delay_constant max=\"2.5e-11\" in_port=\"follower.I[%d] ff[%d].Q\" out_port=\"follower.O[%d]\"/>\n",im,ff_cur,im);
      /*Print Tail*/
      fprintf(farch,"          </mux>\n");
      ff_cur++;
    }
  }

  fprintf(farch,"        </interconnect>\n");

  return 1;
}

/*Print Followers: DFFs and combinational outputs*/
int print_vpr7_arch_followers(t_ble_arch* ble_arch,
                              FILE* farch,
                              t_delay_info delay_info,
                              int verbose)
{
  int ff_num = sum_ff_num(ble_arch,verbose);
  
  /*Print Followers definition*/
  fprintf(farch,"      <pb_type name=\"follower\" num_pb=\"1\">\n");
  fprintf(farch,"        <input name=\"I\" num_pins=\"%d\"/>\n",ble_arch->output_num);
  fprintf(farch,"        <output name=\"O\" num_pins=\"%d\"/>\n",ble_arch->output_num);
  fprintf(farch,"        <clock name=\"clk\" num_pins=\"1\"/>\n");

  /*Print DFFs Definitions*/
  fprintf(farch,"        <pb_type name=\"ff\" blif_model=\".latch\" num_pb=\"%d\" class=\"flipflop\">\n",ff_num);
  fprintf(farch,"          <input name=\"D\" num_pins=\"1\" port_class=\"D\"/>\n");
  fprintf(farch,"          <output name=\"Q\" num_pins=\"1\" port_class=\"Q\"/>\n");
  fprintf(farch,"          <clock name=\"clk\" num_pins=\"1\" port_class=\"clock\"/>\n");
  fprintf(farch,"          <T_setup value=\"%.4g\" port=\"ff.D\" clock=\"clk\"/>\n",delay_info.ff_tsu);
  fprintf(farch,"          <T_clock_to_Q max=\"%.4g\" port=\"ff.Q\" clock=\"clk\"/>\n",delay_info.ff_delay);
  fprintf(farch,"        </pb_type>\n");
  
  print_vpr7_arch_follower_interconnect(ble_arch,farch,verbose);

  fprintf(farch,"      </pb_type>\n");

  return 1;
}


/*Print a VPR7 version Architrcture XML*/
int print_vpr7_arch(t_ble_arch* ble_arch,
                    char* arch_file,
                    t_delay_info delay_info,
                    int verbose)
{
  int im, ip;
  FILE* farch;

  printf("Output VPR7 Architecture XML(%s)...\n",arch_file);
  farch = my_fopen(arch_file,"w");
  /*Print Head*/
  fputs("<!--VPR7 Architecture XML " __DATE__ "-->\n",farch); 
  fprintf(farch,"<!--Author: Xifan TANG-->\n");

  fprintf(farch,"<architecture>\n");
  /*Print models*/ 
  fprintf(farch,"  <models>\n");
  for (im = 0; im<ble_arch->blend_num; im++) {
    fprintf(farch,"    <model name=\"cell%d\">\n",im);
    fprintf(farch,"      <input_ports>\n");
    fprintf(farch,"        <port name=\"I\"/>\n");
    fprintf(farch,"      </input_ports>\n");
    fprintf(farch,"      <output_ports>\n");
    fprintf(farch,"        <port name=\"O\"/>\n");
    fprintf(farch,"      </output_ports>\n");
    fprintf(farch,"    </model>\n");
  }
  fprintf(farch,"  </models>\n");

  /*Print layouts*/
  fprintf(farch,"  <layout auto=\"1.0\"/>\n");

  /*Print device*/
  fprintf(farch,"  <device>\n");
  fprintf(farch,"    <sizing R_minW_nmos=\"8926\" R_minW_pmos=\"16067\" ipin_mux_trans_size=\"1.22226\"/>\n");
  fprintf(farch,"    <timing C_ipin_cblock=\"1.47e-15\" T_ipin_cblock=\"7.247e-11\"/>\n");
  fprintf(farch,"    <area grid_logic_tile_area=\"%.4g\"/>\n",ble_arch->area);
  fprintf(farch,"    <sram area=\"6\"/>\n");
  fprintf(farch,"    <chan_width_distr>\n");
  fprintf(farch,"      <io width=\"1.0\"/>\n");
  fprintf(farch,"      <x distr=\"uniform\" peak=\"1.0\"/>\n");
  fprintf(farch,"      <y distr=\"uniform\" peak=\"1.0\"/>\n");
  fprintf(farch,"    </chan_width_distr>\n");
  fprintf(farch,"    <switch_block type=\"wilton\" fs=\"3\"/>\n");
  fprintf(farch,"  </device>\n");

  /*Print Switchlist*/
  fprintf(farch,"  <switchlist>\n");
  fprintf(farch,"    <switch type=\"mux\" name=\"0\" R=\"551\" Cin=\"7.7e-16\" Cout=\"3.488e-15\" Tdel=\"5.8e-11\" mux_trans_size=\"2.63074\" buf_size=\"27.645901\"/>\n");
  fprintf(farch,"  </switchlist>\n");

  /*Print segmentlist*/
  fprintf(farch,"  <segmentlist>\n");
  fprintf(farch,"    <segment freq=\"1.0\" length=\"4\" type=\"unidir\" Rmetal=\"%.4g\" Cmetal=\"%.4g\">\n",ble_arch->rmetal,ble_arch->cmetal);
  fprintf(farch,"      <mux name=\"0\"/>\n");
  fprintf(farch,"      <sb type=\"pattern\">1 1 1 1 1</sb>\n");
  fprintf(farch,"      <cb type=\"pattern\">1 1 1 1</cb>\n");
  fprintf(farch,"    </segment>\n");
  fprintf(farch,"  </segmentlist>\n");
 
  /*Print Complex Blocks*/
  fprintf(farch,"  <complexblocklist>\n");
  /*Print IO pads*/
  fprintf(farch,"    <pb_type name=\"io\" capacity=\"%d\">\n",(int)(2*sqrt(ble_arch->blend_num)));
  fprintf(farch,"      <input name=\"outpad\" num_pins=\"1\"/>\n");
  fprintf(farch,"      <output name=\"inpad\" num_pins=\"1\"/>\n");
  fprintf(farch,"      <clock name=\"clock\" num_pins=\"1\"/>\n");
  fprintf(farch,"      <mode name=\"inpad\">\n");
  fprintf(farch,"        <pb_type name=\"inpad\" blif_model=\".input\" num_pb=\"1\">\n");
  fprintf(farch,"          <output name=\"inpad\" num_pins=\"1\"/>\n");
  fprintf(farch,"        </pb_type>\n");
  fprintf(farch,"        <interconnect>\n");
  fprintf(farch,"          <direct name=\"inpad\" input=\"inpad.inpad\" output=\"io.inpad\">\n");
  fprintf(farch,"            <delay_constant max=\"%.4g\" in_port=\"inpad.inpad\" out_port=\"io.inpad\"/>\n",delay_info.inpad_delay);
  fprintf(farch,"          </direct>\n");
  fprintf(farch,"        </interconnect>\n");
  fprintf(farch,"      </mode>\n");
  fprintf(farch,"      <mode name=\"outpad\">\n");
  fprintf(farch,"        <pb_type name=\"outpad\" blif_model=\".output\" num_pb=\"1\">\n");
  fprintf(farch,"          <input name=\"outpad\" num_pins=\"1\"/>\n");
  fprintf(farch,"        </pb_type>\n");
  fprintf(farch,"        <interconnect>\n");
  fprintf(farch,"          <direct name=\"outpad\" input=\"io.outpad\" output=\"outpad.outpad\">\n");
  fprintf(farch,"            <delay_constant max=\"%.4g\" in_port=\"io.outpad\" out_port=\"outpad.outpad\"/>\n",delay_info.outpad_delay);
  fprintf(farch,"          </direct>\n");
  fprintf(farch,"        </interconnect>\n");
  fprintf(farch,"      </mode>\n");
  fprintf(farch,"      <fc default_in_type=\"frac\" default_in_val=\"0.15\" default_out_type=\"frac\" default_out_val=\"0.10\"/>\n");
  fprintf(farch,"      <pinlocations pattern=\"custom\">\n");
  fprintf(farch,"        <loc side=\"left\">io.outpad io.inpad io.clock</loc>\n");
  fprintf(farch,"        <loc side=\"top\">io.outpad io.inpad io.clock</loc>\n");
  fprintf(farch,"        <loc side=\"right\">io.outpad io.inpad io.clock</loc>\n");
  fprintf(farch,"        <loc side=\"bottom\">io.outpad io.inpad io.clock</loc>\n");
  fprintf(farch,"      </pinlocations>\n");
  fprintf(farch,"      <gridlocations>\n");
  fprintf(farch,"        <loc type=\"perimeter\" priority=\"10\"/>\n");
  fprintf(farch,"      </gridlocations>\n");
  fprintf(farch,"    </pb_type>\n");

  /*Print MPACK CLB Architecture*/
  fprintf(farch,"    <pb_type name=\"%s\">\n",ble_arch->name);
  fprintf(farch,"      <input name=\"I\" num_pins=\"%d\" equivalent=\"true\"/>\n",ble_arch->input_num);
  fprintf(farch,"      <output name=\"O\" num_pins=\"%d\" equivalent=\"false\"/>\n",ble_arch->output_num);
  fprintf(farch,"      <clock name=\"clk\" num_pins=\"1\"/>\n");
  /*Print Combinational Parts*/
  /*Print cells inside Combinational Parts*/
  for (im = 0; im<ble_arch->blend_num; im++) {
    fprintf(farch,"      <pb_type name=\"cell%d\" blif_model=\".subckt cell%d\" num_pb=\"1\">\n",im,im);
    fprintf(farch,"        <input name=\"I\" num_pins=\"%d\"/>\n",ble_arch->blends[im].input_num);
    fprintf(farch,"        <output name=\"O\" num_pins=\"1\"/>\n");
    fprintf(farch,"        <delay_matrix type=\"max\" in_port=\"cell%d.I\" out_port=\"cell%d.O\">\n",im,im);
    for (ip=0; ip < ble_arch->blends[im].input_num; ip++) {
    fprintf(farch,"          %.4g\n",delay_info.gate_delay);
    }
    fprintf(farch,"        </delay_matrix>\n");
    fprintf(farch,"      </pb_type>\n");
  }
  fprintf(farch,"\n");

  /*Print Followers: DFFs and combinatonal output*/
  print_vpr7_arch_followers(ble_arch,farch,delay_info,verbose);

  /*Print interconnections for Top CLBs*/
  fprintf(farch,"      <interconnect>\n");
  fprintf(farch,"        <direct name=\"%s_clks\" input=\"%s.clk\" output=\"follower.clk\"/>\n",ble_arch->name,ble_arch->name);
  fprintf(farch,"        <direct name=\"follower_output\" input=\"follower.O\" output=\"%s.O\"/>\n",ble_arch->name);
  
  
  print_vpr7_arch_comb_part_inputs(ble_arch,&ble_arch->blends[im].inputs[ip],farch,delay_info,verbose);

  print_vpr7_arch_comb_part_outputs(ble_arch,farch,verbose);

  fprintf(farch,"      </interconnect>\n");
  fprintf(farch,"\n");

  fprintf(farch,"      <fc default_in_type=\"frac\" default_in_val=\"0.15\" default_out_type=\"frac\" default_out_val=\"0.10\"/>\n");
  fprintf(farch,"      <pinlocations pattern=\"spread\"/>\n");
  fprintf(farch,"      <gridlocations>\n");
  fprintf(farch,"        <loc type=\"fill\" priority=\"1\"/>\n");
  fprintf(farch,"      </gridlocations>\n");
  fprintf(farch,"    </pb_type>\n");
  fprintf(farch,"  </complexblocklist>\n");
  fprintf(farch,"</architecture>\n");

  fprintf(farch,"\n");

  return 1;
}
