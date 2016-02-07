
CC=cc
CCFLAGS=-O2 -Wall -Wno-gnu -pedantic -DSDM_USE_BITCOUNT_TABLE -fPIC

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
tests: test_bitstring \
	   test_address_space \
	   test_scanner \
	   test_scanner_thread \
	   test_counter \
	   test_scanner_opencl \
	   test_performance_scanners

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

lib/base64.o: lib/base64.c lib/base64.h
	(cd lib/ ; ${CC} ${CCFLAGS} -c base64.c)

operations.o: operations.c operations.h
	${CC} ${CCFLAGS} -c operations.c

address_space.o: address_space.h address_space.c
	${CC} ${CCFLAGS} -c address_space.c

bitstring.o: bitstring.h bitstring.c
	${CC} ${CCFLAGS} -c bitstring.c

scanner_thread.o: scanner_thread.c
	${CC} ${CCFLAGS} -c scanner_thread.c

scanner_opencl.o: scanner_opencl.c scanner_opencl.h
	${CC} ${CCFLAGS} -c scanner_opencl.c

counter.o: counter.c counter.h
	${CC} ${CCFLAGS} -c counter.c

#-------- OLD ----

lib/sha256.o: lib/sha256.cpp
	(cd lib/ ; ${CXX} -c sha256.cpp)

test_bitstring: bitstring.o test_bitstring.cpp lib/base64.o
	${CXX} ${CXXFLAGS} lib/base64.o bitstring.o test_bitstring.cpp -o test_bitstring

test_address_space: bitstring.o address_space.o test_address_space.cpp lib/sha256.o utils.o
	${CXX} ${CXXFLAGS} utils.o lib/sha256.o lib/base64.o bitstring.o address_space.o test_address_space.cpp -o test_address_space

test_scanner: address_space.o bitstring.o scanner.o test_scanner.cpp lib/sha256.o utils.o
	${CXX} ${CXXFLAGS} utils.o lib/sha256.o lib/base64.o bitstring.o address_space.o scanner.o test_scanner.cpp -o test_scanner

test_scanner_thread: address_space.o bitstring.o scanner.o scanner_thread.o test_scanner_thread.cpp lib/sha256.o utils.o
	${CXX} ${CXXFLAGS} utils.o lib/sha256.o lib/base64.o bitstring.o address_space.o scanner.o scanner_thread.o test_scanner_thread.cpp -o test_scanner_thread

test_scanner_opencl: test_scanner_opencl.cpp $(OBJS)
	${CXX} ${CXXFLAGS} ${CLFLAGS} ${OBJS} ${LIB_OBJS} test_scanner_opencl.cpp -o test_scanner_opencl

test_performance_scanners: test_performance_scanners.cpp $(OBJS) $(LIB_OBJS)
	${CXX} ${CXXFLAGS} ${CLFLAGS} ${OBJS} ${LIB_OBJS} test_performance_scanners.cpp -o test_performance_scanners

test_counter: test_counter.cpp lib/base64.o
	${CXX} ${CXXFLAGS} lib/base64.o test_counter.cpp -o test_counter

scanner.o: scanner.h scanner.cpp lib/base64.h
	${CXX} ${CXXFLAGS} -c scanner.cpp

utils.o: utils.h utils.cpp
	${CXX} ${CXXFLAGS} -c utils.cpp

.PHONY: clean
clean:
	rm -f *.o lib/*.o test_address_space test_bitstring test_scanner test_scanner_thread test_counter test_scanner_opencl test_performance_scanners
	rm -f libsdm.a libsdm.so test1 test1-dyn test2 test3
