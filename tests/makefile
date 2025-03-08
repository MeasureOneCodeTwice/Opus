CC=clang
CFLAGS=-Wall -Wextra

run: varint_test packet_test 
	make valgrind_varint
	make valgrind_packet
	exit 1

valgrind_%: %_test
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=no ./$<

varint_test: varint_test.o testlib.o varint.o assertlib.o
	$(CC) $(CFLAGS) -o $@ $^

packet_test: packet_test.o testlib.o packet.o varint.o assertlib.o
	$(CC) $(CFLAGS) -o $@ $^

%_test.o: %_test.c
	$(CC) $(CFLAGS) -c -o $@ $< 

%.o: ../src/%.c
	$(CC) $(CFLAGS) -c -o $@ $< 

clean:
	rm *.o varint_test packet_test vgcore\.*



