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

int my_next = 0;

int main(int argc, char *argv[])
{
    // if (argc != 2) {
    //     printf("There should be 2 argc!\n");
    //     exit(-1);
    // }
    // making the parent process sensitive to both SIGINT and SIGQUIT//
    //   if ( sigset(SIGUSR1, start_running) == SIG_ERR ) {
    //       perror("Sigset can not set SIGQUIT");
    //       exit(-1);
    //   }
    signal(SIGUSR1, start_running);

    static char buffer[4096];

    child_number = atoi(argv[1]);
    strcpy(child_color, argv[2]);
    // printf("Hi I am child %d, number %d color %s for parent %d \n", getpid(), child_number, child_color, getppid());
    fflush(stdout);

    char fifo_name[20];

    // sleep(2);    // printf("%d\n", j);

    sprintf(fifo_name, "/tmp/fifo%d", getpid());
    // printf("child %s\n", fifo_name);

    mkfifo(fifo_name, 0666);
    // First open in read only and read
    int fd1 = open(fifo_name, O_RDONLY);
    read(fd1, buffer, 80);

    // Print the read string and close
    printf("%d Received: %s\n", getpid(), buffer);
    close(fd1);
    my_next = atoi(buffer);
    // printf("%d\n", my_next);

    close(fd1);
    while(1);
    return 0;
}
void start_running(int sig_num)
{
        printf("--%d--\n", child_number);

    srand(time(0) * getpid());
    int time_running = 0;
    time_running = rand() % 10;
    sleep(time_running);
    kill(my_next, SIGUSR1);
    printf(" I will sleep for : %d -- \n", time_running); 
    printf("I am %d - I have received SigUSR1, I will start running\n", getpid());
    fflush(stdout);
}