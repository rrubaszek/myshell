#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void signal_handler(int sig)
{
    printf("Signal received!\n");
}

int main()
{
    int pid = fork();

    if(pid == -1)
    {
        printf("Error\n");
    }

    if(pid == 0)
    {
        //Child process
        for(int i = 0; i < 10; i++)
        {
            kill(getppid(), SIGUSR1);  
        }

    }
    else
    {
        //Parent process

        struct sigaction sa = { 0 };

        sa.sa_handler = signal_handler;

        sigaction(SIGUSR1, &sa, NULL);

        printf("Parent process...\n");
    
    }

    return 0;

}
