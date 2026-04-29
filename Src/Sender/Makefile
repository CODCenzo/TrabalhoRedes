all:
	gcc -Wall kermit_builder.c send_archive.c ../Util/socket.c -o send_archive 
	gcc -Wall kermit_builder.c send_simple_msg.c ../Util/socket.c -o send_simple_msg

clean:
	rm send_archive
	rm send_simple_msg
