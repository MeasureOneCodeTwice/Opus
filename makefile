CC=clang
CFLAGS=-Wall -Wextra

valgrind: main
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=no ./main

main: main.o packet.o varint.o assertlib.o
	$(CC) -o $@ $^

packet: packet.o varint.o 
	$(CC) $(CFLAGS) -c $^ -o $@ 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 



clean:
	rm *.o vgcore\.* main



