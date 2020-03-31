/**
 NAME: Yichen ZHOU
 EMAIL: yichenzhou@g.ucla.edu
 ID: 205140928
 **/

#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

void sig_handler(int sig);

int main(int argc, char* argv[])
{
    /**
     prototype for getopt_long():
     int getopt_long(int argc, char * const argv[],
        const char *optstring,
        const struct option *longopts, int *longindex);
     **/
    char *input = NULL, *output = NULL;
    bool have_segfault = false;
    char* cur = NULL;
    
    int option_index ;
    struct option long_options[] = {
        {"input", required_argument, NULL, 2},
        {"output", required_argument, NULL, 3},
        {"segfault", no_argument, NULL, 0},
        {"catch", no_argument, NULL, 4},
        {0, 0, 0, 0} ///mark the end of options, not a real valid argument
    };
    
    int cur_arg=0;
    for(;;)
    {
        cur_arg = getopt_long(argc, argv, "2:3:04", long_options, &option_index);
        if (cur_arg == -1)
            break;
        switch(cur_arg)
        {
            case 2:
                input = optarg;
                cur = optarg;
                break;
            case 3:
                output = optarg;
                break;
            case 0:
                have_segfault = true;
                break;
            case 4:
                signal(SIGSEGV, sig_handler);
                //exit(4);
                break;
                
                /**
                 unrecognized argument:
                 print out an error message including a correct usage line
                 exit(2) with a return code of 1
                 **/
            default:
                fprintf(stderr, "Invalid argument detected. Please use appropriate arguments: --input=filename, --output=filename, --segfault, --catch.\n");
                exit(1);
        }
    }
    
    if (input!=NULL)
    {
        int fd = open(input, O_RDONLY);
        if (fd < 0)
        {
            fprintf(stderr, "Cannot open %s. %s\n", input, strerror(errno));
            exit(2);
        }
        close(0);
        dup(fd);
        if (fd == -1)
        {
            fprintf(stderr, "Cannot redirect input %s. %s\n", input, strerror(errno));
            exit(2);
        }
        close(fd);
    }
    
    if (output!=NULL)
    {
        int fd = creat(output, S_IRWXU);
        //FILE *fp = NULL;
        //fp = fopen(output, "w");
//        if (fp==NULL)
//        {
//            fprintf(stderr, "Cannot open %s. %s", output, strerror(errno));
//            exit(3);
//        }
        //int fd = fileno(fp);
        if (fd < 0)
        {
            fprintf(stderr, "Cannot open %s. %s\n", output, strerror(errno));
            exit(3);
        }
        close(1);
        dup(fd);
        //printf("%d %d", s, fd);
        if (fd == -1)
        {
            fprintf(stderr, "Cannot redirect output %s. %s\n", output, strerror(errno));
            exit(3);
        }
        close(fd);
    }
    
    if (have_segfault == true)
    {
        int* p = NULL;
        fprintf(stderr, "Seg fault %d. %s\n", *p, strerror(errno));
    }
    
    
    //int cur = getchar();
    //char* result = (char*)malloc(sizeof(char));
    //char* cur = (char*)malloc(sizeof(char));
    char a;
    if (cur == NULL)
    {
        cur = &a;
    }
    int helper = read(0, cur, 1);
    for(;;)
    {
        //putchar((char)cur);
        //cur = getchar();
        if(helper==0)
            return 0;
        else if(helper < 0)
        {
            fprintf(stderr, "Error from read() system call. %s\n", strerror(errno));
            exit(errno);
        }
//        else if(helper==0)
//            exit(0);
        write(1, cur, 1);
        helper = read(0, cur, 1);
    }
    //free(cur);
    //free(result);
    return (0);
    
}

void sig_handler(int sig)
{
    if (sig == SIGSEGV)
    {
        fprintf(stderr, "Segmentation fault caught. %s\n", strerror(errno));
        exit(4);
    }
}

