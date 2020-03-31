#include "SortedList.h"
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <math.h>


/* Global Variables */
long thr_num;
long itr_num;
char opt_syn;
long ele_num;
pthread_mutex_t mutex_lock;
int spin_lock;
SortedListElement_t *ele;
SortedList_t* head;

/* Helper Functions */
void seg_handler();
void* add_threads(void *tid);
void output(struct timespec start, struct timespec end);

int main(int argc, char** argv)
{
    /* Handle segfault */
    signal(SIGSEGV, seg_handler);
    
    /* Initialization */
    thr_num = 1;
    itr_num = 1;
    opt_syn = 0x0;
    opt_yield = 0;      /* Defined as extern in the header file */
    
    /* initialize the entire list */
    head = (SortedList_t*)malloc(sizeof(SortedList_t));
    head->next = head;      /* Dummy node */
    head->prev = head;
    head->key = NULL;
    
    struct option options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {0, 0, 0, 0}
    };
    
    int cur_arg = 0;
    int option_index;
    int i=0;
    for (;;)
    {
        cur_arg = getopt_long(argc, argv, "t:i:y:s:", options, &option_index);
        if(cur_arg == -1)
            break;
        switch(cur_arg)
        {
            case 't':
                thr_num =atoi(optarg);
                break;
            case 'i':
                itr_num = atoi(optarg);
                break;
            case 'y':
                while((unsigned int)i<strlen(optarg))
                {
                    switch(optarg[i])
                    {
                        case 'i':
                            opt_yield |=  INSERT_YIELD;
                            break;
                        case 'd':
                            opt_yield |=  DELETE_YIELD;
                            break;
                        case 'l':
                            opt_yield |=  LOOKUP_YIELD;
                            break;
                    }
                    i++;
                }
                break;
            case 's':
                opt_syn = optarg[0];
                break;
            default:
                fprintf(stderr, "Please input valid arguments: --threads, --iterations, --yield, --sync.\n");
                exit(1);
        }
    }
    
    /* allocate space for the elements */
    ele_num = thr_num * itr_num;
    ele = (SortedListElement_t*)malloc(ele_num * sizeof(SortedListElement_t));
    if (ele == NULL)
    {
        fprintf(stderr, "Error in malloc() subroutine call.\n");
        exit(1);
    }
    
    /* Initialize key */
    i=0;
    while(i < ele_num)
    {
        char* key = (char*)malloc(sizeof(char) * 2);
        if(key == NULL)
        {
            fprintf(stderr, "Error in malloc() subroutine call.\n");
            exit(1);
        }
        key[0] = rand()%26 + 'a';
        key[1] = '\0';
        ele[i].key = key;
        
        i++;
    }
    
    /* Initialize mutex lock */
    if (opt_syn == 'm')
    {
        if (pthread_mutex_init(&mutex_lock, NULL) == -1)
        {
            fprintf(stderr, "Error in pthread_mutex_init.\n");
            exit(1);
        }
    }
    
    /* Allocate space for threads */
    int* tid = (int*)malloc(thr_num * sizeof(int));
    pthread_t* t = (pthread_t*)malloc(thr_num * sizeof(pthread_t));
    if (tid == NULL || t==NULL)
    {
        fprintf(stderr, "Error in malloc() subroutine call.\n");
        exit(1);
    }
    
    /* Start timer */
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    /* Create threads */
    for(i=0; i<thr_num; i++)
    {
        tid[i] = i;
        if (pthread_create(&t[i], NULL, &add_threads, &tid[i]) != 0)
        {
            fprintf(stderr, "Error in pthread_create().\n");
            exit(1);
        }
    }
    
    /* Join thread back */
    for (i=0; i<thr_num; i++)
    {
        if (pthread_join(t[i], NULL) == -1)
        {
            fprintf(stderr, "Error in pthread_join().\n");
            exit(1);
        }
    }
    
    /* End timer */
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    /* Check length is zero */
    if (SortedList_length(head) != 0)
    {
        fprintf(stderr, "Wrong result.\n");
        exit(2);
    }
    
    /* Print output */
    output(start, end);
    
    /* Free resources */
    free(tid);
    free(ele);
    
    return (0);
}


void seg_handler()
{
    fprintf(stderr, "Segmentation fault caught.\n");
    exit(2);
}

void* add_threads(void* tid)
{
    void* ret=NULL;
    
    int tmp = *((int*)tid);
    
    long i=0;
    
    /* Insert */
    while(tmp<ele_num)
    {
        if (opt_syn == 'm')
        {
            pthread_mutex_lock(&mutex_lock);
            SortedList_insert(head, &ele[tmp]);
            pthread_mutex_unlock(&mutex_lock);
        }
        else if(opt_syn == 's')
        {
            while(__sync_lock_test_and_set(&spin_lock, 1));
            SortedList_insert(head, &ele[tmp]);
            __sync_lock_release(&spin_lock);
        }
        else
        {
            SortedList_insert(head, &ele[tmp]);
        }
        tmp = tmp+thr_num;
    }
    
    i = 0;
    /* length */
    //long len = SortedList_length(head);
    long len;
    if (opt_syn == 'm')
    {
        pthread_mutex_lock(&mutex_lock);
        len = SortedList_length(head);
        pthread_mutex_unlock(&mutex_lock);
    }
    else if(opt_syn == 's')
    {
        while(__sync_lock_test_and_set(&spin_lock, 1));
        len = SortedList_length(head);
        __sync_lock_release(&spin_lock);
    }
    else
    {
        len = SortedList_length(head);
    }
    
    if (len == -1)
    {
        fprintf(stdout, "Invalid length. List has already been corrupted.\n");
        exit(2);
    }
    
    /* Look-up and delete */
    i = *((int*)tid);
    SortedListElement_t* temp;
    while(i<ele_num)
    {
//        temp = SortedList_lookup(head, ele[i].key);
//        if (temp == NULL)
//        {
//            fprintf(stderr, "Element not found.\n");
//            exit(2);
//        }
//        if (SortedList_delete(temp) != 0)
//        {
//            fprintf(stderr, "Deletion failed.\n");
//            exit(2);
//        }
        
        if (opt_syn == 'm')
        {
            pthread_mutex_lock(&mutex_lock);
            temp = SortedList_lookup(head, ele[i].key);
            if(temp == NULL)
            {
                fprintf(stderr, "Look up failed.\n");
                exit(2);
            }
            if (SortedList_delete(temp)!=0)
            {
                fprintf(stderr, "Deletion failed.\n");
            }
            pthread_mutex_unlock(&mutex_lock);
        }
        else if(opt_syn == 's')
        {
            while(__sync_lock_test_and_set(&spin_lock, 1));
            temp = SortedList_lookup(head, ele[i].key);
            if(temp == NULL)
            {
                fprintf(stderr, "Look up failed.\n");
                exit(2);
            }
            if (SortedList_delete(temp)!=0)
            {
                fprintf(stderr, "Deletion failed.\n");
            }
            __sync_lock_release(&spin_lock);
        }
        else
        {
            temp = SortedList_lookup(head, ele[i].key);
            if(temp == NULL)
            {
                fprintf(stderr, "Look up failed.\n");
                exit(2);
            }
            if (SortedList_delete(temp)!=0)
            {
                fprintf(stderr, "Deletion failed.\n");
            }
        }
        
        i = i+thr_num;
    }
    
    /* Unlock, if applicable */
//    if (opt_syn == 'm')
//        pthread_mutex_unlock(&mutex_lock);
//    else if(opt_syn == 's')
//        __sync_lock_release(&spin_lock);
    
    return ret;
}

void output(struct timespec start, struct timespec end)
{
    long sec = end.tv_sec - start.tv_sec;
    long nsec = end.tv_nsec - start.tv_nsec;
    const long coefficient = pow(10, 9);
    long long time = sec * coefficient + nsec;
    long long operation_num = thr_num * itr_num * 3;
    long avg_time = time / operation_num;
    
    switch(opt_yield)
    {
        case 0:
            fprintf(stdout, "list-none");
            break;
        case 1:
            fprintf(stdout, "list-i");
            break;
        case 2:
            fprintf(stdout, "list-d");
            break;
        case 3:
            fprintf(stdout, "list-l");
            break;
        case 4:
            fprintf(stdout, "list-id");
            break;
        case 5:
            fprintf(stdout, "list-il");
            break;
        case 6:
            fprintf(stdout, "list-dl");
            break;
        case 7:
            fprintf(stdout, "list-idl");
            break;
        default:
            fprintf(stdout, "list");
    }
    switch(opt_syn)
    {
        case 'm':
            fprintf(stdout, "-m,");
            break;
        case 's':
            fprintf(stdout, "-s,");
            break;
        default:
            fprintf(stdout, "-none,");
    }
    
    fprintf(stdout, "%ld,%ld,%d,%lld,%lld,%ld\n", thr_num, itr_num, 1, operation_num, time, avg_time);
}
