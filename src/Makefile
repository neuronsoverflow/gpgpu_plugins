# CCFLAGS=-Wall -O0 -g
CCFLAGS=-Wall -O3
CCSTATICFLAGS=${CCFLAGS} -rdynamic
LDFLAGS=
LDSTATICFLAGS=${LDFLAGS} -ldl

OBJECTS=pluginEngine.o helpers.o plugin.o orderedMap.o

all: gpgpu
	@echo "Compiling plugins..."
	cd plugins/; make

gpgpu: ${OBJECTS}
	g++ -o $@ ${OBJECTS} main.cpp ${LDSTATICFLAGS}

orderedMap.o: orderedMap.cpp orderedMap.h
	g++ ${CCFLAGS} -c -o $@ $<

pluginEngine.o: pluginEngine.cpp pluginEngine.h helpers.h plugin.h
	g++ ${CCSTATICFLAGS} -c -o $@ $<

plugin.o: plugin.cpp plugin.h constants.h helpers.h
	g++ ${CCFLAGS} -c -o $@ $<

helpers.o: helpers.cpp helpers.h
	g++ ${CCFLAGS} -c -o $@ $<

clean:
	rm -f *.o gpgpu
	cd plugins/; make clean

.SECONDARY: ${OBJECTS}
