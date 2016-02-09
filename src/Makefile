
CC=cc
CCFLAGS=-O2 -Wall -Wno-gnu -pedantic -fPIC -std=c99

CCFLAGS += -D SDM_USE_BITCOUNT_TABLE
CCFLAGS += -D SDM_ENABLE_OPENCL

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CCFLAGS += -D OS_LINUX
endif
ifeq ($(UNAME_S),Darwin)
	CCFLAGS += -D OS_OSX
	CLFLAGS += -framework OpenCL
endif


LIB_OBJS += lib/base64.o

OBJS += bitstring.o
OBJS += address_space.o
OBJS += scanner_thread.o
OBJS += scanner_opencl.o
OBJS += counter.o
OBJS += operations.o

HEADERS += bitstring.h
HEADERS += address_space.h
HEADERS += scanner_thread.h
HEADERS += scanner_opencl.h
HEADERS += counter.h
HEADERS += operations.h

.PHONY: all
all: libsdm.a libsdm.so

.PHONY: tests
tests: test/test_bitstring

libsdm.so: ${OBJS} ${LIB_OBJS}
	${CC} ${CLFLAGS} -shared -o libsdm.so ${OBJS} ${LIB_OBJS} -lOpenCL -lpthread -lbsd ~/libbsd-0.4.2/src/.libs/libbsd.so

libsdm.a: ${OBJS} ${LIB_OBJS}
	ar -cvr libsdm.a ${OBJS} ${LIB_OBJS}
	ranlib libsdm.a

test1: test1.c libsdm.a
	${CC} ${CCFLAGS} ${CLFLAGS} -o test1 test1.c libsdm.a -lOpenCL -lpthread -lbsd ~/libbsd-0.4.2/src/.libs/libbsd.so

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
