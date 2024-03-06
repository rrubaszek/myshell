#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv, char **envp)
{
    int i = seteuid(0);

    if(i != 0)
    {
        printf("You don't have priviliges to open shell as root!\n");
    }
    else
    {
        printf("Starting new terminal...\n");
        fflush(stdout);
        setreuid(0, -1);
        system("/bin/bash");
    }
    return 0;
}
