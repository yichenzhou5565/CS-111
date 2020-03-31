# NAME: Yichen Zhou
# EMAIL: yichenzhou@g.ucla.edu
# ID: 205140928

MAKEFLAGS += --silent
default:
	gcc -Wall -Wextra -g lab0.c -o lab0

clean:
	rm -rf *.o lab0 *.tar.gz *.txt

dist:
	tar -czvf lab0-205140928.tar.gz lab0.c Makefile README backtrace.png breakpoint.png

check:
	@gcc -Wall -Wextra -g lab0.c -o lab0;
	@echo "CS 111" >in.txt;

	@./lab0 --input=in.txt --output=out.txt;
	@if [[ $$? -eq 0 ]];\
	then echo "SUCCESS in file/file exit code test";\
	fi
	@if [[ $$? -ne 0 ]];\
	then echo "FAIL in file/file exit code test";\
	fi
	@diff --brief in.txt out.txt;
	@if [[ $$? -eq 0 ]];\
	then echo "SUCCESS in file/file content test.";\
	fi
	@if [[ $$? -ne 0 ]];\
	then echo "FAIL in file/file content test.";\
	fi

	@./lab0 --input=in.txt >out.txt;
	@if [[ $$? -eq 0 ]];\
	then echo "SUCCESS in file/output exit code test";\
	fi
	@if [[ $$? -ne 0 ]];\
	then echo "FAIL in file/output exit code test";\
	fi
	@diff --brief in.txt out.txt;
	@if [[ $$? -eq 0 ]];\
	then echo "SUCCESS in file/output content test.";\
	fi
	@if [[ $$? -ne 0 ]];\
	then echo "FAIL in file/output content test.";\
	fi

	@echo "Paul Eggert lol" | ./lab0 --output=out.txt;
	@echo "Paul Eggert lol" > in.txt;
	@if [[ $$? -eq 0 ]];\
	then echo "SUCCESS in input/file exit code test";\
	fi
	@if [[ $$? -ne 0 ]];\
	then echo "FAIL in input/file exit code test";\
	fi
	@diff --brief in.txt out.txt;
	@if [[ $$? -eq 0 ]];\
	then echo "SUCCESS in input/file content test.";\
	fi
	@if [[ $$? -ne 0 ]];\
	then echo "FAIL in input/file content test.";\
	fi

	@echo "David Smallberg :)" >in.txt;
	@echo "David Smallberg :)" | ./lab0 >out.txt;
	@if [[ $$? -eq 0 ]];\
	then echo "SUCCESS in input/output exit code test";\
	fi
	@if [[ $$? -ne 0 ]];\
	then echo "FAIL in input/output exit code test";\
	fi
	diff --brief in.txt out.txt;
	@if [[ $$? -eq 0 ]];\
	then echo "SUCCESS in input/output exit code test";\
	fi
	@if [[ $$? -ne 0 ]];\
	then echo "FAIL in input/output exit code test";\
	fi


