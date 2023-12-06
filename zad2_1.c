#include <stdio.h>
#include <signal.h>

int main()
{
    int res = kill(1, SIGKILL);
    if(res == 0)
    {
        printf("Signal sent succesfully!\n");
    }
    else
    {
        printf("Unable to send signal\n");
    }

    int id = fork();

    if(id == 0)
    {
        kill(getppid(), SIGKILL);
    }
    else
    {
        while(1)
        {
            printf("I'm parent, I will die soon...\n");
            //sleep(2);
        }    

    }

    return 0;
}