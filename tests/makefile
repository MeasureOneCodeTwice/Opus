CC=clang
CFLAGS=-Wall -Wextra -DNDEBUGLOG

run: varint_test packet_test util_packet_body_test
	bash -c ./run_tests.sh

valgrind: util_packet_body_test
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=no ./util_packet_body_test

util_packet_body_test: util_packet_body_test.o util_packet_body.o assertlib.o varint.o assertlib.o testlib.o

varint_test: varint_test.o testlib.o varint.o assertlib.o
	$(CC) $(CFLAGS) -o $@ $^

packet_test: packet_test.o testlib.o packet.o varint.o assertlib.o
	$(CC) $(CFLAGS) -o $@ $^

%_test.o: %_test.c
	$(CC) $(CFLAGS) -c -o $@ $< 

%.o: ../src/%.c
	$(CC) $(CFLAGS) -c -o $@ $< 

clean:
	rm *.o varint_test packet_test util_packet_body_test vgcore\.*



