CCFLAGS=-Wall -O0 -g
CCDYNAMICFLAGS=${CCFLAGS} -fPIC
LDFLAGS=
LDDYNAMICFLAGS=${LDFLAGS} -shared
OBJECTS := hello.o
OBJECTS += matrixLinker.o matrixPlugin.o matrixKernel.o matrix.o
OBJECTS += primeLinker.o primePlugin.o primeKernel.o prime.o

INCLUDE=-I ../ -I include/

# TODO: remove the debugging flags when done, then add optimization flags
DEBUG=-g -DDEBUG
NVCCFLAGS=-G ${DEBUG}
GCCFLAGS=${DEBUG}
NVCCDYNAMICFLAGS=${NVCCFLAGS} --compiler-options '-fPIC'

# commenting out the unsupported gpu architectures for my NVIDIA Device:
# GENCODE_SM10    := -gencode arch=compute_10,code=sm_10
GENCODE_SM20    := -gencode arch=compute_20,code=sm_20
GENCODE_SM30    := -gencode arch=compute_30,code=sm_30
# GENCODE_SM32    := -gencode arch=compute_32,code=sm_32
GENCODE_SM35    := -gencode arch=compute_35,code=sm_35
# GENCODE_SM50    := -gencode arch=compute_50,code=sm_50
# GENCODE_SMXX    := -gencode arch=compute_50,code=compute_50
GENCODE_FLAGS   ?= $(GENCODE_SM10) $(GENCODE_SM20) $(GENCODE_SM30) $(GENCODE_SM32) $(GENCODE_SM35) $(GENCODE_SM50) $(GENCODE_SMXX)

all: hello.so matrix.so matrix prime

# %.so: %.o
# 	${CC} ${LDDYNAMICFLAGS} -o $@ $<

# %.o: %.c
# 	${CC} ${CCDYNAMICFLAGS} ${INCLUDE} -c -o $@ $<

# ///////////////////// Prime plugin

# ///////////////////// generic plugin maker
%.so: %Linker.o %Plugin.o %Kernel.o
	g++ ${LDDYNAMICFLAGS} -o $@ $^ -lcudart

%Linker.o: %Plugin.o %Kernel.o
	nvcc ${NVCCDYNAMICFLAGS} -dlink $^ -o $@ $(GENCODE_FLAGS)

%Plugin.o: %.cpp
	nvcc ${NVCCDYNAMICFLAGS} ${INCLUDE} -dc $< -o $@ $(GENCODE_FLAGS)

%Kernel.o: %.cu
	nvcc ${NVCCDYNAMICFLAGS} ${INCLUDE} -dc $< -o $@ $(GENCODE_FLAGS)

# ///////////////////// generic executable maker
# these are not working for some reason... Hmm...
# %: %.cpp %Executable.o
# 	g++ ${DEBUG} $^ -o $@ ${INCLUDE} -l cudart

# %Executable.o: %.cu
# 	nvcc ${NVCCFLAGS} -c $< -o $@ ${INCLUDE}


# ///////////////////// Prime executable
prime: prime.cpp prime.o
	g++ -g -DDEBUG prime.cpp prime.o -o $@ ${INCLUDE} -l cudart

prime.o: prime.cu
	nvcc ${NVCCFLAGS} -c $< ${INCLUDE}

# # ///////////////////// Matrix executable
matrix: matrix.cpp matrix.o
	g++ -g -DDEBUG matrix.o matrix.cpp -o $@ ${INCLUDE} -l cudart

matrix.o: matrix.cu
	nvcc ${NVCCFLAGS} -c matrix.cu ${INCLUDE}

# ///////////////////// Hello World sample plugin
hello.so: hello.o
	cc ${LDDYNAMICFLAGS} -o $@ $<

hello.o: hello.c
	cc ${CCDYNAMICFLAGS} ${INCLUDE} -c -o $@ $<

# ///////////////////// etc...

clean:
	rm -f *.o *.so matrix prime

.SECONDARY: ${OBJECTS}
