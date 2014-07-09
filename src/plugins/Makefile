CCFLAGS=-Wall -O0 -g
CCDYNAMICFLAGS=${CCFLAGS} -fPIC
LDFLAGS=
LDDYNAMICFLAGS=${LDFLAGS} -shared
OBJECTS=hello.o matrixKernel.o matrixPlugin.o matrixLinker.o matrix.o
INCLUDE=-I ../ -I include/

NVCCFLAGS=-G -g  -DDEBUG # TODO: remove the -G, -g -DDEBUG when done
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

all: hello.so matrix.so

# %.so: %.o
# 	${CC} ${LDDYNAMICFLAGS} -o $@ $<

# %.o: %.c
# 	${CC} ${CCDYNAMICFLAGS} ${INCLUDE} -c -o $@ $<


# ///////////////////// Matrix executable
matrix: matrix.cpp matrix.o
	g++ -g -DDEBUG matrix.o matrix.cpp -o $@ ${INCLUDE} -l cudart

matrix.o: matrix.cu
	nvcc ${NVCCFLAGS} -c matrix.cu ${INCLUDE}

# ///////////////////// Hello World sample plugin
hello.so: hello.o
	cc ${LDDYNAMICFLAGS} -o $@ $<

hello.o: hello.c
	cc ${CCDYNAMICFLAGS} ${INCLUDE} -c -o $@ $<

# ///////////////////// Matrix plugin
matrix.so: matrixKernel.o matrixPlugin.o matrixLinker.o
	g++ ${LDDYNAMICFLAGS} -o $@ matrixKernel.o matrixPlugin.o matrixLinker.o -lcudart

matrixKernel.o: matrix.cu
	nvcc ${NVCCDYNAMICFLAGS} ${INCLUDE} -dc $< -o $@ $(GENCODE_FLAGS)

matrixPlugin.o: matrix.cpp
	nvcc ${NVCCDYNAMICFLAGS} ${INCLUDE} -dc $< -o $@ $(GENCODE_FLAGS)

matrixLinker.o: matrixKernel.o matrixPlugin.o
	nvcc ${NVCCDYNAMICFLAGS} -dlink  matrixKernel.o matrixPlugin.o -o $@ $(GENCODE_FLAGS)

# ///////////////////// etc...

clean:
	rm -f *.o *.so matrix

.SECONDARY: ${OBJECTS}
