
all: bitstring.o address_space.o scanner.o scanner_thread.o utils.o protocol.o server.o server

tests: test_bitstring test_address_space test_scanner test_scanner_thread test_counter

lib/base64.o: lib/base64.cpp
	(cd lib/ ; g++ -c base64.cpp)

lib/sha256.o: lib/sha256.cpp
	(cd lib/ ; g++ -c sha256.cpp)

test_bitstring: bitstring.o test_bitstring.cpp lib/base64.o
	g++ lib/base64.o bitstring.o test_bitstring.cpp -o test_bitstring

test_address_space: bitstring.o address_space.o test_address_space.cpp lib/sha256.o utils.o lib/base64.o
	g++ utils.o lib/sha256.o lib/base64.o bitstring.o address_space.o test_address_space.cpp -o test_address_space

test_scanner: address_space.o bitstring.o scanner.o test_scanner.cpp lib/sha256.o utils.o lib/base64.o
	g++ utils.o lib/sha256.o lib/base64.o bitstring.o address_space.o scanner.o test_scanner.cpp -o test_scanner

test_scanner_thread: address_space.o bitstring.o scanner.o scanner_thread.o test_scanner_thread.cpp lib/sha256.o utils.o
	g++ utils.o lib/sha256.o lib/base64.o bitstring.o address_space.o scanner.o scanner_thread.o test_scanner_thread.cpp -o test_scanner_thread

test_counter: test_counter.cpp lib/base64.o
	g++ lib/base64.o test_counter.cpp -o test_counter

bitstring.o: bitstring.h bitstring.cpp
	g++ -c bitstring.cpp

address_space.o: address_space.h address_space.cpp bitstring.h
	g++ -c address_space.cpp

scanner.o: scanner.h scanner.cpp
	g++ -c scanner.cpp

scanner_thread.o: scanner_thread.h scanner_thread.cpp
	g++ -c scanner_thread.cpp

utils.o: utils.h utils.cpp
	g++ -c utils.cpp

protocol.o: protocol.cpp
	g++ -c protocol.cpp

server.o: server.cpp server.h
	g++ -c server.cpp

server: server.o address_space.o bitstring.o scanner.o scanner_thread.o test_scanner_thread.cpp lib/sha256.o utils.o lib/base64.o
	g++ utils.o server.o lib/sha256.o lib/base64.o bitstring.o address_space.o scanner.o scanner_thread.o -o server

clean:
	rm -f *.o lib/*.o test_address_space test_bitstring test_scanner test_scanner_thread test_counter server
