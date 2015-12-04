
all: bitstring.o address_space.o scanner.o scanner_thread.o

tests: test_bitstring test_address_space test_scanner test_scanner_thread

lib/base64.o: lib/base64.cpp
	(cd lib/ ; g++ -c base64.cpp)

lib/sha256.o: lib/sha256.cpp
	(cd lib/ ; g++ -c sha256.cpp)

test_bitstring: bitstring.o test_bitstring.cpp lib/base64.o
	g++ lib/base64.o bitstring.o test_bitstring.cpp -o test_bitstring

test_address_space: bitstring.o address_space.o test_address_space.cpp lib/sha256.o
	g++ lib/sha256.o lib/base64.o bitstring.o address_space.o test_address_space.cpp -o test_address_space

test_scanner: address_space.o bitstring.o scanner.o test_scanner.cpp lib/sha256.o
	g++ lib/sha256.o lib/base64.o bitstring.o address_space.o scanner.o test_scanner.cpp -o test_scanner

test_scanner_thread: address_space.o bitstring.o scanner.o scanner_thread.o test_scanner_thread.cpp lib/sha256.o
	g++ lib/sha256.o lib/base64.o bitstring.o address_space.o scanner.o scanner_thread.o test_scanner_thread.cpp -o test_scanner_thread

bitstring.o: bitstring.h bitstring.cpp
	g++ -c bitstring.cpp

address_space.o: address_space.h address_space.cpp bitstring.h
	g++ -c address_space.cpp

scanner.o: scanner.h scanner.cpp
	g++ -c scanner.cpp

scanner_thread.o: scanner_thread.h scanner_thread.cpp
	g++ -c scanner_thread.cpp

clean:
	rm -f *.o lib/*.o test_address_space test_bitstring test_scanner test_scanner_thread
