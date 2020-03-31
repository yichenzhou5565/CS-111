default:
	gcc -lprofiler -g3 -Wextra -Wall -pthread -o lab2_list lab2_list.c SortedList.c

tests: default
	./lab2_list --threads=1 --iterations=1000 --sync=m > lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv

	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=1 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=2 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=4 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=8 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=16 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=1 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=2 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=4 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=8 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=16 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=1 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=2 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=4 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=8 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=16 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=1 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=2 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=4 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=8 >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=16 >> lab2b_list.csv

	./lab2_list --yield=id --lists=4 --threads=1 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=80 --sync=m >> lab2b_list.csv


	./lab2_list --yield=id --lists=4 --threads=1 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=80 --sync=s >> lab2b_list.csv




	./lab2_list --sync=m --iterations=1000 --threads=1 --lists=1 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=1 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=1 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=1 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=2 --lists=1 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=2 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=2 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=2 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=4 --lists=1 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=4 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=4 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=4 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=8 --lists=1 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=8 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=8 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=8 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=12 --lists=1 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=12 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=12 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --iterations=1000 --threads=12 --lists=16 >> lab2b_list.csv

	./lab2_list --sync=s --iterations=1000 --threads=1 --lists=1 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=1 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=1 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=1 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=2 --lists=1 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=2 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=2 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=2 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=4 --lists=1 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=4 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=4 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=4 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=8 --lists=1 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=8 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=8 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=8 --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=12 --lists=1 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=12 --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=12 --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --iterations=1000 --threads=12 --lists=16 >> lab2b_list.csv










profile: default
	rm -f *.gperf
	LD_PRELOAD=/lib64/libprofiler.so CPUPROFILE=./prof.gperf ./lab2_list --iterations=1000 --threads=12 --sync=s
	pprof --text lab2_list prof.gperf >profile.out
	pprof --list=add_threads ./lab2_list ./prof.gperf >>profile.out
	rm -f *.gperf

graphs: tests
	./lab2_list.gp

clean:
	rm -rf *.tar.gz *.o lab2_list

dist: graphs profile
	tar -czvf lab2b-205140928.tar.gz lab2_list.c SortedList.c SortedList.h Makefile README lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2_list.gp profile.out lab2b_list.csv
