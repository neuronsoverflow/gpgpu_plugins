CCFLAGS=-Wall -O0 -g
CCSTATICFLAGS=${CCFLAGS} -rdynamic
LDFLAGS=
LDSTATICFLAGS=${LDFLAGS} -ldl

OBJECTS=pluginEngine.o helpers.o plugin.o

all: gpgpu
	@echo "Compiling plugins..."
	cd plugins/; make

gpgpu: ${OBJECTS}
	g++ -o $@ ${OBJECTS} main.cpp ${LDSTATICFLAGS}

pluginEngine.o: pluginEngine.cpp pluginEngine.h helpers.h plugin.h
	g++ ${CCSTATICFLAGS} -c -o $@ $<

plugin.o: plugin.cpp plugin.h constants.h helpers.h
	g++ ${CCFLAGS} -c -o $@ $<

helpers.o: helpers.cpp helpers.h
	g++ ${CCFLAGS} -c -o $@ $<

#############################################################
# TODO: figure out what to do w/ these 2 GPU check functions...
has_cuda_gpu: has_cuda_gpu.cu
	nvcc has_cuda_gpu.cu -o has_cuda_gpu

has_opencl_gpu: has_opencl_gpu.cpp
	g++ has_opencl_gpu.cpp -l OpenCL

clean:
	rm -f *.o gpgpu has_cuda_gpu has_opencl_gpu
	cd plugins/; make clean

.SECONDARY: ${OBJECTS}
