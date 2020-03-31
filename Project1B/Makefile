default:
	gcc -g3 -lz -Wall -Wextra lab1b-client.c -o lab1b-client
	gcc -g3 -lz -Wall -Wextra lab1b-server.c -o lab1b-server

dist:
	tar -czvf lab1b-205140928.tar.gz lab1b-client.c lab1b-server.c Makefile README

clean:
	rm -rf *.o *.tar.gz lab1b-client lab1b-server