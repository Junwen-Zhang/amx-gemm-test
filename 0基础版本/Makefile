CFLAG = -O2 -march=native -fno-strict-aliasing -g
GDB_FLAGS = -gdwarf-5 -gstrict-dwarf 
CC = gcc
BIN = gemm 
CFILES = gemm.c 

all:
	$(CC) $(CFLAG) $(CFILES) $(GDB_FLAGS) -o $(BIN) $(LIBS)

clean:
	-rm $(BIN)

.PHONY: clean

