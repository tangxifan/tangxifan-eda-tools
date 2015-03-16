#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <malloc.h>
#include "util.h"
#include "blif_parser.h"
#include "blearch_parser.h"
#include "parsers.h"


void blif_parser_flow(char* blifname,
                      t_blif_arch* blif_arch)
{
  blif_parser(blifname,blif_arch);
  blif_arch->nets = build_nets(blif_arch); 
  
}

void ble_arch_parser_flow(char* ble_arch_file,
                          t_ble_arch* ble_arch)
{
  parse_ble_arch(ble_arch_file,ble_arch); 
}
