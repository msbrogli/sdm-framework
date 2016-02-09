
CC=cc
CCFLAGS=-O2 -Wall -Wno-gnu -pedantic -fPIC -DSDM_USE_BITCOUNT_TABLE -DSDM_ENABLE_OPENCL #-std=c90

CLFLAGS=-framework OpenCL

LIB_OBJS=lib/base64.o

OBJS=bitstring.o \
	 address_space.o \
	 scanner_thread.o \
	 scanner_opencl.o \
	 counter.o \
	 operations.o

HEADERS=bitstring.h \
		address_space.h \
		scanner_thread.h \
		scanner_opencl.h \
		counter.h \
		operations.h

.PHONY: all
all: libsdm.a libsdm.so

.PHONY: tests
tests: test/test_bitstring

libsdm.so: ${OBJS} ${LIB_OBJS}
	${CC} ${CLFLAGS} -shared -o libsdm.so ${OBJS} ${LIB_OBJS}

libsdm.a: ${OBJS} ${LIB_OBJS}
	ar -cvr libsdm.a ${OBJS} ${LIB_OBJS}
	ranlib libsdm.a

test1: test1.c libsdm.a
	${CC} ${CCFLAGS} ${CLFLAGS} libsdm.a test1.c -o test1

test1-dyn: test1.c libsdm.so
	${CC} ${CCFLAGS} ${CLFLAGS} libsdm.so test1.c -o test1-dyn

test2: test2.c libsdm.a
	${CC} ${CCFLAGS} ${CLFLAGS} libsdm.a test2.c -o test2

test3: test3.c libsdm.a
	${CC} ${CCFLAGS} ${CLFLAGS} libsdm.a test3.c -o test3

test4: test4.c libsdm.a
	${CC} ${CCFLAGS} ${CLFLAGS} libsdm.a test4.c -o test4

lib/base64.o: lib/base64.c lib/base64.h
	(cd lib/ ; ${CC} ${CCFLAGS} -c base64.c)

operations.o: operations.c operations.h
	${CC} ${CCFLAGS} -c operations.c

address_space.o: address_space.h address_space.c version.h
	${CC} ${CCFLAGS} -c address_space.c

bitstring.o: bitstring.h bitstring.c
	${CC} ${CCFLAGS} -c bitstring.c

scanner_thread.o: scanner_thread.c
	${CC} ${CCFLAGS} -c scanner_thread.c

scanner_opencl.o: scanner_opencl.c scanner_opencl.h
	${CC} ${CCFLAGS} -c scanner_opencl.c

counter.o: counter.c counter.h version.h
	${CC} ${CCFLAGS} -c counter.c

test/test_bitstring: test/test_bitstring.c libsdm.a
	${CC} ${CCFLAGS} -I. libsdm.a test/test_bitstring.c -o test/test_bitstring

#-------- OLD ----

lib/sha256.o: lib/sha256.cpp
	(cd lib/ ; ${CXX} -c sha256.cpp)

test_bitstring: bitstring.o test_bitstring.cpp lib/base64.o
	${CXX} ${CXXFLAGS} lib/base64.o bitstring.o test_bitstring.cpp -o test_bitstring

test_scanner_opencl: test_scanner_opencl.cpp $(OBJS)
	${CXX} ${CXXFLAGS} ${CLFLAGS} ${OBJS} ${LIB_OBJS} test_scanner_opencl.cpp -o test_scanner_opencl

.PHONY: clean
clean:
	rm -f *.o lib/*.o test_bitstring test_scanner_opencl
	rm -f libsdm.a libsdm.so test1 test1-dyn test2 test3 test4
	rm -f test/test_bitstring
