default:
	gcc -g3 -Wall -Wextra -lm -lmraa lab4c_tcp.c -o lab4c_tcp
	gcc -g3 -Wall -Wextra -lm -lmraa -lssl -lcrypto lab4c_tsl.c -o lab4c_tls

clean:
	rm -rf *.tar.gz *.o lab4c_tsl lab4c_tcp

dist:
	tar -cvzf lab4c-205140928.tar.gz lab4c_tcp.c lab4c_tsl.c README Makefile