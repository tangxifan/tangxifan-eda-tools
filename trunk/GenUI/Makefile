CC = gcc
LIB = -lm
SRC_DIR = SRC

WARN_FLAGS = -Wall

EXE = GenUI

OBJ = OBJ/main.o \
OBJ/read_opts.o \
OBJ/util.o

H = SRC/Options.h\
SRC/util.h

$(EXE): $(OBJ) Makefile 
	$(CC) $(WARN_FLAGS) -o $(EXE) $(OBJ) $(LIB)

OBJ/%.o:$(SRC_DIR)/%.c
	$(CC) $(FLAGS) -c $< -o $@

clean: 
	rm -f $(EXE) $(OBJ)

