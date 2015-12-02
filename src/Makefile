
all: bitstring.o address_space.o

bitstring.o: bitstring.h bitstring.cpp
	gcc -c bitstring.cpp

address_space.o: address_space.h address_space.cpp
	gcc -c address_space.cpp

clean:
	rm -f *.o
