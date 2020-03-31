/**
 NAME: Yichen Zhou
 EMAIL: yichenzhou@g.ucla.edu
 ID: 205140928
 **/

/**
 General Design:
 Step #1: Set terminal mode into un-canonical
 Step #2: Construct and get option argument
 Step #3: If --shell is specified.. Do some extra forking stuff
 
 Step #4: Do the general copy (no matter --shell is specified or not)
 Step #5: Restore terminal mode
 **/

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Helper Functions */
void setModeNoneCano();         /** Set terminal mode into non-canonical **/
void restoreModeCano();         /** Restore terminal mode into canonical **/
void generalCopy();             /** General copy, for the forth step **/
void child();                   /** For the child process **/
void parent();                  /** Fot the parent process **/

/* Global Variables */
struct termios termios_cp;      /** Saved copy of initial terminal mode **/
bool shell_option = false;      /** Whether --shell is specified **/
int toChild[2];                 /** Pipe from parent to child **/
int toParent[2];                /** Pipe from child to parent **/
int pid;

int main(int argc, char** argv)
{
    /* Step #1 */
    setModeNoneCano();
    
    /* Step #2 */
    //bool debug_option = false;
    struct option long_options[] = {
        {"shell", no_argument, NULL, 's'},
        //{"debug", no_argument, NULL, 'd'},
        {0, 0, 0, 0}
    };
    int option_index=0;
    
    for(;;)
    {
        int cur_arg = getopt_long(argc, argv, "sd", long_options, &option_index);
        if (cur_arg == -1)
            break;
        switch(cur_arg)
        {
            case 's':
                shell_option = true;
                break;
//            case 'd':
//                debug_option = true;
//                break;
            default:
                fprintf(stderr, "Invalid input argument. Please enter one of the following arguments: --debug, --shell.\n");
                exit(1);
        }
    }
    
    /* Step #3 */
    
    /// Create pipe
    if (   pipe( toChild )   == -1
        || pipe( toParent )  == -1  )
    {
        fprintf(stderr, "Error from pipe() system call. %s\n", strerror(errno));
        exit(1);
    }
    
    /// Fork
    if (shell_option == true)
    {
        
        pid = fork(); ///spec: fork the process
        if (pid < 0)
        {
            fprintf(stderr, "Error from fork() system call. %s\n", strerror(errno));
            exit(1);
        }
        else if (pid == 0)       /* Child */
        {
            child();
        }
        else                /* Parent */
        {
            parent();
        }
    }
    
    /* Step #4 */
    if (shell_option == false)
    {
        generalCopy();
    }
    
    
    /* Step #5 */
    //restoreModeCano();
    
    exit(0);
}

/**
 ===========================================================================
 =============================== END OF MAIN ===============================
 ===========================================================================
 **/

void setModeNoneCano()
{
    struct termios termios_p;
    tcgetattr(0, &termios_p);
    atexit(restoreModeCano);
    tcgetattr(0, &termios_cp);  ///get the copy for later use
    
    
    termios_p.c_iflag = ISTRIP;    /* only lower 7 bits    */
    termios_p.c_oflag = 0;         /* no processing    */
    termios_p.c_lflag = 0;         /* no processing    */
    if (tcsetattr(0, TCSANOW, &termios_p) < 0)
    {
        fprintf(stderr, "Error in setting terminal modes.\n");
        ///restoreModeCano();
        exit(1);
    }
}

void restoreModeCano()
{
    if ( tcsetattr(STDIN_FILENO, TCSANOW, &termios_cp) < 0 )
    {
        fprintf(stderr, "Error in restoring terminal modes.\n");
        exit(1);
    }
    int tmp;
    if (shell_option == true)
    {
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
}

void generalCopy()
{
    char buffer[256];
    int ret = read(0, buffer, (sizeof(char))*256);
    char cl[2] = {'\r', '\n'}; ///spec: map received <cr> or <lf> into <cr><lf>
    for(;;)
    {
        if (ret == 0)
            break;
        else if (ret < 0)
        {
            fprintf(stderr, "Error from read() system call. %s\n", strerror(errno));
            exit(1);
        }
        
        int i = 0;
        while(i<ret)
        {
            if (buffer[i]=='\r' || buffer[i]=='\n')
            {
                if( write(1, &cl, 2*sizeof(char)) == -1 )
                {
                    fprintf(stderr, "g1Error from write() system call. %s\n", strerror(errno));
                    exit(1);
                }
                ///continue;
            }
            else if( buffer[i] == 4 )             /* EOF or ^D */
            {
                exit(0);
            }
            
            else
            {
                if (write(1, &buffer[i], sizeof(char)) == -1)
                {
                    fprintf(stderr, "g2Error from write() system call. %s\n", strerror(errno));
                    exit(1);
                }
            }
            i++;
        }
        memset(buffer, 0, 256);
        ret = read(0, buffer, (sizeof(char)) * 256 );
    }
}

void child()
{
    printf("Enter the child process..");
    /* Duplicate */
    close(toChild[1]);      /* Close the write end of toChild */
    close(toParent[0]);     /* Close the read end of toParent */
    
    dup2(toChild[0], 0);    /* Redirect whatever is read as input to stdin */
    dup2(toParent[1], 1);   /* Redirect whatever is written to stdout */
    dup2(toParent[1], 2);
    
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
    //printf("Enter the parent process..");
    close(toParent[1]);
    close(toChild[0]);
    
    //char buffer[256];
//    char cl[2] = {'\r', '\n'}; ///spec: map received <cr> or <lf> into <cr><lf>
//    char l = '\n';
    
    /* copy for --shell option */
    struct pollfd fds[2];
    fds[1].fd = toParent[0]; /* describe the pipe that returns output from the shell */
    fds[0].fd = 0;           /* describing the keyboard (stdin) */
    
    /* Have both pollfds wait for input(POLLIN) or error(POLLHUP, POLLERR) */
    fds[0].events = POLLIN | POLLERR | POLLHUP;
    fds[1].events = POLLIN | POLLERR | POLLHUP;
    int ret;
    for(;;)
    {
        ret = poll(fds, 2, 0);
        if (ret < 0)
        {
            fprintf(stderr, "Error in poll() system call. %s\n", strerror(errno));
            exit(1);
        }
//        if (ret == 0)
//        {
//            fprintf(stderr, "Timed out for poll() system call.\n");
//            exit(0);
//        }
        
        /* Error */
//        if ((fds[0].revents & (POLLERR|POLLHUP)))
//        {
//            fprintf(stderr, "Error caught for POLLERR.\n");
//            exit(0);
//        }
        
        
        /* Input */
        if ((fds[0].revents & POLLIN))        /* stdin */
        {
            char buffer[256];
            int ret_read = read(0, buffer, (sizeof(char))*256);
//            for(;;)
//            {
//                if (ret_read == 0)
//                    break;
                if (ret_read < 0)
                {
                    fprintf(stderr, "Error from read() system call. %s\n", strerror(errno));
                    exit(0);
                }
                int i = 0;
                while(i < ret_read)
                {
                    if (buffer[i] == '\r' || buffer[i]=='\n')
                    {
                        ////printf("0-rn");
                        if( write(1, "\r\n", 2*sizeof(char)) == -1 )
                        {
                            fprintf(stderr, "01Error from write() system call. %s\n", strerror(errno));
                            exit(1);
                        }
                        if( write(toChild[1], "\n", sizeof(char)) == -1)
                        {
                            fprintf(stderr, "02Error from write() system call. %s\n", strerror(errno));
                            exit(1);
                        }
                        //continue;
                    }
                    
                    else if (buffer[i] == 0x03)        /* ^C */
                    {
                        /* spec: kill(2) to send a SIGINT to the shell process */
                        //write(1, "^C", 2);
                        kill(pid, SIGINT);
                    }
                    
                    else if (buffer[i] == 0x04)        /* ^D */
                    {
                        /* close pipe to but continue processing input from  shell */
                        close(toChild[1]);
                    }
                    
                    else
                    {
                        if (write(1, &buffer[i], sizeof(char)) == -1)
                        {
                            fprintf(stderr, "04Error from write() system call. %s\n", strerror(errno));
                            exit(1);
                        }
                        if( write(toChild[1], &buffer[i], sizeof(char)) == -1)
                        {
                            fprintf(stderr, "03Error from write() system call. %s\n", strerror(errno));
                            exit(1);
                        }
                        
                    }
                    //ret_read = read(1, buffer, sizeof(char) * 256);
                    i++;
                }
            //memset(buffer, 0, 256);
//            if (ret_read < 0)
//            {
//                fprintf(stderr, "Error in read() system call. %s\n", strerror(errno));
//                exit(1);
//            }
//            }
        }
        
        /* Input */
        if ((fds[1].revents & POLLIN))        /* shell */
        {
            char buffer[256];
            int ret_read = read(toParent[0], buffer, (sizeof(char))*256);
//            for(;;)
//            {
//                if (ret_read == 0)
//                    break;
                if (ret_read < 0)
                {
                    fprintf(stderr, "Error from read() system call. %s\n", strerror(errno));
                    exit(0);
                }
                int i = 0;
                while( i < ret_read )
                {
                    if (buffer[i] == '\n')
                    {
                        //printf("1-n");
                        if (write(1, "\r\n", 2*sizeof(char)) < 0 )
                        {
                            fprintf(stderr, "11Error in write() system call. %s\n", strerror(errno));
                            exit(1);
                        }
                        //continue;
                    }
                    else
                    {
                        if (write(1, &buffer[i], sizeof(char)) < 0)
                        {
                            fprintf(stderr, "12Error in write() system call. %s\n", strerror(errno));
                            exit(1);
                        }
                    }
                    i++;
                    
                }
                //memset(buffer, 0, 256);
            
//            }
        }
        if ((fds[1].revents & (POLLERR|POLLHUP)))
        {
            //fprintf(stderr, "Error caught for POLLERR.\n");
            exit(0);
        }
    }
}
