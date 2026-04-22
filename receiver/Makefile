all:
	gcc -Wall echo_raw.c socket.c -o echo_raw
	gcc -Wall kermit.c socket.c recv.c -o recv

clean:
	rm echo_raw && rm recv 
