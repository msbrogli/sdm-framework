
CC=cc
CCFLAGS=-O2 -Wall -Wstrict-prototypes -Wsign-compare -Wno-gnu -pedantic -fPIC -std=c99

CCFLAGS += -D SDM_ENABLE_OPENCL

#CCFLAGS += -D SDM_USE_BITCOUNT_TABLE
CCFLAGS += -D SDM_USE_BUILTIN_POPCOUNT

# Profiling
#CCFLAGS += -g

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CCFLAGS += -D OS_LINUX
	CLFLAGS += -lOpenCL -lpthread -lbsd
endif
ifeq ($(UNAME_S),Darwin)
	CCFLAGS += -D OS_OSX
	CLFLAGS += -framework OpenCL
endif


LIB_OBJS += lib/base64.o

OBJS += bitstring.o
OBJS += address_space.o
OBJS += scanner_thread.o
OBJS += scanner_thread2.o
OBJS += scanner_opencl.o
OBJS += scanner_opencl2.o
OBJS += counter.o
OBJS += operations.o

HEADERS += bitstring.h
HEADERS += address_space.h
HEADERS += scanner_thread.h
HEADERS += scanner_thread2.h
HEADERS += scanner_opencl.h
HEADERS += counter.h
HEADERS += operations.h

.PHONY: all
all: libsdm.a libsdm.so

.PHONY: tests
tests: test1 test2 tests_performance

.PHONY: tests_performance
tests_performance: test_linear test_linear2 test_thread test_thread2 test_opencl test_opencl2

libsdm.so: ${OBJS} ${LIB_OBJS}
	${CC} -shared -o libsdm.so ${OBJS} ${LIB_OBJS} ${CLFLAGS}

libsdm.a: ${OBJS} ${LIB_OBJS}
	ar -cvr libsdm.a ${OBJS} ${LIB_OBJS}
	ranlib libsdm.a

test1: test1.c libsdm.a
	${CC} -o test1 test1.c libsdm.a ${CCFLAGS} ${CLFLAGS}

test2: test2.c libsdm.a
	${CC} -o test2 test2.c libsdm.a ${CCFLAGS} ${CLFLAGS}

test3: test3.c libsdm.a
	${CC} -o test3 test3.c libsdm.a ${CCFLAGS} ${CLFLAGS}

test4: test4.c libsdm.a
	${CC} -o test4 test4.c libsdm.a ${CCFLAGS} ${CLFLAGS}

opencl_debug: opencl_debug.c libsdm.a
	${CC} -o opencl_debug opencl_debug.c libsdm.a ${CCFLAGS} ${CLFLAGS}

test_linear: test_linear.c libsdm.a
	${CC} -o test_linear test_linear.c libsdm.a ${CCFLAGS} ${CLFLAGS}

test_linear2: test_linear2.c libsdm.a
	${CC} -o test_linear2 test_linear2.c libsdm.a ${CCFLAGS} ${CLFLAGS}

test_thread: test_thread.c libsdm.a
	${CC} -o test_thread test_thread.c libsdm.a ${CCFLAGS} ${CLFLAGS}

test_thread2: test_thread2.c libsdm.a
	${CC} -o test_thread2 test_thread2.c libsdm.a ${CCFLAGS} ${CLFLAGS}

test_opencl: test_opencl.c libsdm.a
	${CC} -o test_opencl test_opencl.c libsdm.a ${CCFLAGS} ${CLFLAGS}

test_opencl2: test_opencl2.c libsdm.a
	${CC} -o test_opencl2 test_opencl2.c libsdm.a ${CCFLAGS} ${CLFLAGS}

lib/base64.o: lib/base64.c lib/base64.h
	(cd lib/ ; ${CC} -c base64.c ${CCFLAGS})

operations.o: operations.c operations.h
	${CC} -c operations.c ${CCFLAGS}

address_space.o: address_space.h address_space.c version.h
	${CC} -c address_space.c ${CCFLAGS}

bitstring.o: bitstring.h bitstring.c
	${CC} -c bitstring.c ${CCFLAGS}

scanner_thread.o: scanner_thread.c
	${CC} -c scanner_thread.c ${CCFLAGS}

scanner_thread2.o: scanner_thread2.c
	${CC} -c scanner_thread2.c ${CCFLAGS}

scanner_opencl.o: scanner_opencl.c scanner_opencl.h
	${CC} -c scanner_opencl.c ${CCFLAGS}

scanner_opencl2.o: scanner_opencl2.c scanner_opencl.h
	${CC} -c scanner_opencl2.c ${CCFLAGS}

counter.o: counter.c counter.h version.h
	${CC} -c counter.c ${CCFLAGS}

test/test_bitstring: test/test_bitstring.c libsdm.a
	${CC} -I. libsdm.a test/test_bitstring.c -o test/test_bitstring ${CCFLAGS}

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
	rm -f libsdm.a libsdm.so test1 test1-dyn test2 test3 test4 test_linear test_thread test_opencl test_linear2 test_thread2 test_opencl2
	rm -f test/test_bitstring
