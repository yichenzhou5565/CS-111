#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2_1.png ... Aggregate throughput: total opetations per second for all threads combined
#	lab2_2.png ... Average wait time and time per operation vs. threads #
#	lab2_3.png ... Successful Iterations vs threads
#	lab2_4.png ... 
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# how many threads/iterations we can run without failure (w/o yielding)
set title "List-1: Aggregate Throughput vs Number of Threads"
set xlabel "Threads"
set xrange [1:26]
set logscale x 10
set ylabel "Aggregate Throughput"
set logscale y 10
set output 'lab2b_1.png'

# grep out only single threaded, un-protected, non-yield results
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2_list.csv" using ($2):(1000000000/($7)) \
	title 'spin' with linespoints lc rgb 'green'


set title "List-2: AvgWaitTime and AvgTimerPerOperation vs. number of threads"
set xlabel "Threads"
set logscale x 2
set xrange [1:26]
set ylabel "Time"
set logscale y 10
set output 'lab2b_2.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2_list.csv" using ($2):($7) \
        title 'AvgTimerPerOperation' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2_list.csv" using ($2):($8) \
        title 'AvgWaitTime' with linespoints lc rgb 'green'
     
set title "List-3: Successful Iterations vs Threads"
set logscale x 2
set xrange [0.75:]
set xlabel "Number of Threads"
set ylabel "successful iterations"
set logscale y 10
set output 'lab2b_3.png'
plot \
     "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2_list.csv" using ($2):($3) \
        title 'No locks' with points lc rgb 'red', \
     "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2_list.csv" using ($2):($3) \
        title 'Mutex' with points lc rgb 'green', \
     "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2_list.csv" using ($2):($3) \
        title 'Spin' with points lc rgb 'blue'


# unset the kinky x axis
unset xtics
set xtics

set title "List-4: Scalability of synchronization mechanisms"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Length-adjusted cost per operation(ns)"
set logscale y
set output 'lab2_list-4.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,' lab2_list.csv" using ($2):($7)/(($3)/4) \
	title '(adjusted) list w/mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,' lab2_list.csv" using ($2):($7)/(($3)/4) \
	title '(adjusted) list w/spin-lock' with linespoints lc rgb 'green'
