
CXX=g++
CXXFLAGS=-O2 -Wall
CLFLAGS=-framework OpenCL

LIB_OBJS=lib/base64.o \
		 lib/sha256.o

OBJS=bitstring.o \
	 address_space.o \
	 scanner.o \
	 scanner_thread.o \
	 scanner_opencl.o \
	 utils.o

HEADERS=bitstring.h \
		address_space.h \
		scanner.h \
		scanner_thread.h \
		scanner_opencl.h \
		utils.h

all: bitstring.o address_space.o scanner.o scanner_thread.o utils.o scanner_opencl.o

tests: test_bitstring \
	   test_address_space \
	   test_scanner \
	   test_scanner_thread \
	   test_counter \
	   test_scanner_opencl \
	   test_performance_scanners

lib/base64.o: lib/base64.cpp
	(cd lib/ ; ${CXX} -c base64.cpp)

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

bitstring.o: bitstring.h bitstring.cpp
	${CXX} ${CXXFLAGS} -c bitstring.cpp

address_space.o: address_space.h address_space.cpp bitstring.h lib/base64.h lib/sha256.h
	${CXX} ${CXXFLAGS} -c address_space.cpp

scanner.o: scanner.h scanner.cpp lib/base64.h
	${CXX} ${CXXFLAGS} -c scanner.cpp

scanner_thread.o: scanner_thread.h scanner_thread.cpp bitstring.h
	${CXX} ${CXXFLAGS} -c scanner_thread.cpp

scanner_opencl.o: scanner_opencl.h scanner_opencl.cpp $(HEADERS)
	${CXX} ${CXXFLAGS} -c scanner_opencl.cpp

utils.o: utils.h utils.cpp
	${CXX} ${CXXFLAGS} -c utils.cpp

clean:
	rm -f *.o lib/*.o test_address_space test_bitstring test_scanner test_scanner_thread test_counter test_scanner_opencl test_performance_scanners
