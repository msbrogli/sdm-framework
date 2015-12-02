
all: bitstring.o address_space.o

tests: test_bitstring test_address_space

test_bitstring: bitstring.o test_bitstring.cpp
	g++ bitstring.o test_bitstring.cpp -o test_bitstring

test_address_space: bitstring.o address_space.o test_address_space.cpp
	g++ ../lib/base64.o bitstring.o address_space.o test_address_space.cpp -o test_address_space

bitstring.o: bitstring.h bitstring.cpp
	g++ -c bitstring.cpp

address_space.o: address_space.h address_space.cpp bitstring.h
	g++ -c -I../lib/ address_space.cpp

clean:
	rm -f *.o test_address_space test_bitstring
