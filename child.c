#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>

// functions declarations
void start_running(int);

int child_number;
char child_color[6];
char fifo_name[20];
int flag;
int my_next = 0;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("WRONG NUMBER OF ARGUMENTS!\n");
        exit(-1);
    }

    signal(SIGUSR1, start_running);

    static char buffer[4096];

    child_number = atoi(argv[1]);
    strcpy(child_color, argv[2]);

    char fifo_name[20];

    sprintf(fifo_name, "/tmp/fifo%d", getpid());

    mkfifo(fifo_name, 0666);
    // First open in read only and read
    int fd1 = open(fifo_name, O_RDONLY);
    read(fd1, buffer, 80);

    // Print the read string and close
    printf("I AM PLAYER %d - MY NEXT PLAYER IS %s\n", getpid(), buffer);
    close(fd1);
    my_next = atoi(buffer);

    close(fd1);
    while (1)
        ;
    return 0;
}
void start_running(int sig_num)
{
    char fifo_name[30];
    sprintf(fifo_name, "result");
    mkfifo(fifo_name, 0666);

    char buffer[4096];
    int fd = open(fifo_name, O_RDWR);

    memset(buffer, 0x0, 4096);
    if (my_next == getppid() && flag == 0)
    {
        sprintf(buffer, "%d", child_number);
        write(fd, buffer, strlen(buffer) + 1);
        close(fd);
    }
    srand(getpid());
    int time_running = 0;
    time_running = rand() % 9 + 1;
    sleep(time_running);

    kill(my_next, SIGUSR1);

    fflush(stdout);

   
    if (child_number < 5)
    {
        printf("\033[0;31m"); // set the color to red
        printf("I am player %d - %d I will start running for %d to reach my next %d\n", child_number, getpid(), time_running, my_next);
        printf("\033[0m"); // reset the color to the default
    }
    if (child_number > 5)
    {
        printf(" \033[0;32m"); // set the color to green
        printf("I am player %d - %d I will start running for %d to reach my next %d\n", child_number, getpid(), time_running, my_next);
        printf("\033[0m"); // reset the color to the default
    }
    fflush(stdout);
}