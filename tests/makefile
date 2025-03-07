CC=clang
CFLAGS=-Wall -Wextra

run: varint_test.o packet_test.o 
	make valgrind_varint
	make valgrind_packet

valgrind_%: %
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=no ./$<_test

varint: varint_test.o testlib.o _varint.o _assertlib.h
	$(CC) $(CFLAGS) -o varint_test varint_test.o _varint.o testlib.o

packet: packet_test.o testlib.o _packet.o _varint.o _assertlib.h
	$(CC) $(CFLAGS) -o packet_test packet_test.o testlib.o _packet.o _varint.o 

_%.o: ../%.c
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

_%.h: ../%.h
	echo "using header file $<"

clean:
	rm *.o varint_test packet_test vgcore\.*



