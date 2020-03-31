#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>
#include <sched.h>
#include <string.h>


/* Global Variables */
long long counter;               /* Target */
long thr_num;
long itr_num;
int opt_yield;
char opt_syn;
pthread_mutex_t mutex_lock;
int spin_lock;

/* Functions */
void add(long long* pointer, long long value);
void add_mutex(long long* pointer, long long value);
void add_spin(long long* pointer, long long value);
void add_compareSwap(long long* pointer, long long value);
void* add_threads();
void output(struct timespec start, struct timespec end);

int main(int argc, char** argv)
{
    thr_num = 1;
    itr_num = 1;
    opt_syn = 0x0;
    opt_yield = 0;
    
    struct option options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", no_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {0, 0, 0, 0}
    };
    
    int cur_arg = 0;
    int option_index;
    for(;;)
    {
        cur_arg = getopt_long(argc, argv, "t:i:ys:", options, &option_index);
        if (cur_arg == -1)
            break;
        switch(cur_arg)
        {
            case 't':
                thr_num = atoi(optarg);
                break;
            case 'i':
                itr_num = atoi(optarg);
                break;
            case 'y':
                opt_yield = 1;
                break;
            case 's':
                opt_syn = optarg[0];
                break;
            default:
                fprintf(stderr, "Please input valid arguments: --threads, --iterations, --yield, --sync.\n");
                exit(1);
        }
    }
    
    /* Initialize counter */
    counter = 0;
    
    struct timespec start, end;
    
    /* Get start time */
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_t* tid = (pthread_t*)malloc(thr_num * sizeof(pthread_t));
    if (tid == NULL)
    {
        fprintf(stderr, "Error in malloc() subroutine call.\n");
        exit(1);
    }
    int i;
    /* initialize mutex */
    if (opt_syn == 'm')
    {
        if (pthread_mutex_init(&mutex_lock, NULL) == -1)
        {
            fprintf(stderr, "Error in pthread_mutex_init().\n");
             exit(1);
        }
    }
    /* Create threads */
    for(i=0; i<thr_num; i++)
    {
        if (pthread_create(&tid[i], NULL, &add_threads, NULL) != 0)
        {
            fprintf(stderr, "Error in pthread_create(). %s\n", strerror(errno));
            exit(1);
        }
    }
    /* Join the threads */
    for(i=0; i<thr_num; i++)
    {
        if(pthread_join(tid[i], NULL) == -1)
        {
            fprintf(stderr, "Error in pthread_join().\n");
            exit(1);
        }
    }
    /* Get end time */
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    /* Print output */
    output(start, end);
    
    free(tid);
    return (0);
}

void* add_threads()
{
    void* ret=NULL;
    long i=0;
    while(i<itr_num)
    {
        switch(opt_syn)
        {
            case 'm':
                add_mutex(&counter, 1);
                add_mutex(&counter, -1);
                break;
            case 's':
                add_spin(&counter, 1);
                add_spin(&counter, -1);
                break;
            case 'c':
                add_compareSwap(&counter, 1);
                add_compareSwap(&counter, -1);
                break;
            default:
                add(&counter, 1);
                add(&counter, -1);
        }
        i++;
    }
    return ret;
}


void add(long long* pointer, long long value)
{
    /* From spec */
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

void add_mutex(long long* pointer, long long value)
{
    pthread_mutex_lock(&mutex_lock);
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
    pthread_mutex_unlock(&mutex_lock);
    
}

void add_spin(long long* pointer, long long value)
{
    while(__sync_lock_test_and_set(&spin_lock, 1));
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
    __sync_lock_release(&spin_lock);
}


void add_compareSwap(long long* pointer, long long value)
{
    long long temp = counter;
    do
    {
        temp = counter;
        if (opt_yield)
            sched_yield();
    } while(__sync_val_compare_and_swap(pointer, temp, temp+value)!=temp);
}

void output(struct timespec start, struct timespec end)
{
    long sec = end.tv_sec - start.tv_sec;
    long nsec = end.tv_nsec - start.tv_nsec;
    const long coefficient = pow(10, 9);
    long long time = sec*coefficient + nsec;
    long long operation_num = thr_num * itr_num * 2;
    long avg_time = time / operation_num;
    
    if (opt_yield == 1)
    {
        switch(opt_syn)
        {
            case 'm':
                fprintf(stdout,"add-yield-m,%ld,%ld,%lld,%lld,%ld,%lld\n", thr_num, itr_num, operation_num, time, avg_time, counter);
                break;
            case 's':
                fprintf(stdout,"add-yield-s,%ld,%ld,%lld,%lld,%ld,%lld\n", thr_num, itr_num, operation_num, time, avg_time, counter);
                break;
            case 'c':
                fprintf(stdout,"add-yield-c,%ld,%ld,%lld,%lld,%ld,%lld\n", thr_num, itr_num, operation_num, time, avg_time, counter);
                break;
            default:
                fprintf(stdout,"add-yield-none,%ld,%ld,%lld,%lld,%ld,%lld\n", thr_num, itr_num, operation_num, time, avg_time, counter);
        }
    }
    else
    {
        switch(opt_syn)
        {
            case 'm':
                fprintf(stdout,"add-m,%ld,%ld,%lld,%lld,%ld,%lld\n", thr_num, itr_num, operation_num, time, avg_time, counter);
                break;
            case 's':
                fprintf(stdout,"add-s,%ld,%ld,%lld,%lld,%ld,%lld\n", thr_num, itr_num, operation_num, time, avg_time, counter);
                break;
            case 'c':
                fprintf(stdout,"add-c,%ld,%ld,%lld,%lld,%ld,%lld\n", thr_num, itr_num, operation_num, time, avg_time, counter);
                break;
            default:
                fprintf(stdout,"add-none,%ld,%ld,%lld,%lld,%ld,%lld\n", thr_num, itr_num, operation_num, time, avg_time, counter);
        }
    }
    
}
