
check_PROGRAMS = test_bitstring test_address_space test_scanner test_scanner_thread test_counter test_scanner_opencl test_performance_scanners

BASIC_SOURCES = \
	scanner.cpp \
	bitstring.cpp \
	address_space.cpp \
	lib/base64.cpp \
	lib/sha256.cpp \
	utils.cpp

SDM_SOURCES = \
	$(BASIC_SOURCES) \
	scanner_thread.cpp \
	scanner_opencl.cpp

test_bitstring_SOURCES = \
	test_bitstring.cpp \
	bitstring.cpp \
	lib/base64.cpp

test_address_space_SOURCES = \
	test_address_space.cpp \
	bitstring.cpp \
	address_space.cpp \
	lib/base64.cpp \
	lib/sha256.cpp \
	utils.cpp

test_scanner_SOURCES = \
	test_scanner.cpp \
	$(BASIC_SOURCES)

test_scanner_thread_SOURCES = \
	test_scanner_thread.cpp \
	scanner_thread.cpp \
	$(BASIC_SOURCES)

test_counter_SOURCES = \
	test_counter.cpp \
	lib/base64.cpp

test_scanner_opencl_SOURCES = \
	test_scanner_opencl.cpp \
	$(SDM_SOURCES)

test_scanner_opencl_CXXFLAGS = -framework OpenCL

test_performance_scanners_SOURCES = \
	test_performance_scanners.cpp \
	$(SDM_SOURCES)

test_performance_scanners_CXXFLAGS = -framework OpenCL
