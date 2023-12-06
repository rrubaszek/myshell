#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void signal_handler(int sig)
{
    write(1, "Signal intercepted!\n", 20);
}

int main()
{
    struct sigaction sa;

    sa.sa_handler = signal_handler;

    
    for(int i = 1; i <= 31; i++)
    {
        sigaction(i, &sa, NULL);
    }

    while(1)
    {
        printf("Infinite loop! %d\n", getpid());
        sleep(2);
    }
    return 0;
}