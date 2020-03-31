#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <mraa.h>
#include <poll.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
//#include <bool.h>

/* Global variables */
int period;
char scale;
FILE* file;
int opt_log=0;
int report = 1;
time_t next = 0;

/* aio/gpio variable */
mraa_aio_context temp;
mraa_gpio_context button;

/* Functions */
void command(char* input, int size);
void command_output(char* input);
void turn_off();
int stringcmp(char* str1, char* str2);


int main(int argc, char** argv)
{
    /* Initialization */
    period = 1;
    scale = 'F';
    file = NULL;
    
    button = mraa_gpio_init(60);
    temp = mraa_aio_init(1);
    if (temp==NULL || button==NULL)
    {
        fprintf (stderr, "Error in gpio/aio initialization.\n");
        exit(1);
    }
    mraa_gpio_dir(button, MRAA_GPIO_IN);
    
    struct pollfd fds;
    fds.fd = 0;
    fds.events= POLLIN | POLLERR;
    
    struct option options[] = {
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {0, 0, 0, 0}
    };
    
    int cur_arg = 0;
    int option_index;
    
    for(;;)
    {
        cur_arg = getopt_long(argc, argv, "p:s:l:", options, &option_index);
        if (cur_arg == -1)
            break;
        switch(cur_arg)
        {
            case 'p':
                period = atoi(optarg);
                break;
            case 's':
                if (optarg[0]!='C' && optarg[0]!='F')
                {
                    fprintf(stderr, "Please input valid scale arguments: C, F.\n");
                    exit(1);
                }
                scale = optarg[0];
                break;
            case 'l':
                opt_log = 1;
                file = fopen(optarg, "w");
                if (file == NULL)
                {
                    fprintf(stderr, "Error in fopen().\n");
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "Please input valid arguments: --period, --scale, --log.\n");
                exit(1);
        }
    }
    
    for(;;)
    {
        char buffer[256];
        struct timeval tv;
        struct tm *cur;
        
        /* Get time */
        gettimeofday(&tv, 0);
        if (report==1 && next <= tv.tv_sec)
        {
            cur = localtime(&tv.tv_sec);
            /* Get temperature from the sensor */
            int ret = mraa_aio_read(temp);
            double r;
            double temperature;
            r = 1023.0/((double)ret)-1.0;
            r = r * 100000.0;
            temperature = 1.0/(log(r/100000.0)/4275+1/298.15)-273.15;
            if (scale == 'F')
                temperature = temperature * 9.0 / 5.0 + 32.0;
            /* reports: store in buffer first */
            sprintf(buffer, "%02d:%02d:%02d %.1lf\n", cur->tm_hour, cur->tm_min, cur->tm_sec, temperature);
            next = tv.tv_sec + period;
            
            fputs(buffer, stdout);
            if (opt_log == 1)
                fputs(buffer, file);
            ///printf("***");
        }
        
        
        ///printf("###");
        if (poll(&fds, 1, 0) < 0)
        {
            fprintf(stderr, "Error in poll() system call.\n");
            exit(1);
        }
        
        if(fds.revents & POLLIN)
        {
            char input[256];
            //fgets(input, 256, stdin);
            int size = read(0, input, 256);
            
            int i = 0;
            int j = 0;
            char cmd[256];
            for(i=0; i<size; i++)
            {
                //char cmd[256];
                //int j = 0;
                if (input[i] != '\n')
                {
                    cmd[j] = input[i];
                    j++;
                }
                if (input[i] == '\n')
                {
                    cmd[j] = '\0';
                    /* Hnadle commands */
                    j = 0;
                    command(cmd, 256);
                    
                }
            }
        }
        
        if (mraa_gpio_read(button))
        {
            //printf ("HAHAHAHA\n");
            turn_off();
        }
        
        
    }
    
    /* close sensor */
    mraa_aio_close(temp);
    mraa_gpio_close(button);
    
    return (0);
    
}

/********************* END OF MAIN ********************/

//int stringcmp(char* str1, char* str2)
//{
//    if ( strlen(str1) - 1 != strlen(str2))
//        return 1;
//    unsigned int i = 0;
//    while ( i < strlen(str2) )
//    {
//        if (str1[i] == '\n')
//            return 0;
//        if (str1[i] != str2[i])
//            return 1;
//        i++;
//    }
//    return 0;
//}




void command_output(char* input)
{
    if (opt_log == 1)
    {
        fprintf(file, "%s\n", input);
        fflush(file);
    }
    else
    {
        //fflush(input);
        fprintf(stdout, "%s", input);
    }
}

void command(char* input, int size)
{
    //input[size-1] = '\0';
    int i = 0;
    while( i<size && input[i]==' ' )
        i++;
    
    if(strcmp(input, "SCALE=F") == 0)
    {
        scale = 'F';
        command_output(input);
    }
    else if(strcmp(input, "SCALE=C") == 0)
    {
        scale = 'C';
        command_output(input);
    }
    else if(strstr(input, "PERIOD=") != NULL)
    {
        command_output(input);
        char* ptr = input;
        ptr += 7;
        if (*ptr == 0)
        {
            fprintf(stderr, "Please input an valid PERIOD: >=1.\n");
            return;
        }
        period = atoi(ptr);
        while (*ptr != 0)
        {
            if (!isdigit(*ptr))
                return;
            ptr += 1;
        }
        //printf("%s", input);
        
    }
    else if (strcmp(input, "STOP") == 0)
    {
        report = 0;
        command_output(input);
    }
    else if (strcmp(input, "START") == 0)
    {
        report = 1;
        command_output(input);
    }
    else if (strstr(input, "LOG") != NULL)
    {
        
        command_output(input);
    }
    else if (strcmp(input, "OFF") == 0)
    {
        command_output(input);
        turn_off();
    }
}

void turn_off()
{
    /* log a time-stamped SHUTDOWN message */
    struct timeval tv;
    struct tm* cur;
    
    gettimeofday(&tv, 0);
    cur = localtime(&(tv.tv_sec));
    char buffer[256];
    sprintf(buffer, "%02d:%02d:%02d SHUTDOWN\n", cur->tm_hour, cur->tm_min, cur->tm_sec);
    if (opt_log == 1)
        fputs(buffer, file);
    fputs(buffer, stdout);
    
//    command_output(buffer);
    
    exit(0);
}
