#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "mpack_conf.h"
#include "mpack_types.h"
#include "blearch_parser.h"
#include "mpack_util.h"
#include "check_ble_arch.h"

/**
 * Check the BLE Architecture
 * Task List:
 * 1. Mark positions that can be configured as an inverter
 * 2. Mark fully SAT position
 */
int check_ble_arch(t_ble_arch* ble_arch)
{
  t_ble_info ble_info;
  init_ble_info(ble_arch,&ble_info);

  /*Check input number =? 3*/
  //check_cell_input_number(ble_arch);
  /*Mark all inverter-able cell*/
  mark_invs_ble(ble_arch,&ble_info,1);
  /*Mark positions that can fully satisfied*/ 
  printf("Info: Checking SAT position in BLE architecture...\n");
  mark_fully_sat_cells_ble(ble_arch,&ble_info,1);
  /*Depth BLE arch*/
  printf("Info: DFS BLE architecture...\n");
  dfs_ble_arch(ble_arch,&ble_info,1);

  return 1;
}


/**
 * Check Cells' input number
 * MPACK requires input number should be 3!
 */
int check_cell_input_number(t_ble_arch* ble_arch)
{
  int ind;

  /*Search each cell*/
  for (ind=0; ind<ble_arch->blend_num; ind++)
  {
    /*Check input number*/
    if (3 != ble_arch->blends[ind].input_num)
    {
      printf("Error: Cell[%d] has %d inputs, which illegal! (Legal: 3 inputs)\n",ind, ble_arch->blends[ind].input_num);
      exit(1);
    }
  }

  return 1;
}

/**
 * Check cella are connected to cellb's output  or not
 * Return 1 if connected
 * Return 0 if not connected
 */
int check_cells_conkted(t_blend* cella,
                        t_blend* cellb,
                        int* port_idx,
                        int* cand_idx,
                        t_ble_info* ble_info)
{
  int im;
  int icnd;

  int port_used;
  int port_mux;

  for (im=0; im<cella->input_num; im++) {
    get_port_used_mux(&cella->inputs[im],ble_info,&port_used,&port_mux);
    for (icnd=0; icnd<cella->inputs[im].cand_num; icnd++) {
      if ((0 == port_used)||((1 == port_used)&&(icnd == port_mux))) {
        if (cellb->idx == cella->inputs[im].cands[icnd]->blend_idx) {
          (*port_idx) = im;
          (*cand_idx) = icnd;
          return 1;
        }
      }      
    }
  } 

  return 0;
}

/**
 * Check Port(Cell_idx, port_idx) is connected to a cell(unused)
 * Return the solutions. Number of solutions are stored in *cell_num
 */
int* check_port_conkt_cell(t_ble_arch* ble_arch,
                          t_ble_info* ble_info,
                          int cell_idx,
                          int port_idx,
                          int* cell_num)
{
  int preidx;
  int* cells;
  int icnd; 

  /*Initial*/ 
  (*cell_num) = 0;
  /*Find the number of cell connected*/
  for (icnd=0; icnd<ble_arch->blends[cell_idx].inputs[port_idx].cand_num; icnd++) {
    if (BLE_ND == ble_arch->blends[cell_idx].inputs[port_idx].cands[icnd]->type) {
      preidx = ble_arch->blends[cell_idx].inputs[port_idx].cands[icnd]->blend_idx;
      if (0 == ble_info->blend_used[preidx]) {
        (*cell_num)++;
      }
    }
  }
  /*Allocate memory and Re-initial*/
  cells = (int*)my_malloc(sizeof(int)*(*cell_num));
  (*cell_num) = 0;
  /*Store solutions*/
  for (icnd=0; icnd<ble_arch->blends[cell_idx].inputs[port_idx].cand_num; icnd++) {
    if (BLE_ND == ble_arch->blends[cell_idx].inputs[port_idx].cands[icnd]->type) {
      preidx = ble_arch->blends[cell_idx].inputs[port_idx].cands[icnd]->blend_idx;
      if (0 == ble_info->blend_used[preidx]) {
        cells[(*cell_num)] = icnd;
        (*cell_num)++;
      }
    }
  }

  return cells;
}


/**
 * Check tree conkts
 *  leaf0 leaf1
 *     \  /
 *     root
 * Return 1 if connected
 * Return 0 if not connected
 */
int check_tree_conkts(t_blend* root,
                      t_blend* leaf0,
                      t_blend* leaf1,
                      t_ble_info* ble_info)
{
  int porta;
  int portb;
  int canda;
  int candb;

  if (check_cells_conkted(root,leaf0,&porta,&canda,ble_info)) {
    if (check_cells_conkted(root,leaf1,&portb,&candb,ble_info)) {
      if (porta != portb) {
        return 1;}
    }
  }
  
  return 0;
}

/** 
 * Check port used and muxes with given BLE info 
 * Return 1 if find
 * Return used and mux candidate in *used and *mux
 */
int get_port_used_mux(t_bleport* port,
                      t_ble_info* ble_info,
                      int* used,
                      int* mux)
{
  /*Check NULL ptr*/
  if (NULL == ble_info) {
    printf("Error: (Get_port_used_conkt)input BLE_info is NULL!\n");
    exit(1);
  }

  /*Check BLE_IN*/ 
  if (BLE_IN == port->type) {
    /*Check overflow*/
    if (port->idx > (ble_info->input_num-1)) {
      printf("Error: Port index (%d) > BLE's input number-1 (%d)!\n",port->idx, ble_info->input_num-1);
      exit(1);
    }
    *used = ble_info->input_used[port->idx];
    *mux = -1;
  }

  /*Check BLE_OUT*/ 
  if (BLE_OUT == port->type) {
    /*Check overflow*/
    if (port->idx > (ble_info->output_num-1)) {
      printf("Error: Port index (%d) > BLE's output number-1 (%d)!\n",port->idx, ble_info->output_num-1);
      exit(1);
    }
    *used = ble_info->output_used[port->idx];
    *mux = ble_info->output_muxes[port->idx];
  }

  /*Check BLE_ND*/ 
  if (BLE_ND == port->type) {
    /*Check overflow*/
    if (port->blend_idx > (ble_info->blend_num-1)) {
      printf("Error: Port's BLE_ND index (%d) > BLE's BLE_ND number-1 (%d)!\n",port->blend_idx, ble_info->blend_num-1);
      exit(1);
    }
    *mux = ble_info->blend_muxes[port->blend_idx][port->idx];
    *used = ble_info->blend_port_used[port->blend_idx][port->idx]; 
  }

  return 1;
}

/**
 * Check shared inputs
 * Return 1 if found
 * Return 0 if failed
 */
int check_shared_inputs(t_bleport* porta,
                        t_bleport* portb,
                        t_ble_info* ble_info)
{
  int im;
  int ip;

  int porta_used;
  int porta_mux;
  int portb_used;
  int portb_mux;

  get_port_used_mux(porta,ble_info,&porta_used,&porta_mux);
  get_port_used_mux(portb,ble_info,&portb_used,&portb_mux);

  for (im=0; im<porta->cand_num; im++) {
    if ((0 == porta_used)||((1 == porta_used)&&(im == porta_mux))) {
      for (ip=0; ip<portb->cand_num; ip++) {
        if ((0 == portb_used)||((1 == portb_used)&&(ip == porta_mux))) {
          if (0 == strcmp(porta->candtoks[im],portb->candtoks[ip])) {
            return 1;
          }
        }
      }
    }
  } 
  
  return 0;
}

/**
 * Check a ble port is connected to a BLE input
 * Return 1 if connected
 */
int check_conkt_ble_input(t_bleport* port,
                          t_ble_info* ble_info)
{
  int icnd;
  int port_used;
  int port_mux;

  if (BLE_IN == port->type) {
    printf("Error: Port type cannot be BLE_IN in check_conkt_ble_input!\n");
    exit(1);
  }

  get_port_used_mux(port,ble_info,&port_used,&port_mux);

  for (icnd=0; icnd<port->cand_num; icnd++) {
    if ((0 == port_used)||((1 == port_used)&&(icnd == port_mux))) {
      if (BLE_IN == port->cands[icnd]->type) {
        return 1;
      }
    }
  }

  return 0; 
}

/**
 * Check the given cell's inputs can be a inverter or not
 */
int check_inv_cell(t_ble_arch* ble_arch,
                   t_ble_info* ble_info,
                   int cell_idx)
{
  int im;
  /*Re-initial*/
  for (im=0; im<ble_arch->blends[cell_idx].input_num; im++) {
    ble_arch->blends[cell_idx].inputs[im].inv_capable = 0;
  }  
  
  /*Search inputs 0 */
  if (check_shared_inputs(&ble_arch->blends[cell_idx].inputs[0],&ble_arch->blends[cell_idx].inputs[2],ble_info))
  {
    ble_arch->blends[cell_idx].inputs[0].inv_capable = 1;
  }
  /*Search inputs 1 */
  if (check_shared_inputs(&ble_arch->blends[cell_idx].inputs[1],&ble_arch->blends[cell_idx].inputs[2],ble_info))
  {
    ble_arch->blends[cell_idx].inputs[1].inv_capable = 1;
  }
  /*Search inputs 2 */
  if (check_shared_inputs(&ble_arch->blends[cell_idx].inputs[0],&ble_arch->blends[cell_idx].inputs[1],ble_info))
  {
    ble_arch->blends[cell_idx].inputs[2].inv_capable = 1;
  }
  /*Check no. of inputs conkt to BLE input*/
  if (2 == (check_conkt_ble_input(&ble_arch->blends[cell_idx].inputs[0],ble_info) + check_conkt_ble_input(&ble_arch->blends[cell_idx].inputs[2],ble_info))) {
    ble_arch->blends[cell_idx].inputs[1].inv_capable = 1;
  }
  if (2 == (check_conkt_ble_input(&ble_arch->blends[cell_idx].inputs[1],ble_info) + check_conkt_ble_input(&ble_arch->blends[cell_idx].inputs[2],ble_info))) {
    ble_arch->blends[cell_idx].inputs[0].inv_capable = 1;
  }
  if (2 == (check_conkt_ble_input(&ble_arch->blends[cell_idx].inputs[0],ble_info) + check_conkt_ble_input(&ble_arch->blends[cell_idx].inputs[1],ble_info))) {
    ble_arch->blends[cell_idx].inputs[2].inv_capable = 1;
  }

  return 1;
}


/**
 * Mark all possible position an inverter can be realized, each cell has three inputs!
 * Notice that: reconfig cell can realize A^B^C, AB+BC+AC.
 * When B=C, OUT=A (Func: A^B^C), OUT=B (Func: AB+AC+BC)
 * Return 0 if none can be found
 */
int mark_invs_ble(t_ble_arch* ble_arch,
                  t_ble_info* ble_info,
                  int verbose)
{
  int ind;
  int im;
  int inv_num = 0;

  /*Search cells*/
  for (ind=0; ind<ble_arch->blend_num; ind++) {
    check_inv_cell(ble_arch,ble_info,ind);
  }

  for (ind=0; ind<ble_arch->blend_num; ind++) {
    for (im=0; im<ble_arch->blends[ind].input_num; im++) {
      if (1 == ble_arch->blends[ind].inputs[im].inv_capable) {
        inv_num++;break;
      }
    }
  }
   
  if (0 == inv_num)
  {
    printf("Error: zero cell in BLE can be inverter! Check BLE Architecture!\n");
    exit(1);
  }
  else if (1 == verbose)
  {
    printf("Info: %d cell in BLE arch can be inverter.\n",inv_num);
  }
  
  return 1;
}

/*We only check port a and port b*/
int** check_fully_sat2_cell(t_ble_arch* ble_arch,
                            t_ble_info* ble_info,
                            int cell_idx,
                            int* sat2_num)
{
  int im;
  //int icnd;
 
  int ii;
  int ij;
  //int ik;

  int cell_num[2];
  int* cells[2];

  //int preidx;
  
  int** sat2_solve;
  (*sat2_num) = 0;
  
  /*Initial*/
  ble_arch->blends[cell_idx].sat2_capable = 0;

  /*Initial*/
  for (im=0; im<2; im++) {
      cell_num[im] = 0;
  }
  /*Collect cell interconnections*/ 
  cells[0] = check_port_conkt_cell(ble_arch,ble_info,cell_idx,0,&cell_num[0]);
  cells[1] = check_port_conkt_cell(ble_arch,ble_info,cell_idx,1,&cell_num[1]);

  /*If one of them has none cell connection*/
  if (0 == (cell_num[0]*cell_num[1])) {
    ble_arch->blends[cell_idx].sat2_capable = 0;
    return NULL;
  }
  /*Check if this is a SAT position*/
  for (ii=0; ii<cell_num[0]; ii++) {
    for (ij=0; ij<cell_num[1]; ij++) {
      if (check_sat2_conkts(ble_arch,cell_idx,cells[0][ii],cells[1][ij],ble_info)) {
        ble_arch->blends[cell_idx].sat2_capable = 1;
        (*sat2_num)++;
      }
    }
  }
  /*Allocate*/
  sat2_solve = (int**)my_malloc(sizeof(int*)*(*sat2_num));
  for (im=0; im<(*sat2_num); im++) {
    sat2_solve[im] = (int*)my_malloc(sizeof(int)*2);
  }
  (*sat2_num) = 0;
  /*Store SAT solutions*/
  for (ii=0; ii<cell_num[0]; ii++) {
    for (ij=0; ij<cell_num[1]; ij++) {
      if (check_sat2_conkts(ble_arch,cell_idx,cells[0][ii],cells[1][ij],ble_info)) {
        ble_arch->blends[cell_idx].sat2_capable = 1;
        sat2_solve[(*sat2_num)][0] = cells[0][ii]; 
        sat2_solve[(*sat2_num)][1] = cells[1][ij]; 
        (*sat2_num)++;
      }
    }
  }

  /*Initial*/
  for (im=0; im<2; im++) {
    cell_num[im] = 0;
    free(cells[im]);
  }  

  return sat2_solve;

}

int** check_fully_sat3_cell(t_ble_arch* ble_arch,
                            t_ble_info* ble_info,
                            int cell_idx,
                            int* sat3_num)
{
  int im;
  //int icnd;
 
  int ii;
  int ij;
  int ik;

  int cell_num[3];
  int* cells[3];

  //int preidx;
  
  int** sat3_solve;
  (*sat3_num) = 0;
  
  /*Initial*/
  ble_arch->blends[cell_idx].sat3_capable = 0;

  /*Initial*/
  for (im=0; im<3; im++) {
      cell_num[im] = 0;
  }
  /*Collect cell interconnections*/ 
  for (im=0; im<ble_arch->blends[cell_idx].input_num; im++) {
    cells[im] = check_port_conkt_cell(ble_arch,ble_info,cell_idx,im,&cell_num[im]);
  }
  /*If one of them has none cell connection*/
  if (0 == (cell_num[0]*cell_num[1]*cell_num[2])) {
    ble_arch->blends[cell_idx].sat3_capable = 0;
    return NULL;
  }
  /*Check if this is a SAT position*/
  for (ii=0; ii<cell_num[0]; ii++) {
    for (ij=0; ij<cell_num[1]; ij++) {
      for (ik=0; ik<cell_num[2]; ik++) {
        if (check_sat3_conkts(ble_arch,cell_idx,cells[0][ii],cells[1][ij],cells[2][ik],ble_info)) {
          ble_arch->blends[cell_idx].sat3_capable = 1;
          (*sat3_num)++;
        }
      }
    }
  }
  /*Store SAT solutions*/
  sat3_solve = (int**)my_malloc(sizeof(int*)*(*sat3_num));
  for (im=0; im<(*sat3_num); im++) {
    sat3_solve[im] = (int*)my_malloc(sizeof(int)*3);
  }
  (*sat3_num) = 0;

  for (ii=0; ii<cell_num[0]; ii++) {
    for (ij=0; ij<cell_num[1]; ij++) {
      for (ik=0; ik<cell_num[2]; ik++) {
        if (check_sat3_conkts(ble_arch,cell_idx,cells[0][ii],cells[1][ij],cells[2][ik],ble_info)) {
          ble_arch->blends[cell_idx].sat3_capable = 1;
          sat3_solve[(*sat3_num)][0] = cells[0][ii];
          sat3_solve[(*sat3_num)][1] = cells[1][ij];
          sat3_solve[(*sat3_num)][2] = cells[2][ik];
          (*sat3_num)++;
        }
      }
    }
  }

  /*Initial*/
  for (im=0; im<3; im++) {
    cell_num[im] = 0;
    free(cells[im]);
  }  

  return sat3_solve;
}

/**
 * Mark fully satisfied positions in BLE
 */
int mark_fully_sat_cells_ble(t_ble_arch* ble_arch,
                             t_ble_info* ble_info,
                             int verbose)
{
  int ind;
  int sat3_solve_num;
  int sat2_solve_num;
  
  for (ind=0; ind<ble_arch->blend_num; ind++) {
    check_fully_sat3_cell(ble_arch,ble_info,ind,&sat3_solve_num);
    check_fully_sat2_cell(ble_arch,ble_info,ind,&sat2_solve_num);
    if (1 == verbose) {
      if (1 == ble_arch->blends[ind].sat3_capable) {
        printf("Info: BLE Cell[%d] is SAT3 capable(Solve No.: %d)!\n",ind,sat3_solve_num);}
      if (1 == ble_arch->blends[ind].sat2_capable) {
        printf("Info: BLE Cell[%d] is SAT2 capable(Solve No,: %d)!\n",ind,sat2_solve_num);}
    }
  }
  
  return 1;
}

/**
 * Check SAT3 connections 
 */
int check_sat3_conkts(t_ble_arch* ble_arch,
                      int cell_idx,
                      int input0_cell,
                      int input1_cell,
                      int input2_cell,
                      t_ble_info* ble_info)
{
  t_blend* cell[3];
  //int icnd;
  //int im;
  //int ic;
  int port;
  int cand;

  cell[0] = &ble_arch->blends[ble_arch->blends[cell_idx].inputs[0].cands[input0_cell]->blend_idx]; 
  cell[1] = &ble_arch->blends[ble_arch->blends[cell_idx].inputs[1].cands[input1_cell]->blend_idx]; 
  cell[2] = &ble_arch->blends[ble_arch->blends[cell_idx].inputs[2].cands[input2_cell]->blend_idx]; 

  /** 
   * Rules:
   * 1. Two of them has shared inputs
   * 2. The rest two are connected
   */ 
   if (check_tree_conkts(cell[0],cell[1],cell[2],ble_info))
   {
     if (check_cells_conkted(cell[1],cell[2],&port,&cand,ble_info))
     {return 1;}
     if (check_cells_conkted(cell[2],cell[1],&port,&cand,ble_info))
     {return 1;}
   }

   if (check_tree_conkts(cell[1],cell[0],cell[2],ble_info))
   {
     if (check_cells_conkted(cell[0],cell[2],&port,&cand,ble_info))
     {return 1;}
     if (check_cells_conkted(cell[2],cell[0],&port,&cand,ble_info))
     {return 1;}
   }

   if (check_tree_conkts(cell[2],cell[0],cell[1],ble_info))
   {
     if (check_cells_conkted(cell[0],cell[1],&port,&cand,ble_info))
     {return 1;}
     if (check_cells_conkted(cell[1],cell[0],&port,&cand,ble_info))
     {return 1;}
   }

  return 0;
}

/**
 * Check SAT2 connections 
 */
int check_sat2_conkts(t_ble_arch* ble_arch,
                      int cell_idx,
                      int input0_cell,
                      int input1_cell,
                      t_ble_info* ble_info)
{
  t_blend* cell[2];
  int port;
  int cand;

  cell[0] = &ble_arch->blends[ble_arch->blends[cell_idx].inputs[0].cands[input0_cell]->blend_idx]; 
  cell[1] = &ble_arch->blends[ble_arch->blends[cell_idx].inputs[1].cands[input1_cell]->blend_idx]; 

  /** 
   * Rules:
   *   There is connection between each other
   */ 
   if (check_cells_conkted(cell[0],cell[1],&port,&cand,ble_info))
   {return 1;}
   if (check_cells_conkted(cell[1],cell[0],&port,&cand,ble_info))
   {return 1;}

  return 0;
}

/**
 * Mark Cell depth in  BLE architecture
 * We use Belman Ford Algorithm
 */
int dfs_ble_arch(t_ble_arch* ble_arch,
                 t_ble_info* ble_info,
                 int verbose)
{
  int im;
  int ip;
  int icnd;
  t_bleport* pre_port;
  int tmp_mux;
  //int stop = 0;
  int pre_match = 0;

  int* pre_depths = (int*)my_malloc(sizeof(int)*ble_arch->blend_num);
  int* dfsed = (int*)my_malloc(sizeof(int)*ble_arch->blend_num);

  /*Initial previous depths*/
  for (im=0; im<ble_arch->blend_num; im++) {
    pre_depths[im] = 0;
    dfsed[im] = 0;
    ble_arch->blends[im].depth = 0;
  }
  /*Update depths of each cell until no changes with last round*/
  while(1) {
    for (im=0; im<ble_arch->blend_num; im++) {
      for (ip=0; ip<ble_arch->blends[im].input_num; ip++) {
        /*Check if this port has been used, if used, update according to muxes*/
        if (1 == ble_info->blend_port_used[im][ip]) {
          tmp_mux = ble_info->blend_muxes[im][ip];
          pre_port = ble_arch->blends[im].inputs[ip].cands[tmp_mux];
          /*Ignore BLE_IN, we only care BLE_ND, Ensure dfsed == 0, avoid loop*/
          if ((BLE_ND == pre_port->type)&&(ble_arch->blends[im].depth < (ble_arch->blends[pre_port->blend_idx].depth + 1))&&(0 == dfsed[im])) {
            /*Update depth,dfsed*/
            ble_arch->blends[im].depth = ble_arch->blends[pre_port->blend_idx].depth + 1;
            dfsed[im] = 1;
          }
        }
        /*If not used, sweep all possible candidates*/
        else {
          for (icnd=0; icnd<ble_arch->blends[im].inputs[ip].cand_num; icnd++) {
            pre_port = ble_arch->blends[im].inputs[ip].cands[icnd];
            /*Ignore BLE_IN, we only care BLE_ND*/
            if ((BLE_ND == pre_port->type)&&(ble_arch->blends[im].depth < (ble_arch->blends[pre_port->blend_idx].depth + 1))&&(0 == dfsed[im])) {
              /*Update depth,dfsed*/
              ble_arch->blends[im].depth = ble_arch->blends[pre_port->blend_idx].depth + 1;
              dfsed[im] = 1;
            }
          }
        }
      } 
      /*Mark dfsed*/
      dfsed[im] = 1;
    }
    /**
     * Check if pre_depth is the same or not
     * If same, stop. If not, go on and update pre_depth
     */
    pre_match = 0;
    for (im=0; im<ble_arch->blend_num; im++) {
      if (pre_depths[im] == ble_arch->blends[im].depth) {
        pre_match++;
      }
    }
    /*Same results as before, stop*/
    if (pre_match == ble_arch->blend_num) {
      break;
    }
    /*Update pre_depth*/
    for (im=0; im<ble_arch->blend_num; im++) {
      pre_depths[im] = ble_arch->blends[im].depth;
    }     

    /*Clear dfsed*/
    for (im=0; im<ble_arch->blend_num; im++) {
      dfsed[im] = 0;
    }
  }       

  if (verbose == 1) {
    for (im=0; im<ble_arch->blend_num; im++) {
      printf("Info: Cell[%d].depth = %d\n",im,ble_arch->blends[im].depth);
    }
  }

  return 1;
}

int* local_dfs_ble_cell(t_ble_arch* ble_arch,
                        t_ble_info* ble_info,
                        int num_dfsnds,
                        t_blend** dfsnds,
                        int verbose)
{
  int im;
  int ip;
  int icnd;
  int idfsnd;
  t_bleport* pre_port;
  int tmp_mux;
  //int stop = 0;
  int pre_match = 0;
  t_blend* pre_cell;

  int* pre_depths = (int*)my_malloc(sizeof(int)*num_dfsnds);
  int* dfsed = (int*)my_malloc(sizeof(int)*num_dfsnds);
  int* cur_depths = (int*)my_malloc(sizeof(int)*num_dfsnds);

  /*Initial previous depths*/
  for (im=0; im<num_dfsnds; im++) {
    pre_depths[im] = 0;
    dfsed[im] = 0;
    cur_depths[im] = 0;
  }
  /*Update depths of each cell until no changes with last round*/
  while(1) {
    for (im=0; im<num_dfsnds; im++) {
      for (ip=0; ip<dfsnds[im]->input_num; ip++) {
        /*Check if this port has been used, if used, update according to muxes*/
        if (1 == ble_info->blend_port_used[im][ip]) {
          tmp_mux = ble_info->blend_muxes[im][ip];
          pre_port = ble_arch->blends[im].inputs[ip].cands[tmp_mux];
          /*Ignore BLE_IN, we only care BLE_ND, Ensure dfsed == 0, avoid loop*/
          if ((BLE_ND == pre_port->type)&&(0 == dfsed[im])) {
            pre_cell = &ble_arch->blends[pre_port->blend_idx];
            /*Check if connected BLE cell is in the list*/
            for (idfsnd=0; idfsnd<num_dfsnds; idfsnd++) {
              if ((dfsnds[idfsnd] == pre_cell)&&(cur_depths[im] < (cur_depths[idfsnd]+1))) {
                /*Update depth,dfsed*/
                cur_depths[im] = cur_depths[idfsnd] + 1;
                dfsed[im] = 1;
              }
            }
          }
        }
        /*If not used, sweep all possible candidates*/
        else {
          for (icnd=0; icnd< dfsnds[im]->inputs[ip].cand_num; icnd++) {
            pre_port = dfsnds[im]->inputs[ip].cands[icnd];
            /*Ignore BLE_IN, we only care BLE_ND, Ensure dfsed == 0, avoid loop*/
            if ((BLE_ND == pre_port->type)&&(0 == dfsed[im])) {
              pre_cell = &ble_arch->blends[pre_port->blend_idx];
              /*Check if connected BLE cell is in the list*/
              for (idfsnd=0; idfsnd<num_dfsnds; idfsnd++) {
                if ((dfsnds[idfsnd] == pre_cell)&&(cur_depths[im] < (cur_depths[idfsnd]+1))) {
                  /*Update depth,dfsed*/
                  cur_depths[im] = cur_depths[idfsnd] + 1;
                  dfsed[im] = 1;
                }
              }
            }
          }
        }
      } 
      /*Mark dfsed*/
      dfsed[im] = 1;
    }
    /**
     * Check if pre_depth is the same or not
     * If same, stop. If not, go on and update pre_depth
     */
    pre_match = 0;
    for (im=0; im<num_dfsnds; im++) {
      if (pre_depths[im] == cur_depths[im]) {
        pre_match++;
      }
    }
    /*Same results as before, stop*/
    if (pre_match == num_dfsnds) {
      break;
    }
    /*Update pre_depth*/
    for (im=0; im<num_dfsnds; im++) {
      pre_depths[im] = cur_depths[im];
    }     

    /*Clear dfsed*/
    for (im=0; im<num_dfsnds; im++) {
      dfsed[im] = 0;
    }
  }       
  bubble_sort_depth_ble_dfsnds(num_dfsnds,dfsnds,cur_depths);

  if (verbose == 1) {
    for (im=0; im<num_dfsnds; im++) {
      printf("Info: Cell[%d].depth = %d\n",dfsnds[im]->idx,cur_depths[im]);
    }
  }

  return cur_depths;
}

/*Bubble Descend Sort, local DFS BLE cells*/
int bubble_sort_depth_ble_dfsnds(int num_dfsnd,
                                 t_blend** dfsnds,
                                 int* cur_depths)
{
  int i = num_dfsnd;
  int j;
  int tmp;
  t_blend* tmpnd;

  while(i > 0) {
    for (j=0; j<(i - 1); j++) {
      if (cur_depths[j] > cur_depths[j+1]) {
        /*Swap*/
        tmp = cur_depths[j];
        tmpnd = dfsnds[j];
        cur_depths[j] = cur_depths[j+1];
        dfsnds[j] = dfsnds[j+1];
        cur_depths[j+1] = tmp;
        dfsnds[j+1] = tmpnd;
      }
    }
    i--;
  }
  return 1;
}

/**
 * Check independent cell in BLE.
 * Independent cell definition: each input(refer to input number of given logic cell) has 
 *                              connection to BLE input(we don't mind if it has been used,
 *                              because we can share) and there is free BLE output. 
 * Return 1 if it is an independent cell.
 * Return 0 if it is not.
 */
int check_independent_cell(t_ble_arch* ble_arch,
                           t_ble_info* ble_info,
                           int cell_idx,
                           int input_num)
{
  int im;
  int icnd;
  int ret = 0;
  t_bleport* port;

  /*check if cell has been used*/
  if (1 == ble_info->blend_used[cell_idx]) {
    return 0;
  }
  /*Check inputs have connections to BLE input*/
  for (im=0; im<input_num; im++) {
    ret = 0;
    for (icnd=0; icnd<ble_arch->blends[cell_idx].inputs[im].cand_num; icnd++) {
      if (1 == ble_info->blend_port_used[cell_idx][im]) {
        printf("Warning: BLE cell port used but cell is not used! (check_indepentent cell)\n");
        return 0;
      }
      port = ble_arch->blends[cell_idx].inputs[im].cands[icnd];
      /*If we find, mark success*/
      if (BLE_IN == port->type) {
        ret = 1;
      }
    }
    /*Check. Return if we have not found*/
    if (0 == ret) {
      return 0;
    } 
  }

  /*check if we have free output*/
  ret = 0;
  for (im=0; im<ble_arch->output_num; im++) {
    for (icnd=0; icnd<ble_arch->outputs[im].cand_num; icnd++) {
      port = ble_arch->outputs[im].cands[icnd];
      if ((0 == ble_info->output_used[im])&&(BLE_ND == port->type)&&(cell_idx == port->blend_idx)) {
        ret = 1;
      }
    }
  }

  return ret;
} 

/**
 * Check network is SAT[sat_type], sat_type > 0
 * Return result: sat_idexes.
 * Sat_idxes contains the index of each sat cell. 
 * IMPORTANT: here we DON'T malloc it. Preferred size of sat_idxes is sat_type
 * Return 1 if it is
 * Return 0 if it isn't
 */
int check_sat(int sat_type,
              t_ble_arch* ble_arch,
              t_ble_info* ble_info,
              t_blend* root,
              int* sat_idxes,
              int verbose)
{
  int im = 0;
  int icnd = 0;
  t_blend** root_ntwk = NULL;
  int* depths;

  /*Check if sat_type > 0, you don't need to check sat0*/
  if (sat_type < 0) {
    printf("Warning: (check_sat)sat_type must be no less than 0!\n");
    return 0;
  }
  
  if (0 == sat_type) {
    return 1;
  }

  /*Check if sat_type is greater than root node's input number*/
  if (sat_type > root->input_num) {
    printf("Warning: (check_sat)sat_type is greater than root's input number!\n");
    return 0;
  }
  /*Construct array root ntwk, root sits the last position*/
  root_ntwk = (t_blend**)my_malloc(sizeof(t_blend*)*sat_type+1);
  root_ntwk[sat_type] = root;
  for (im=0; im<sat_type; im++) {
    root_ntwk[im] = NULL;
  } 
  /* Check every inputs of root node is unused. And fill root ntwk 
   * Here we put restriction on the number of BLE ND each root inputs connected to.
   * It should be only one! We only search the first BLE ND connected!
   */
  for (im=0; im<sat_type; im++) {
    for (icnd=0; icnd<root->inputs[im].cand_num; icnd++) {
      if (BLE_ND == root->inputs[im].cands[icnd]->type) {
        if (1 == ble_info->blend_used[root->inputs[im].cands[icnd]->blend_idx]) {
          free(root_ntwk);
          return 0;
        }
        else {
          root_ntwk[im] = &ble_arch->blends[root->inputs[im].cands[icnd]->blend_idx];
        }
      }
    }
    /*We finish searching this round, check if we have found anything*/ 
    if (NULL == root_ntwk[im]) {
      free(root_ntwk);
      return 0;
    }
  }
  /*Check each element in root ntwk is unique*/
  for (im=0; im<(sat_type+1); im++) {
    for (icnd=im+1; icnd<(sat_type+1); icnd++) {
      if (root_ntwk[im] == root_ntwk[icnd]) {
        printf("Warning:(check_sat)Found duplicated elements in root ntwk!\n");
        free(root_ntwk);
        return 0;
      }
    }
  }
  /*DFS roots' network*/
  depths = local_dfs_ble_cell(ble_arch,ble_info,sat_type+1,root_ntwk,0);

  /*Check predecessor's network*/
  if (0 == check_sat(sat_type-1,ble_arch,ble_info,root_ntwk[sat_type-1],sat_idxes,verbose)) {
    free(depths);
    free(root_ntwk);
    return 0;
  }

  /*Record the index*/
  for (im=0; im<sat_type; im++) {
    sat_idxes[im] = root_ntwk[im]->idx;
  }
  
  /*Free resources*/ 
  free(root_ntwk);
  free(depths);

  return 1; 
}

/**
 * Pick a Position from BLE architecture
 * A. An available cell with highest priority or SAT2/3
 *   1. Logic node has 2 inputs, give a SAT2 cell
 *   2. Logic node has 3 inputs, give a SAT3 cell
 * B. If there is none avaiable SAT cell, give a cell with highest priority
 */
int pick_position_from_ble_arch(t_ble_arch* ble_arch,
                                t_ble_info* ble_info,
                                t_lgknd* lgknd,
                                int* blend_tried)
{
  int ret = -1;
  int priority = -1;
  int im;
  int sat_type = lgknd->input_num;

  /*Re-mark SAT2,SAT3 cell in the BLE with updated info*/
  mark_fully_sat_cells_ble(ble_arch,ble_info,0);
  /*DFS BLE Arch*/
  dfs_ble_arch(ble_arch,ble_info,0);

  /*Try position from SAT2 or SAT3 cell with highest priority*depth */
  for (im=0; im<ble_info->blend_num; im++) {
    /*Check unused cell*/
    if (0 == blend_tried[im]) {
      if ((-1 == priority)||(ble_arch->blends[im].priority > priority)) {
        if ((2 == sat_type)&&(1 == ble_arch->blends[im].sat2_capable)) {
          ret = im;
          priority = ble_arch->blends[im].priority*ble_arch->blends[im].depth;
        }
        if ((3 == sat_type)&&(1 == ble_arch->blends[im].sat3_capable)) {
          ret = im;
          priority = ble_arch->blends[im].priority*ble_arch->blends[im].depth;
        }
      }
    }
  }
  /*If we find, return ret*/
  if (-1 != ret) {
    return ret;
  }

  /*If we don't find, continue searching, an un-used cell with highest priority*/
  for (im=0; im<ble_info->blend_num; im++) {
    /*Check unused cell*/
    if (0 == blend_tried[im]) {
      if (0 == ble_info->blend_used[im]) {
        if ((-1 == priority)||(ble_arch->blends[im].priority > priority)) {
          ret = im;
          priority = ble_arch->blends[im].priority;
        }
      }
    }
  }

  return ret;
}

/* Return the index of an unused cell whose depth and priority is maximum
 * Return -1 if nothing can be found
 */
int pick_cell_idx_from_ble_arch(t_ble_arch* ble_arch,
                                t_ble_info* ble_info,
                                int* blend_tried)
{
  int ret = -1;
  int priority = -1;
  int tmp_priority;

  int im;

  /*DFS BLE Arch*/
  //dfs_ble_arch(ble_arch,ble_info,0);

  /*If we don't find, continue searching, an un-used cell with highest priority*/
  for (im=0; im<ble_info->blend_num; im++) {
    /*Check unused cell*/
    if ((0 == blend_tried[im])&&(0 == ble_info->blend_used[im])) {
      tmp_priority = ble_arch->blends[im].priority*ble_arch->blends[im].depth;
      if ((-1 == priority)||(tmp_priority > priority)) {
        ret = im;
        priority = tmp_priority;
      }
    }
  }
  
  return ret;
}

/*Sum the number of all cell inputs*/
int sum_cell_input_num(t_ble_arch* ble_arch,
                       int verbose)
{ 
  int ret = 0;
  int i;  

  for (i=0; i<ble_arch->blend_num; i++) {
    ret += ble_arch->blends[i].input_num;
  }

  return ret;
}

/*Calculate the number of DFFs in BLE architecture*/
int sum_ff_num(t_ble_arch* ble_arch,
               int verbose)
{
  int im;
  int ff_num = 0;

  for (im = 0; im<ble_arch->output_num; im++) {
    if (0 == ble_arch->outputs[im].is_comb) {
      ff_num++;
    }
  }
  return ff_num;
}

