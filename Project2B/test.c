#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

const int BILLION = 1000000000;

//declare global variables
long long counter = 0;
pthread_mutex_t* mutexes;
int* spinLocks;
SortedList_t* head;
SortedListElement_t* ele;
char yieldopts[10] = "-";
long long lock_time = 0;
int* which_list;

//declare argument flags
int threads_flag;
int iterations_flag;
int yield_flag = 0;
int sync_flag;
int head_flag;

//declare argument values
int num_threads = 1;
int num_iterations = 1;
int num_ele;
int num_head = 1;
int lock_type = 0; //default no lock
int opt_yield = 0;

//declare lock type flags
const int NONE = 0;
const int MUTEX = 1;
const int SPIN_LOCK = 2;

void setYield(char* optarg){
    int len = strlen(optarg);
    if (len > 3){
        fprintf(stderr, "too many arguments to yield\n");
        exit(1);
    }
    
    int i = 0;
    for (; i < len; i++){
        switch (optarg[i]){
            case 'i': opt_yield |= INSERT_YIELD; strcat(yieldopts, "i"); break;
            case 'd': opt_yield |= DELETE_YIELD; strcat(yieldopts, "d"); break;
            case 'l': opt_yield |= LOOKUP_YIELD; strcat(yieldopts, "l"); break;
            default:
                fprintf(stderr, "invalid argument to yield!\n");
                exit(1);
        }
    }
}

void handler(){
    fprintf(stderr, "error: segmentation fault\n");
    exit(2);
}

int hash(const char* key) {
    //int h = 'f' % num_head;
    return (key[0] % num_head);
}

void deleteElement(SortedListElement_t* element){
    if (element == NULL){
        printf("f ");
        fprintf(stderr, "error on SortedList_lookup\n");
        exit(2);
    }
    
    if (SortedList_delete(element)){
        printf("f ");
        fprintf(stderr, "error on SortedList_delete\n");
        exit(2);
    }
}

void* listWork(void* tid){
    struct timespec lock_start;
    struct timespec lock_end;
    
    //insert
    int i = *(int*)tid;
    //printf("%d\n", i);
    for (; i < num_ele; i += num_threads){
        switch (lock_type){
            case 0: //none
                SortedList_insert(&head[which_list[i]], &ele[i]);
                break;
            case 1: //mutex
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) == -1){
                    fprintf(stderr, "error starting mutex timing");
                    exit(1);
                }
                pthread_mutex_lock(&mutexes[which_list[i]]);
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) == -1){
                    fprintf(stderr, "error ending mutex timing");
                    exit(1);
                }
                lock_time += BILLION * (lock_end.tv_sec - lock_start.tv_sec) + lock_end.tv_nsec - lock_start.tv_nsec;//timeElapsed(lock_end.tv_sec, lock_start.tv_sec, lock_end.tv_nsec, lock_start.tv_nsec);
                //printf("lock time is %lld\n", lock_time);
                SortedList_insert(&head[which_list[i]], &ele[i]);
                pthread_mutex_unlock(&mutexes[which_list[i]]);
                break;
            case 2: //spin lock (test and set)
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) == -1){
                    fprintf(stderr, "error starting spin lock timing\n");
                    exit(1);
                }
                while (__sync_lock_test_and_set(&spinLocks[which_list[i]], 1));
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) == -1){
                    fprintf(stderr, "error ending spin lock timing\n");
                    exit(1);
                }
                lock_time += BILLION * (lock_end.tv_sec - lock_start.tv_sec) + lock_end.tv_nsec - lock_start.tv_nsec; //timeElapsed(lock_end.tv_sec, lock_start.tv_sec, lock_end.tv_nsec, lock_start.tv_nsec);
                //printf("lock time is %lld\n", lock_time);
                SortedList_insert(&head[which_list[i]], &ele[i]);
                //printf("the length after insert is %d for i = %d\n", SortedList_length(&head[which_list[i]]), i);
                __sync_lock_release(&spinLocks[which_list[i]]);
                break;
        }
    }
    //get length
    int len = 0;
    switch (lock_type){
        case 0: //none
            for (i = 0; i < num_head; i++){
                int sub_len = SortedList_length(&head[i]);
                if (sub_len == -1){
                    printf("f ");
                    fprintf(stderr, "error on SortedList_length\n");
                    exit(2);
                }
                len += sub_len;
            }
            break;
        case 1: //mutex
            for (i = 0; i < num_head; i++){
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) == -1){
                    fprintf(stderr, "error starting mutex timing");
                    exit(1);
                }
                pthread_mutex_lock(&mutexes[i]);
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) == -1){
                    fprintf(stderr, "error ending mutex timing");
                    exit(1);
                }
                lock_time += BILLION * (lock_end.tv_sec - lock_start.tv_sec) + lock_end.tv_nsec - lock_start.tv_nsec; //timeElapsed(lock_end.tv_sec, lock_start.tv_sec, lock_end.tv_nsec, lock_start.tv_nsec);
                //printf("lock time is %lld\n", lock_time);
                int sub_len = SortedList_length(&head[i]);
                if (sub_len == -1){
                    fprintf(stderr, "error on SortedList_length\n");
                    exit(2);
                }
                len += sub_len;
                pthread_mutex_unlock(&mutexes[i]);
            }
            break;
        case 2: //spin lock (test and set)
            for (i = 0; i < num_head; i++){
                //printf("i is %d\n", i);
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) == -1){
                    fprintf(stderr, "error starting spin lock timing\n");
                    exit(1);
                }
                while (__sync_lock_test_and_set(&spinLocks[i], 1));
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) == -1){
                    fprintf(stderr, "error ending spin lock timing\n");
                    exit(1);
                }
                lock_time += BILLION * (lock_end.tv_sec - lock_start.tv_sec) + lock_end.tv_nsec - lock_start.tv_nsec; //timeElapsed(lock_end.tv_sec, lock_start.tv_sec, lock_end.tv_nsec, lock_start.tv_nsec);
                
                //printf("lock time is %lld\n", lock_time);
                int sub_len = SortedList_length(&head[i]);
                if (sub_len == -1){
                    printf("f ");
                    fprintf(stderr, "error on SortedList_length\n");
                    exit(2);
                }
                len += sub_len;
                __sync_lock_release(&spinLocks[i]);
            }
            break;
    }
    
    //delete old ele
    SortedListElement_t* element;
    for (i = *(int*)tid; i < num_ele; i += num_threads){
        switch (lock_type){
            case 0: //none
                element = SortedList_lookup(&head[which_list[i]], ele[i].key);
                deleteElement(element);
                break;
            case 1: //mutex
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) == -1){
                    fprintf(stderr, "error starting mutex timing");
                    exit(1);
                }
                pthread_mutex_lock(&mutexes[which_list[i]]);
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) == -1){
                    fprintf(stderr, "error ending mutex timing");
                    exit(1);
                }
                lock_time += BILLION * (lock_end.tv_sec - lock_start.tv_sec) + lock_end.tv_nsec - lock_start.tv_nsec; //timeElapsed(lock_end.tv_sec, lock_start.tv_sec, lock_end.tv_nsec, lock_start.tv_nsec);
                //printf("lock time is %lld\n", lock_time);
                element = SortedList_lookup(&head[which_list[i]], ele[i].key);
                deleteElement(element);
                pthread_mutex_unlock(&mutexes[which_list[i]]);
                break;
            case 2: //spin lock (test and set)
                if (clock_gettime(CLOCK_MONOTONIC, &lock_start) == -1){
                    fprintf(stderr, "error starting spin lock timing\n");
                    exit(1);
                }
                while (__sync_lock_test_and_set(&spinLocks[which_list[i]], 1));
                if (clock_gettime(CLOCK_MONOTONIC, &lock_end) == -1){
                    fprintf(stderr, "error ending spin lock timing\n");
                    exit(1);
                }
                lock_time += BILLION * (lock_end.tv_sec - lock_start.tv_sec) + lock_end.tv_nsec - lock_start.tv_nsec;//timeElapsed(lock_end.tv_sec, lock_start.tv_sec, lock_end.tv_nsec, lock_start.tv_nsec);
                //printf("lock time is %lld\n", lock_time);
                element = SortedList_lookup(&head[which_list[i]], ele[i].key);
                deleteElement(element);
                __sync_lock_release(&spinLocks[which_list[i]]);
                break;
        }
    }
    return NULL; //function must be void* to fit pthread_create argument types
}

char tag[50] = "list";
void getTag(){
    if (strlen(yieldopts) == 1)
        strcat(yieldopts, "none");
    
    strcat(tag, yieldopts);
    
    switch (lock_type){
        case 0: strcat(tag, "-none"); break;
        case 1: strcat(tag, "-m"); break;
        case 2: strcat(tag, "-s"); break;
    }
}

int main(int argc, char **argv){
    //process options with getopt_long
    static struct option long_options[] = {
        {"threads",    required_argument, &threads_flag,    1},
        {"iterations",    required_argument, &iterations_flag,     1},
        {"yield",    required_argument, &yield_flag,        1},
        {"sync",    required_argument, &sync_flag,        1},
        {"lists",    required_argument, &head_flag,        1}
    };
    //var to store index of command that getopt finds
    int opt_index;
    
    int c;
    while (1){
        c = getopt_long(argc, argv, "", long_options, &opt_index);
        if (c == -1)
            break;
        if (c != 0){
            fprintf(stderr, "error: invalid argument\n");
            exit(1);
        }
        
        if (opt_index == 0){
            num_threads = atoi(optarg);
        }
        if (opt_index == 1){
            num_iterations = atoi(optarg);
        }
        if (opt_index == 2){
            setYield(optarg);
        }
        if (opt_index == 3 && strlen(optarg) == 1){
            switch (optarg[0]){
                case 'm':
                    lock_type = MUTEX; break;
                case 's':
                    lock_type = SPIN_LOCK; break;
                default:
                    fprintf(stderr, "error: invalid sync argument\n");
                    exit(1);
            }
        }
        if (opt_index == 4){
            num_head = atoi(optarg);
        }
    }
    
    signal(SIGSEGV, handler);
    
    //initialize locks
    if (lock_type == MUTEX){
        mutexes = malloc(num_head*sizeof(pthread_mutex_t));
        int i = 0;
        for (; i < num_head; i++)
            pthread_mutex_init(&mutexes[i], NULL);
    }
    else if (lock_type == SPIN_LOCK){
        spinLocks = malloc(num_head * sizeof(int));
        int i = 0;
        for (; i < num_head; i++)
            spinLocks[i] = 0;
    }
    //initializeHead();
    head = malloc(num_head * sizeof(SortedList_t));
    int i = 0;
    for (; i < num_head; i++) {
        head[i].key = NULL;
        head[i].next = &head[i];
        head[i].prev = &head[i];
    }
    //initializeKeys();
    num_ele = num_threads * num_iterations;
    ele = malloc(num_ele*sizeof(SortedListElement_t));
    srand(time(NULL));
    i = 0;
    for (; i < num_ele; i++){
        int rand_char = (rand() % 26) + 'a';
        
        char* rand_key = malloc(2*sizeof(char));
        rand_key[0] = rand_char;
        rand_key[1] = '\0';
        
        ele[i].key = rand_key;
    }
    
    which_list = malloc(num_ele*sizeof(int));
    i = 0;
    for (; i < num_ele; i++)
        which_list[i] = hash(ele[i].key);
    
    //create threads
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    int* tids = malloc(num_threads * sizeof(int));
    
    //start timing
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    //run threads
    i = 0;
    for (; i < num_threads; i++){
        tids[i] = i;
        if (pthread_create(threads + i, NULL, listWork, &tids[i])){
            fprintf(stderr, "error on pthread_create\n");
            exit(1);
        }
    }
    //wait for threads
    for (i = 0; i < num_threads; i++){
        int ret = pthread_join(*(threads + i), NULL);
        if (ret){
            fprintf(stderr, "error on pthread_join\n");
            exit(1);
        }
    }
    //stop timing
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    //free allocated memory
    free(ele);
    free(tids);
    free(mutexes);
    
    //calculate metrics
    //long long elapsed_time = timeElapsed(end_time.tv_sec, start_time.tv_sec, end_time.tv_nsec, start_time.tv_nsec);
    int num_operations = num_threads * num_iterations * 3;
    long long elapsed_time = BILLION * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_nsec - start_time.tv_nsec;
    int avg_op_time = elapsed_time / num_operations;
    long avg_lock_time = lock_time / num_operations;
    
    //report data
    getTag();
    printf("%s,%d,%d,%d,%d,%lld,%d,%ld\n", tag, num_threads, num_iterations, num_head, num_operations, elapsed_time, avg_op_time, avg_lock_time);
    
    //make sure head are all empty
    for (i = 0; i < num_head; i++){
        if (SortedList_length(&head[i]) != 0){
            fprintf(stderr, "nonzero list length!\n");
            exit(2);
        }
    }
    
    //successful exit, finally
    exit(0);
}
