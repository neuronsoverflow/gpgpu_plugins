# DEBUG=-O0 -g -DDEBUG
CCFLAGS=-Wall ${DEBUG}
CCDYNAMICFLAGS=${CCFLAGS} -fPIC
LDFLAGS=
LDDYNAMICFLAGS=${LDFLAGS} -shared
EXECUTABLES := matrix prime graph
PLUGINS := hello.so matrix.so prime.so graph.so
OBJECTS := hello.o
OBJECTS += matrixLinker.o matrixPlugin.o matrixKernel.o matrix.o
OBJECTS += primeLinker.o primePlugin.o primeKernel.o prime.o
OBJECTS += graphLinker.o graphPlugin.o graphKernel.o graph.o

INCLUDE=-I ../ -I include/

CUDA_PATH ?= /opt/cuda
GCC ?= g++
NVCC := $(CUDA_PATH)/bin/nvcc -ccbin $(GCC)
NVCCFLAGS   := -m64 -O3

# NVCCDEBUGFLAGS=-G ${DEBUG}
NVCCFLAGS += ${NVCCDEBUGFLAGS}
GCCFLAGS=${DEBUG}
NVCCDYNAMICFLAGS=${NVCCFLAGS} --compiler-options '-fPIC'

# commenting out the unsupported gpu architectures for my NVIDIA Device:
# GENCODE_SM10    := -gencode arch=compute_10,code=sm_10
# GENCODE_SM11    := -gencode arch=compute_11,code=sm_11
# GENCODE_SM20    := -gencode arch=compute_20,code=sm_20
GENCODE_SM30    := -gencode arch=compute_30,code=sm_30
GENCODE_SM32    := -gencode arch=compute_32,code=sm_32
GENCODE_SM35    := -gencode arch=compute_35,code=sm_35
GENCODE_SM50    := -gencode arch=compute_50,code=sm_50
GENCODE_SMXX    := -gencode arch=compute_50,code=compute_50
GENCODE_FLAGS   ?= $(GENCODE_SM10) $(GENCODE_SM11) $(GENCODE_SM20) $(GENCODE_SM30) $(GENCODE_SM32) $(GENCODE_SM35) $(GENCODE_SM50) $(GENCODE_SMXX)

all: plugins

plugins: ${PLUGINS}

executables: ${EXECUTABLES}

# %.so: %.o
#	${CC} ${LDDYNAMICFLAGS} -o $@ $<

# %.o: %.c
#	${CC} ${CCDYNAMICFLAGS} ${INCLUDE} -c -o $@ $<

# ///////////////////// Prime plugin

# ///////////////////// generic plugin maker
# NOTE: dependency "graph.h" is not listed for "graph.so"
%.so: %Linker.o %Plugin.o %Kernel.o
	$(NVCC)${LDDYNAMICFLAGS} $(GENCODE_FLAGS) -o $@ $^
# g++ ${LDDYNAMICFLAGS} -o $@ $^ -lcudart

%Linker.o: %Plugin.o %Kernel.o
	$(NVCC) ${NVCCDYNAMICFLAGS} -dlink $^ -o $@ $(GENCODE_FLAGS)

%Plugin.o: %.cpp
	$(NVCC) ${NVCCDYNAMICFLAGS} ${INCLUDE} -dc $< -o $@ $(GENCODE_FLAGS)

%Kernel.o: %.cu
	$(NVCC) ${NVCCDYNAMICFLAGS} ${INCLUDE} -dc $< -o $@ $(GENCODE_FLAGS)

# ///////////////////// generic executable maker
# these are not working for some reason... Hmm...
# %: %.cpp %Executable.o
#	g++ ${DEBUG} $^ -o $@ ${INCLUDE} -l cudart

# %Executable.o: %.cu
#	$(NVCC) ${NVCCFLAGS} -c $< -o $@ ${INCLUDE}


# ///////////////////// Prime executable
prime: prime.cpp prime.o
	$(NVCC) ${NVCCFLAGS} $(GENCODE_FLAGS) $^ -o $@ ${INCLUDE}

prime.o: prime.cu
	$(NVCC) ${NVCCFLAGS} $(GENCODE_FLAGS) -c $< ${INCLUDE}

# ///////////////////// Matrix executable
matrix: matrix.cpp matrix.o
	$(NVCC) ${NVCCFLAGS} $(GENCODE_FLAGS) $^ -o $@ ${INCLUDE}

matrix.o: matrix.cu
	$(NVCC) ${NVCCFLAGS} $(GENCODE_FLAGS) -c $< ${INCLUDE}

# ///////////////////// BFS executable
graph: graph.cpp graph.o graph.h
	$(NVCC) ${NVCCFLAGS} $(GENCODE_FLAGS) $< graph.o -o $@ ${INCLUDE}

graph.o: graph.cu graph.h
	$(NVCC) ${NVCCFLAGS} $(GENCODE_FLAGS) -c $< ${INCLUDE}

# ///////////////////// Hello World sample plugin
hello.so: hello.o
	cc ${LDDYNAMICFLAGS} -o $@ $<

hello.o: hello.c
	cc ${CCDYNAMICFLAGS} ${INCLUDE} -c -o $@ $<

# ///////////////////// etc...

clean:
	rm -f *.o *.so ${EXECUTABLES}

.SECONDARY: ${OBJECTS}
