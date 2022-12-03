#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

// functions declarations
void forking_children();
void know_next();
void start_round();
void set_score();
void catch_usr2();
int counter;
int number_of_rounds;
int pid_numbers[10];
int i = 0;
int j = 0;

int number_of_children = 10;

// int fd[2];
int main()
{
    signal(SIGUSR1, set_score);
        signal(SIGUSR1, catch_usr2);

    printf("Parent id is: %d\n", getpid());
    FILE *fin;

    // pid_numbers = malloc(10 * sizeof(int));
    forking_children();
    know_next();
    sleep(1);
    start_round();

    while (1)
        ;
}
void forking_children()
{
    pid_t pidparent, pid;
    for (i = 0; i < number_of_children; i++)
    {
        if ((pid = fork()) == -1)
        {
            printf("FAILURE\n");
            exit(-1);
        }
        if (pid != 0)
        {                         // parent
            pid_numbers[i] = pid; // saving pid of child processes in an array
            char fifo_name[30];
            sprintf(fifo_name, "/tmp/fifo%d", pid_numbers[i]);
            /* Generate the private FIFO */
            // printf("Parent %s\n", fifo_name);
            mkfifo(fifo_name, 0666);
            continue;
        }
        else
        { // child
            char child_number[20];
            char child_color[6];
            if (i >= 0 && i < 5)
            {
                strcpy(child_color, "Red");
            }
            else
            {
                strcpy(child_color, "Green");
            }
            sprintf(child_number, "%d", i);
            execlp("./child", "./child", child_number, child_color, (char *)NULL);
        }
    }
}
void know_next()
{
    static char buffer[50];
    for (i = 0; i < number_of_children; i++)
    {
        int private_fifo = 0;
        char fifo_name[30];
        char buffer[4096];
        memset(buffer, 0x0, 4096);
        sprintf(fifo_name, "/tmp/fifo%d", pid_numbers[i]);
        if (i == 4 || i == 9)
        {
            sprintf(buffer, "%d", getpid());
        }
        else
        {
            sprintf(buffer, "%d", pid_numbers[i + 1]);
        }
        int fd = open(fifo_name, O_WRONLY);
        write(fd, buffer, strlen(buffer) + 1);
        close(fd);
    }
}

void start_round()
{
    // printf("I am sending two signals\n\n");
    // fflush(stdout);
    kill(pid_numbers[0], SIGUSR1);
    printf("Sent the first one - %d\n", pid_numbers[0]);
    fflush(stdout);
    kill(pid_numbers[5], SIGUSR1);
    printf("Sent the second one - %d\n", pid_numbers[5]);
    fflush(stdout);
}

void set_score(){
    if (counter%2 == 0)
    {
        
    }
    
    printf("I won!\n");
}



void catch_usr2(void) {
    counter++;
    if ((counter)%2 == 0)
    {
        number_of_rounds++;
        printf("number of round -->%d\n", number_of_rounds);
    }
    
    printf("-->%d<--\n",counter);
}
