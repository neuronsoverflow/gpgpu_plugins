#  right now, not using CUDA or OpenCL for the plugins...
CCFLAGS=-Wall -O0 -g
CCDYNAMICFLAGS=${CCFLAGS} -fPIC
LDFLAGS=
LDDYNAMICFLAGS=${LDFLAGS} -shared
OBJECTS=hello.o
INCLUDE=-I ../

all: hello.so

%.so: %.o
	${CC} ${LDDYNAMICFLAGS} -o $@ $<

%.o: %.c
	${CC} ${CCDYNAMICFLAGS} ${INCLUDE} -c -o $@ $<

clean:
	rm -f *.o *.so

.SECONDARY: ${OBJECTS}
