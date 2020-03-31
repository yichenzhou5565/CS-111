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

/* Global Constant */
const long coefficient = 1000000000;

/* Global Variables */
long thr_num;
long itr_num;
char opt_syn;
long ele_num;
int list_num;
int* which_list;
pthread_mutex_t* mutex_lock;
int* spin_lock;
SortedListElement_t *ele;
SortedList_t* head;
long long waiting_time = 0;

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
    list_num = 1;
    opt_syn = 0x0;
    opt_yield = 0;      /* Defined as extern in the header file */
    int i=0;
    /* initialize the entire list */
    
    
    struct option options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {"lists", required_argument, NULL, 'l'},
        {0, 0, 0, 0}
    };
    
    int cur_arg = 0;
    int option_index;
    //i=0;
    for (;;)
    {
        cur_arg = getopt_long(argc, argv, "t:i:y:s:l:", options, &option_index);
        if(cur_arg == -1)
            break;
        switch(cur_arg)
        {
            case 't':
                thr_num =atoi(optarg);
                break;
            case 'i':
                itr_num = atoi(optarg);
                //printf("%ld\n", itr_num);
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
                //printf("%c\n", opt_syn);
                break;
            case 'l':
                list_num = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Please input valid arguments: --threads, --iterations, --yield, --sync.\n");
                exit(1);
        }
    }
    /* Initialize mutex lock */
    if (opt_syn == 'm')
    {
        mutex_lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * list_num);
        if (mutex_lock == NULL)
        {
            fprintf(stderr, "Error in malloc().\n");
            exit(1);
        }
        i = 0;
        while (i < list_num)
        {
            //printf("Init mutx.\n");
            if (pthread_mutex_init(&mutex_lock[i], NULL) == -1)
            {
                fprintf(stderr, "Error in pthread_mutex_init.\n");
                exit(1);
            }
            //printf("%x\n", &mutex_lock[i]);
            i++;
        }
    }
    else if (opt_syn == 's')
    {
        spin_lock = (int*)malloc(sizeof(int) * list_num);
        if (spin_lock == NULL)
        {
            fprintf(stderr, "Error in malloc() subroutine call.\n");
            exit(1);
        }
        i = 0;
        while( i < list_num)
        {
            spin_lock[i] = 0;
            i++;
        }
    }
    
    /* Initialize the lists */
    i=0;
    head = (SortedList_t*)malloc(sizeof(SortedList_t) * list_num);
    while(i < list_num)
    {
        head[i].next = &head[i];
        head[i].prev = &head[i];
        head[i].key  = NULL;
        
        i++;
    }
    
    /* Initialize key */
    /* allocate space for the elements */
    ele_num = thr_num * itr_num;
    ele = (SortedListElement_t*)malloc(ele_num * sizeof(SortedListElement_t));
    if (ele == NULL)
    {
        fprintf(stderr, "Error in malloc() subroutine call.\n");
        exit(1);
    }
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
    
    /* Decide which list */
    which_list = (int*)malloc(sizeof(int) * ele_num);
    i = 0;
    while (i < ele_num)
    {
        which_list[i] = ele[i].key[0] % list_num;
        i++;
    }
    
    /* Allocate space for threads */
    int* tid = (int*)malloc(thr_num * sizeof(int));
    pthread_t* t = (pthread_t*)malloc(thr_num * sizeof(pthread_t));
    if (tid == NULL || t == NULL)
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
        if (pthread_create(&t[i], NULL, add_threads, &tid[i]) != 0)
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
    
    /* Check length of every list is zero */
    i = 0;
    while (i < list_num)
    {
        if (SortedList_length(&head[i]) != 0)
        {
            fprintf(stderr, "Wrong result.\n");
            exit(2);
        }
        i++;
    }
    
    /* Print output */
    output(start, end);
    
    /* Free resources */
    free(tid);
    free(ele);
    free(mutex_lock);
    free(spin_lock);
    
    return (0);
}

/************************** END OF MAIN *************************************/

void seg_handler()
{
    fprintf(stderr, "Segmentation fault caught.\n");
    exit(2);
}

void* add_threads(void* tid)
{
    //printf("HAHA MUTEX!\n");
    void* ret=NULL;
    
    int i = *((int*)tid);
    struct timespec start, end;
    
    /* Insert */
    while(i<ele_num)
    {
        if (opt_syn == 'm')
        {
            //printf("HAHA MUTEX!\n");
            clock_gettime(CLOCK_MONOTONIC, &start);
            pthread_mutex_lock(&mutex_lock[which_list[i]]);
            clock_gettime(CLOCK_MONOTONIC, &end);
            
            waiting_time += coefficient*(end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec);
            //printf("HAHA MUTEX!\n");
            SortedList_insert( &head[which_list[i]], &ele[i] );
            //printf("HAHA MUTEX!\n");
            pthread_mutex_unlock(&mutex_lock[which_list[i]]);
        }
        else if(opt_syn == 's')
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
            while(__sync_lock_test_and_set(&spin_lock[which_list[i]], 1));
            clock_gettime(CLOCK_MONOTONIC, &end);
            
            waiting_time += coefficient*(end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec);
            
            SortedList_insert( &head[which_list[i]], &ele[i] );
            __sync_lock_release(&spin_lock[which_list[i]]);
        }
        else
        {
            SortedList_insert(&head[which_list[i]], &ele[i]);
        }
        i = i + thr_num;
    }
    
    //i = 0;
    /* length */
    //long len = SortedList_length(head);
    long len = 0;
    long this_len = 0;
    if (opt_syn == 'm')
    {
        i = 0;
        //printf("HAHA MUTEX!\n");
        while (i < list_num)
        {
            //printf("HAHA MUTEX!\n");
            clock_gettime(CLOCK_MONOTONIC, &start);
            pthread_mutex_lock(&mutex_lock[i]);
            clock_gettime(CLOCK_MONOTONIC, &end);
            waiting_time += coefficient*(end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec);
            
            this_len = SortedList_length(&head[i]);
            if (this_len == -1)
            {
                fprintf(stdout, "Invalid length. List has already been corrupted.\n");
                exit(2);
            }
            len += this_len;
            
            pthread_mutex_unlock(&mutex_lock[i]);
            
            i++;
        }
        
        
    }
    else if(opt_syn == 's')
    {
        i = 0;
        while (i < list_num)
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
            while(__sync_lock_test_and_set(&spin_lock[i], 1));
            clock_gettime(CLOCK_MONOTONIC, &end);
            waiting_time += coefficient*(end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec);
            
            this_len = SortedList_length(&head[i]);
            if (this_len == -1)
            {
                fprintf(stdout, "Invalid length. List has already been corrupted.\n");
                exit(2);
            }
            len += this_len;
            
            __sync_lock_release(&spin_lock[i]);
            
            i++;
        }
    }
    else
    {
        i = 0;
        while (i < list_num)
        {
            this_len = SortedList_length(&head[i]);
            if (this_len == -1)
            {
                fprintf(stdout, "Invalid length. List has already been corrupted.\n");
                exit(2);
            }
            len += this_len;
            
            i++;
        }
    }
    
    /* Look-up and delete */
    i = *((int*)tid);
    SortedListElement_t* temp;
    while(i < ele_num)
    {
        int k = which_list[i];
        
        if (opt_syn == 'm')
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
            pthread_mutex_lock(&mutex_lock[k]);
            clock_gettime(CLOCK_MONOTONIC, &end);
            waiting_time += coefficient*(end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec);
            
            
            temp = SortedList_lookup(&head[which_list[i]], ele[i].key);
            if(temp == NULL)
            {
                fprintf(stderr, "Look up failed.\n");
                exit(2);
            }
            if (SortedList_delete(temp)!=0)
            {
                fprintf(stderr, "Deletion failed.\n");
                exit(2);
            }
            pthread_mutex_unlock(&mutex_lock[k]);
        }
        else if(opt_syn == 's')
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
            while(__sync_lock_test_and_set(&spin_lock[k], 1));
            clock_gettime(CLOCK_MONOTONIC, &end);
            waiting_time += coefficient*(end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec);
            
            temp = SortedList_lookup(&head[which_list[i]], ele[i].key);
            if(temp == NULL)
            {
                fprintf(stderr, "Look up failed.\n");
                exit(2);
            }
            if (SortedList_delete(temp)!=0)
            {
                fprintf(stderr, "Deletion failed.\n");
                exit(2);
            }
            __sync_lock_release(&spin_lock[k]);
        }
        else
        {
            temp = SortedList_lookup(&head[which_list[i]], ele[i].key);
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
    
    return ret;
}

void output(struct timespec start, struct timespec end)
{
    long sec = end.tv_sec - start.tv_sec;
    long nsec = end.tv_nsec - start.tv_nsec;
    long long time = sec * coefficient + nsec;
    long long operation_num = thr_num * itr_num * 3;
    long avg_time = time / operation_num;
    long avg_wait_time = waiting_time / operation_num;
    
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
            fprintf(stdout, "list-id");
            break;
        case 4:
            fprintf(stdout, "list-l");
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
    
    fprintf(stdout, "%ld,%ld,%d,%lld,%lld,%ld,%ld\n", thr_num, itr_num, list_num, operation_num, time, avg_time, avg_wait_time);
}
