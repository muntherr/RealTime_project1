

/*Yasmin Abusharbak - 1182523
Munther Anati - 1182028
Yazan Abunasser - 118
*/

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
#define PUBLIC "/tmp/PUBLIC"
#define DEFAULT_NUMBER_OF_ROUNDS 5
#define NUMBER_OF_CHILDREN 10

// functions prototypes
void forking_children();        // fork 10 child processes and exec child.c
void know_next();               // write next player's PID on the private fifo
void start_round();             // order to start race- sending signals from parent to the first player of each team
void set_score();               // checking the number of rounds compared to user's input/ default value
void catch_SIGUSR1();           // callback function at SIGUSR1 to count the number of received signals by parent
void increment_winner_values(); // incrementing score per round according to the first catched signal by the parent
void terminate();               // deallocating, killing forked processes, and exit

// defining global variables
int counter;
int rounds_counter;
int *pid_numbers;
int i = 0;
int j = 0;
int red_team;
int green_team;
char buffer[4096];

int number_of_rounds = 0;

int main(int argc, char *argv[])
{
    // checking if the user passes a desired number of rounds
    if (argc != 2)
    { // no value was passed --> set the number of rounds to the default value
        number_of_rounds = DEFAULT_NUMBER_OF_ROUNDS;
        printf("\033[1;36m"); // set the color to cyan
        printf("-------------------------------THE RACE IS STARTING FOR 5 ROUNDS-------------------------------\n");
        printf("\033[0m"); // reset the color to the default
    }
    else
    { // user passed a number of rounds to be converted to an integer and considered
        number_of_rounds = atoi(argv[1]);
        if (number_of_rounds == 0)
        {
            printf("\033[1;36m"); // set the color to cyan
            printf("-------------------------------THERE SHOULD BE AT LEAST 1 ROUND!-------------------------------\n");
            printf("\033[0m"); // reset the color to the default
            exit(-1);
        }
        if (number_of_rounds == 1)
        {
            printf("\033[1;36m"); // set the color to cyan
            printf("-------------------------------THE RACE IS STARTING FOR %d ROUND!-------------------------------\n", number_of_rounds);
            printf("\033[0m"); // reset the color to the default
        }
        else
        {
            printf("\033[1;36m"); // set the color to cyan
            printf("-------------------------------THE RACE IS STARTING FOR %d ROUNDS!-------------------------------\n", number_of_rounds);
            printf("\033[0m"); // reset the color to the default
        }
    }

    // making the parent sensitive for SIGUSR1 signal received by child processes
    // the callback function catch_SIGUSR1 is to be described later
    signal(SIGUSR1, catch_SIGUSR1);

    printf("-----------------------------------------------------------------------------------------------------------\n");
    printf("Parent id is: %d\n", getpid());
    printf("-----------------------------------------------------------------------------------------------------------\n");

    // dynamic allocation of an array to store children PIDs to be used for communication
    pid_numbers = malloc(NUMBER_OF_CHILDREN * sizeof(int));
    if (pid_numbers == NULL)
    { // error allocationg memory
        printf("ERROR: MEMORY NOT ALLOCATED!\n");
        exit(0);
    }

    unlink("result"); // delete "result" from the filesystem if exists and make the space availabe to be used again
    // generate the "result" public FIFO for parent-child communcation
    if (mkfifo("result", 0666) == -1)
    {
        // error in creating FIFO
        perror("Error");
        exit(-1);
    }

    forking_children();
    know_next();
    sleep(1); // giving the parent the chance to pair each child player with its next player
    start_round();

    // keeping the parent alive
    while (1)
        ;
}

// fork child processes and exec child.c
void forking_children()
{
    pid_t pidparent, pid;
    for (i = 0; i < 10; i++)
    {
        if ((pid = fork()) == -1)
        { // failure on forking
            printf("ERROR: FORKING FAILURE!\n");
            exit(-1);
        }
        if (pid != 0)
        {                         // parent
            pid_numbers[i] = pid; // saving pid of the child process to the array
            char fifo_name[30];   // string to be used to define each private FIFO name
            // a private fifo is created for each child to allow its communcation with the next player
            sprintf(fifo_name, "/tmp/fifo%d", pid_numbers[i]); // associating fifo_name with PID after converting it from an integer to a string
            if (mkfifo(fifo_name, 0666) == -1)
            {
                // error in creating FIFO
                perror("Error");
                exit(-1);
            }

            continue;
        }
        else
        { // child
            if (i >= 0 && i <= 9)
            {
                // strings used to set the arguments passed in the exec systemcall
                char child_number[20]; // child number related to index
                char child_color[6];   // child color (Red/ Green)
                if (i >= 0 && i < (NUMBER_OF_CHILDREN / 2))
                {
                    strcpy(child_color, "Red");
                }
                else
                {
                    strcpy(child_color, "Green");
                }
                sprintf(child_number, "%d", i);                                        // converting integer value of i to store at child_number string
                execlp("./child", "./child", child_number, child_color, (char *)NULL); // replace the process image with child.c for each child
            }

            
        }
    }
}

// for the parent to complete the environment setup, link each child proces to its next
void know_next()
{
    static char buffer[50]; // string value to be written on each private FIFO
    for (i = 0; i < NUMBER_OF_CHILDREN; i++)
    {
        char fifo_name[30];
        char buffer[4096];
        memset(buffer, 0x0, 4096);
        sprintf(fifo_name, "/tmp/fifo%d", pid_numbers[i]);
        if (i == ((NUMBER_OF_CHILDREN / 2) - 1) || i == ((NUMBER_OF_CHILDREN - 1)))
        { // make the last player of each team communicate with the parent (my_next --> parent)
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
    kill(pid_numbers[0], SIGUSR1);
    printf("\033[0;31m"); // set the color to red

    printf("The parent sent the first signal to the first red player: %d- last team player %d \n", pid_numbers[0], pid_numbers[4]);
    printf("\033[0m"); // reset the color to the default

    fflush(stdout);

    kill(pid_numbers[5], SIGUSR1);
    printf("\033[0;32m"); // set the color to green
    printf("The parent sent the first signal to the first green player: %d- last team player %d \n", pid_numbers[5], pid_numbers[9]);
    printf("\033[0m"); // reset the color to the default

    fflush(stdout);
}

void set_score()
{

    if (rounds_counter < number_of_rounds)
    {
        if (counter % 2 == 0) // To make sure that the two children finished the race
        {
            printf("\033[1;36m"); // set the color to cyan
            printf("-----------------------------------------------------------------------------------------------------------\n");
            printf("--------------------------------------------Round %d started--------------------------------------------\n", rounds_counter + 1);
            printf("-----------------------------------------------------------------------------------------------------------\n");
            printf("\033[0m"); // reset the color to the default

            fflush(stdout);
            start_round(); // start a new round
        }
    }

    else if (rounds_counter == number_of_rounds)
    {
        if (red_team > green_team)
        {
            printf("\033[0;31m"); // set the color to red

            printf("-----------------------------------------------------------------------------------------------------------\n");
            printf("--------------------------------THE WINNER IS THE RED TEAM--------------------------------\n");
            printf("-----------------------------------------------------------------------------------------------------------\n");
            printf("\033[0m"); // reset the color to the default
            terminate();
        }
        else if (red_team == green_team)
            printf("-----------------------------------------------------------------------------------------------------------\n");
        printf("-----------------------------THE TWO TEAMS SCORED EQUAL POINTS--------------------------------\n");
        printf("-----------------------------------------------------------------------------------------------------------\n");

        terminate();
    }
    else
    {
        printf("\033[0;32m"); // set the color to green

        printf("-----------------------------------------------------------------------------------------------------------\n");
        printf("---------------------------THE WINNER IS THE GREEN TEAM--------------------------------\n");
        printf("-----------------------------------------------------------------------------------------------------------\n");
        printf("\033[0m"); // reset the color to the default
        terminate();
    }
}

void catch_SIGUSR1(void)
{
    counter++;
    if ((counter) % 2 == 0)
    { // Both teams finished round
        rounds_counter++;
        printf("number of round -->%d\n", rounds_counter + 1);
        fflush(stdout);
        memset(buffer, 0x0, 4096);

        set_score();
    }
    else
    {
        increment_winner_values();
    }
}

void increment_winner_values(void)
{
    int fd1 = open("result", O_NONBLOCK | O_RDONLY);

    read(fd1, &buffer, sizeof(buffer));

    close(fd1);
    if (atoi(buffer) < 5)
    {
        red_team++;
    }
    else
    {
        green_team++;
    }
    printf("-----------------------------------------------------------------------------------------------------------\n");
    printf("Red team Score = %d\n", red_team);
    printf("green team Score = %d\n", green_team);
    printf("-----------------------------------------------------------------------------------------------------------\n");

    fflush(stdout);
}
void terminate()
{
    int j;

    // killing the forked processes
    for (j = 0; j < NUMBER_OF_CHILDREN; j++)
    {
        kill(*(pid_numbers + i), SIGKILL);
    }
    // free the children_pids
    free(pid_numbers);
    exit(0);
}
