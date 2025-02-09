# source /home/wangpy/zjw/intel/oneapi/setvars.sh
BINARY_NAME = gemm
CC			= /home/wangpy/zjw/intel/oneapi/2024.2/bin/dpcpp
CFLAGS		= -O2 -g -march=native -fno-strict-aliasing
GDB_FLAGS = -gdwarf-5 -gstrict-dwarf

MKLPATH		= /home/wangpy/zjw/intel/oneapi/mkl/2024.2
LDFLAGS		= -L$(MKLPATH)/lib/intel64
INCFLAGS	= -I$(MKLPATH)/include

OTHER_FLGAS  = -qmkl=parallel
OTHER_LIBS   = 

SRC			= $(wildcard *.c)
build : $(BINARY_NAME)

$(BINARY_NAME): $(SRC)
	$(CC) $(CFLAGS) $(GDB_FLAGS) $(LDFLAGS) $(INCFLAGS) $(OTHER_FLGAS) $(OTHER_LIBS) $(SRC) -o $(BINARY_NAME)

clean:
	rm $(BINARY_NAME)
