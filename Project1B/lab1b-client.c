/**
 NAME: Yichen Zhou
 EMAIL: yichenzhou@g.ucla.edu
 ID: 205140928
 **/

/* CLIENT */
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
void setModeNoneCano();         /** Set terminal mode into non-canonical **/
void restoreModeCano();         /** Restore terminal mode into canonical **/
void clientCopy();              /** Forward input **/

/* Global Variables */
bool log_option = false;        /** Whether --log is specified **/
bool compress_option = false;   /** Whether --ccompress is specified **/
bool port_option = false;       /** Whether --port is specified **/
struct termios termios_cp;      /** Saved copy of initial terminal mode **/
int portno;                     /** Port Number **/
char* logfile=NULL;             /** Filename specified by --log option **/
int sockfd, logfd;              /** File descriptor for socket and log **/
z_stream in;
z_stream out;

int main(int argc, char** argv)
{
    /* Set non-canonical mode */
    setModeNoneCano();
    
    struct option long_options[] = {
        {"log", required_argument, NULL, 'l'},
        {"compress", no_argument, NULL, 'c'},
        {"port", required_argument, NULL, 'p'},
        {0, 0, 0, 0}
    };
    
    int option_index=0;
    for(;;)
    {
        int cur_arg = getopt_long(argc, argv, "l:cp:", long_options, &option_index);
        if (cur_arg == -1)
            break;
        switch(cur_arg)
        {
            case 'l':
                log_option = true;
                logfile = optarg;
                logfd = open(logfile, O_CREAT|O_WRONLY|O_TRUNC, 0666);
                if (logfd < 0)
                {
                    fprintf(stderr, "Error on open() system call. %s\n", strerror(errno));
                    exit(1);
                }
                break;
            case 'c':
                compress_option = true;
                break;
            case 'p':
                port_option = true;
                portno = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Invalid input argument. Please enter one of the following arguments: --log, --compress, --port.\n");
                exit(1);
        }
    }
    
    if (port_option == false)
    {
        fprintf(stderr, "Error: --port option MUST be specified.\n");
        exit(1);
    }
    
    struct hostent *server;
    struct sockaddr_in serv_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "Error in sockect() system call. %s\n", strerror(errno));
        exit(1);
    }
    
    server = gethostbyname("localhost");
    if (server == NULL)
    {
        fprintf(stderr, "Error in gethostbyname() subroutine call.\n");
        exit(1);
    }
    
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy( (char*)&serv_addr.sin_addr.s_addr, (char*)server->h_addr, server->h_length );
    /*instead of simply copying the port number to this field, it is necessary to convert this to network byte order*/
    serv_addr.sin_port = htons(portno);
    
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))<0)
    {
        fprintf(stderr, "Error in connect() system call. %s\n", strerror(errno));
        exit(1);
    }
    
    clientCopy();
    
    return (0);
}

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
}

void clientCopy()
{
    struct pollfd fds[2];
    fds[0].fd = 0;                 /* Read from keyboard stdin */
    fds[1].fd = sockfd;            /* Write to the socket'ed end */
    
    fds[0].events = POLLIN | POLLERR | POLLHUP | POLLRDHUP;
    fds[1].events = POLLIN | POLLERR | POLLHUP | POLLRDHUP;
    
    for(;;)
    {
        if (poll(fds, 2, 0) < 0)
        {
            fprintf(stderr, "Error in poll() system call. %s\n", strerror(errno));
            exit(0);
        }
        
        if ((fds[1].revents & (POLLERR | POLLHUP | POLLRDHUP) ))
        {
            exit(0);
        }
        
        if ((fds[0].revents & POLLIN))      /* Keyboard stdin */
        {
            unsigned char buffer[256];
            unsigned char c_buffer[256];
            int ret = read(0, buffer, sizeof(char) * 256);
            if (ret == -1)
            {
                fprintf(stderr, "Error in read() system call. %s\n", strerror(errno));
                exit(1);
            }
            if (compress_option == false)
            {
                int i=0;
                while (i<ret)
                {
                    if (buffer[i]=='\r' || buffer[i]=='\n')
                    {
                        write(1, "\r\n", 2*sizeof(char));
                        write(sockfd, &buffer[i], sizeof(char));
                        if (log_option == true)
                        {
                            write(logfd, "SENT 1 bytes: ", 13*sizeof(char));
                            write(logfd, &buffer[i], sizeof(char));
                            write(logfd, "\n", sizeof(char));
                        }
                    }
                    else
                    {
                        write(1, &buffer[i], sizeof(char));
                        write(sockfd, &buffer[i], sizeof(char));
                        if (log_option == true)
                        {
                            write(logfd, "SENT 1 bytes: ", 13*sizeof(char));
                            write(logfd, &buffer[i], sizeof(char));
                            write(logfd, "\n", sizeof(char));
                        }
                    }
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
                write(sockfd, &c_buffer, 256-in.avail_out);
                
                /* Write to the log file */
                if (log_option == true)
                {
                    unsigned int i=0;
                    while (i < (256-in.avail_out))
                    {
                        write(logfd, "SENT 1 bytes: ", 13*sizeof(char));
                        write(logfd, &buffer[i], sizeof(char));
                        write(logfd, "\n", sizeof(char));
                        
                        i++;
                    }
                }
                
                /* Free resources */
                deflateEnd(&in);
                /* End of compressing data */
                
                /* Write to stdout */
                int i=0;
                while (i < ret)
                {
                    if (buffer[i]=='\r' || buffer[i]=='\n')
                    {
                        write(1, "\r\n", 2*sizeof(char));
                    }
                    else
                    {
                        write(1, &buffer[i], sizeof(char));
                    }
                    i++;
                }
            }
        }
        
        
        if ((fds[1].revents & POLLIN))      /* socket */
        {
            unsigned char buffer[256], c_buffer[256];
            int ret = read(sockfd, buffer, 256*sizeof(char));
            
            if (ret == 0)           /* EOF */
            {
                exit(0);
            }
            
            if (ret < 0)            /* Error */
            {
                fprintf(stderr, "Error from read() system call. %s\n", strerror(errno));
                exit(1);
            }
            
            /* Write to the log file sepcified */
            if (log_option == true)
            {
                int i=0;
                while (i < ret)
                {
                    write(logfd, "RECEIVED 1 bytes: ", 17*sizeof(char));
                    write(logfd, &buffer[i], sizeof(char));
                    write(logfd, "\n", sizeof(char));
                }
            }
            
            if (compress_option == false)
            {
                
                int i = 0;
                while(i < ret)
                {
                    if (buffer[i]=='\r' || buffer[i]=='\n')
                    {
                        write(1, "\r\n", 2*sizeof(char));
                        if (log_option == true)
                        {
                            write(logfd, "SENT 1 bytes: ", 13*sizeof(char));
                            write(logfd, &buffer[i], sizeof(char));
                            write(logfd, "\n", sizeof(char));
                        }
                    }
                    else
                    {
                        write(1, &buffer[i], sizeof(char));
                        if (log_option == true)
                        {
                            write(logfd, "SENT 1 bytes: ", 13*sizeof(char));
                            write(logfd, &buffer[i], sizeof(char));
                            write(logfd, "\n", sizeof(char));
                        }
                    }
                    
                    i++;
                }
                
                
            }
            
            
            if (compress_option == true)
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
                
                /* Transfer decompressed data to stdout */
                unsigned int i=0;
                while (i < (256-out.avail_out))
                {
                    if (c_buffer[i]=='\r' || c_buffer[i]=='\n')
                    {
                        write(1, "\r\n", 2*sizeof(char));
                    }
                    else
                    {
                        write(1, &buffer[i], sizeof(char));
                    }
                    i++;
                }
                
                /* Free resources */
                inflateEnd(&out);
            }
            
        }
        
    }
    
    
    
}
/* END of clientCopy() */
