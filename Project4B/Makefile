default:
	gcc -lmraa -Wall -Wextra -g3 -lm lab4b.c -o lab4b


clean:
	rm -rf *.o *.tar.gz lab4b *.txt

dist:
	tar -cvzf lab4b-205140928.tar.gz README Makefile lab4b.c

check: default
	./test.sh


