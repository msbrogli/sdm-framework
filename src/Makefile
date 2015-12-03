
all: bitstring.o address_space.o counter.o

tests: test_bitstring test_address_space test_counter

test_bitstring: bitstring.o test_bitstring.cpp
	g++ ../lib/sha256.o ../lib/base64.o bitstring.o test_bitstring.cpp -o test_bitstring

test_address_space: bitstring.o address_space.o test_address_space.cpp
	g++ ../lib/sha256.o ../lib/base64.o bitstring.o address_space.o test_address_space.cpp -o test_address_space

test_counter: counter.o test_counter.cpp
	g++ ../lib/sha256.o ../lib/base64.o counter.o test_counter.cpp -o test_counter

bitstring.o: bitstring.h bitstring.cpp
	g++ -c -I../lib/ bitstring.cpp

address_space.o: address_space.h address_space.cpp bitstring.h
	g++ -c -I../lib/ address_space.cpp

counter.o: counter.h counter.cpp
	g++ -c -I../lib/ counter.cpp

clean:
	rm -f *.o test_address_space test_bitstring test_counter
