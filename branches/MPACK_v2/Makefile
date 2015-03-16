CC = gcc
LIB = -lm
SRC_DIR = SRC

FLAGS = -g -Wall
#FLAGS = -g

WARN_FLAGS = -Wall

EXE = mpack2

OBJ = OBJ/main.o \
OBJ/read_opts.o \
OBJ/util.o \
OBJ/config_mpack.o \
OBJ/parsers.o \
OBJ/blif_parser.o \
OBJ/setup_mpack.o \
OBJ/mpack_util.o \
OBJ/blearch_parser.o \
OBJ/packing.o \
OBJ/check_ble_arch.o \
OBJ/stats.o \
OBJ/lgkntwk_common.o \
OBJ/sat_api.o \
OBJ/print_blif.o \
OBJ/print_vpr_arch.o \
OBJ/print_vpr_netlist.o 

H = SRC/Options.h \
SRC/util.h \
SRC/global.h \
SRC/mpack_conf.h \
SRC/init_opts.h \
SRC/read_opts.h \
SRC/blif_parser.h \
SRC/parsers.h \
SRC/blearch_parser.h \
SRC/packing.h \
SRC/setup_mpack.h \
SRC/check_ble_arch.h \
SRC/stats.h \
SRC/lgkntwk_common.h \
SRC/sat_api.h \
SRC/mpack_api.h \
SRC/print_vpr_arch.h \
SRC/print_vpr_netlist.h \
SRC/init_mpack_types.h \
SRC/arch_timing.h 

$(EXE): $(OBJ) Makefile 
	$(CC) $(WARN_FLAGS) -o $(EXE) $(OBJ) $(LIB)

OBJ/%.o:$(SRC_DIR)/%.c
	$(CC) $(FLAGS) -c $< -o $@

clean: 
	rm -f $(EXE) $(OBJ)

