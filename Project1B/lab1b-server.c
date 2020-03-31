/**
 NAME: Yichen Zhou
 EMAIL: yichenzhou@g.ucla.edu
 ID: 205140928
 **/

/* Server */
#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <zlib.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>

/* Helper Functions */
void child();                       /* Called when runnign child process */
void parent();                      /* Called when running parent process */
void restore();                     /* Perform waitpid */
//void intHandler();                  /* Handle SIGINT */
//void pipeHandler();                 /* Handle SIGPIPE */

/* Global Variables */
bool compress_option = false;       /* --compress */
bool port_option = false;           /* --port= */
int toChild[2];                     /* Pipe from parent -> child */
int toParent[2];                    /* Pipe from child -> parent */
int pid;                            /* Process ID */
int portno;                         /* Port Number */
int sockfd, newsockfd;              /* File descriptor for the socket'ed */
z_stream in;
z_stream out;

int main(int argc, char** argv)
{
//    signal(SIGINT, intHandler);
//    signal(SIGPIPE, pipeHandler);
    atexit(restore);
    struct option long_option[] = {
        {"port", required_argument, NULL, 'p'},
        {"compress", no_argument, NULL, 'c'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    for(;;)
    {
        int cur_arg = getopt_long(argc, argv, "p:c", long_option, &option_index);
        if (cur_arg == -1)
            break;
        switch (cur_arg)
        {
            case 'p':
                port_option = true;
                portno = atoi(optarg);
                break;
            case 'c':
                compress_option = true;
                break;
            default:
                fprintf(stderr, "Invalid input argument. Please enter one of the following argument: --port, --compress\n");
                exit(1);
        }
    }
    
    /* Create soekct */
    struct sockaddr_in serv_addr, cli_addr;
    unsigned int clilen;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "Error in socket() systen call. %s\n", strerror(errno));
        exit(1);
    }
    
    /* Initialization */
    memset((char*) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    /* Bind the socket to the current host and the portno on which it'll run */
    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))<0)
    {
        fprintf(stderr, "Error in bind() system call. %s\n", strerror(errno));
        exit(1);
    }
    
    /* Listen to the connections */
    listen(sockfd, 5);      /* Will not fail as long as bind() succeded */
    
    
    /* Block process until a client connects to the server */
    clilen = sizeof(cli_addr);
    /* wakes up the process when a connection from a client
     has been successfully established */
    newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);
    if (newsockfd < 0)
    {
        fprintf(stderr, "Error in accept() system call. %s\n", strerror(errno));
        exit(1);
    }
    
    /* Create pipe */
    if (   pipe(toChild) == -1
        || pipe(toParent) == -1  )
    {
        fprintf(stderr, "Error from pipe() system call. %s\n", strerror(errno));
        exit(1);
    }
    
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Error from fork() system call. %s\n", strerror(errno));
        exit(1);
    }
    if (pid == 0)           /* Child */
    {
        child();
    }
    else                    /* Parent */
    {
        parent();
    }
    
    
    return (0);
}
/** END of main() **/

//void intHandler()
//{
//    kill(pid, SIGINT);
//}
//
//void pipeHandler()
//{
//    exit(1);
//}

void restore()
{
    close(sockfd);
    close(newsockfd);
    close(toParent[0]);
    close(toChild[1]);
    
    int tmp;
    if (waitpid(pid, &tmp, 0) == -1)
    {
        fprintf(stderr, "Error on waitpid().\n");
        exit(1);
    }
    if (WIFEXITED(tmp))
    {
        fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(tmp), WEXITSTATUS(tmp));
        exit(0);
    }
}


void child()
{
    /* Duplicate */
    close(toChild[1]);      /* Close the write end of toChild */
    close(toParent[0]);     /* Close the read end of toParent */
    
    dup2(toChild[0], 0);    /* Redirect whatever is read as input to stdin */
    dup2(toParent[1], 1);   /* Redirect whatever is written to stdout */
    dup2(toParent[1], 2);   /* Redirect error output */
    
    close(toChild[0]);
    close(toParent[1]);
    
    /* exec */
    char path[] = "/bin/bash";
    char* arg[] = { path, NULL };     /* NULL terminated */
    if (execvp(path, arg) < 0)
    {
        fprintf(stderr, "Error from execvp() system call. %s\n", strerror(errno));
        exit(1);
    }
}




void parent()
{
    close(toParent[1]);
    close(toChild[0]);
    
    struct pollfd fds[2];
    fds[1].fd = toParent[0]; /* describe the pipe that return output from the shell */
    fds[0].fd = newsockfd;           /* describing the keyboard (stdin) */
    
    /* Have both pollfds wait for input(POLLIN) or error(POLLHUP, POLLERR) */
    fds[0].events = POLLIN | POLLERR | POLLHUP;
    fds[1].events = POLLIN | POLLERR | POLLHUP;
    
    //int ret;
    for(;;)
    {
        int tmp;
        
        unsigned char buffer[256], c_buffer[256];
        if (poll(fds, 2, 0) < 0)
        {
            fprintf(stderr, "Error in poll() system call. %s\n", strerror(errno));
            exit(1);
        }
        
        if ((fds[1].revents & (POLLERR|POLLHUP)))   /* Error */
        {
            fprintf(stderr, "POLLERR caught.\n");
            exit(0);
        }
        
        if ( (fds[0].revents & POLLIN) )            /* socket */
        {
            int ret = read(newsockfd, buffer, 256*sizeof(char));
            if (ret == -1)
            {
                fprintf(stderr, "Error in read() system call. %s\n", strerror(errno));
                exit(1);
            }
            if (compress_option == false)
            {
                int i=0;
                while(i < ret)
                {
                    if (buffer[i]=='\r' || buffer[i]=='\n')
                    {
                        write(toChild[1], "\n", sizeof(char));
                    }
                    
                    else if (buffer[i] == 0x03)          /* ^C */
                    {
                        kill(pid, SIGINT);
                    }
                    
                    else if (buffer[i] == 0x04)          /* ^D */
                    {
                        close(toChild[1]);
                    }
                    
                    else
                    {
                        write(toChild[1], buffer, sizeof(char));
                    }
                    
                    i++;
                }
            }
            
            if (compress_option == true)        /* --compress */
            {
                /* Initialization before performing decompress */
                out.zalloc = Z_NULL;
                out.zfree = Z_NULL;
                out.opaque = Z_NULL;
                if (inflateInit(&out) != Z_OK)
                {
                    fprintf(stderr, "Error in inflateInit() subroutine call.\n");
                    exit(1);
                }
                
                /* Update */
                out.next_in = buffer;
                out.avail_in = ret;
                out.next_out = c_buffer;
                out.avail_out = 256;
                
                /* Decompress */
                for(;;)
                {
                    if(inflate(&out, Z_SYNC_FLUSH) != Z_OK)
                    {
                        fprintf(stderr, "Error in inflate() subroutine call.\n");
                        exit(1);
                    }
                    if (out.avail_in <= 0)
                        break;
                }
                
                /* Process decompressed data */
                unsigned int i=0;
                while (i < (256-out.avail_out))
                {
                    if (c_buffer[i]=='\r' || c_buffer[i]=='\n')
                    {
                        write(toChild[1], "\n", sizeof(char));
                    }
                    else if (c_buffer[i] == 0x03)
                    {
                        restore();
                    }
                    else if (c_buffer[i] == 0x04)
                    {
                        restore();
                    }
                    else
                    {
                        write(toChild[1], &buffer[i], sizeof(char));
                    }
                    i++;
                }
                
                /* Free resources */
                inflateEnd(&out);
            }
        }
        
        if ((fds[1].revents & POLLIN))              /* Shell */
        {
            int ret = read(toParent[0], buffer, 256*sizeof(char));
            if (ret == -1)
            {
                fprintf(stderr, "Error in read() system call. %s\n", strerror(errno));
                exit(1);
            }
            
            
            if (compress_option == false)
            {
                int i=0;
                while (i < ret)
                {
                    write(newsockfd, &buffer[i], sizeof(char));
                    i++;
                }
            }
            
            
            if (compress_option == true)
            {
                /* Initialization */
                in.zalloc = Z_NULL;
                in.zfree = Z_NULL;
                in.opaque = Z_NULL;
                if (deflateInit(&in, Z_DEFAULT_COMPRESSION) != Z_OK)
                {
                    fprintf(stderr, "Error in deflateInit() subroutine call.\n");
                    exit(1);
                }
                
                /* Update */
                in.next_in = buffer;
                in.avail_in = ret;
                in.next_out = c_buffer;
                in.avail_out = 256;
                
                /* Compress */
                for(;;)
                {
                    if ( deflate(&in, Z_SYNC_FLUSH) != Z_OK )
                    {
                        fprintf(stderr, "Error in deflate() subroutine call.\n");
                        exit(0);
                    }
                    if(in.avail_in <= 0)
                        break;
                }
                
                /* Transfer compressed data to socket */
                write(newsockfd, &c_buffer, 256-in.avail_out);
                
                /* Free resources */
                deflateEnd(&in);
                
            }
            
            
            
        }
    }
    
}
