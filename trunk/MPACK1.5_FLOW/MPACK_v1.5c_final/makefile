# MCluster Packer (MPack) makefile

# compiler
CC = gcc

# flags
CFLAGS=-Wall -g

# sources 
SRC=./SRC/*.c
EXEC=mpack

# figures 
MAPDOT= $(wildcard ./WORK/FIG/map_cluster*.dot)
MAPROOT=$(MAPDOT:.dot=)
FCTDOT= $(wildcard ./WORK/FIG/fct_cluster*.dot)
FCTROOT=$(FCTDOT:.dot=)

# rules

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC)

fig:
	rm -f ./WORK/FIG/*.svg
	for f in $(MAPROOT); do \
		neato -n -Tsvg $$f.dot -o $$f.svg; \
	done
	for f in $(FCTROOT); do \
		dot -n -Tsvg $$f.dot -o $$f.svg; \
	done
	rm -f ./WORK/FIG/*.dot

workclean:
	rm -rf ./WORK

clean:
	rm -f $(EXEC)
	rm -f *~ ./SRC/*~

mrproper: clean
	rm -f *.echo *.blif *.rpt *.dot * 