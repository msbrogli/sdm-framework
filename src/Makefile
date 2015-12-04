
all: bitstring.o address_space.o scanner.o scanner_thread.o

tests: test_bitstring test_address_space test_scanner test_scanner_thread

test_bitstring: bitstring.o test_bitstring.cpp
	g++ ../lib/sha256.o ../lib/base64.o bitstring.o test_bitstring.cpp -o test_bitstring

test_address_space: bitstring.o address_space.o test_address_space.cpp
	g++ ../lib/sha256.o ../lib/base64.o bitstring.o address_space.o test_address_space.cpp -o test_address_space

test_scanner: address_space.o bitstring.o scanner.o test_scanner.cpp
	g++ ../lib/sha256.o ../lib/base64.o bitstring.o address_space.o scanner.o test_scanner.cpp -o test_scanner

test_scanner_thread: address_space.o bitstring.o scanner.o scanner_thread.o test_scanner_thread.cpp
	g++ ../lib/sha256.o ../lib/base64.o bitstring.o address_space.o scanner.o scanner_thread.o test_scanner_thread.cpp -o test_scanner_thread

bitstring.o: bitstring.h bitstring.cpp
	g++ -c -I../lib/ bitstring.cpp

address_space.o: address_space.h address_space.cpp bitstring.h
	g++ -c -I../lib/ address_space.cpp

scanner.o: scanner.h scanner.cpp
	g++ -c scanner.cpp

scanner_thread.o: scanner_thread.h scanner_thread.cpp
	g++ -c scanner_thread.cpp

clean:
	rm -f *.o test_address_space test_bitstring test_scanner test_scanner_thread
