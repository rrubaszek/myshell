#include <fcntl.h>
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <signal.h>
#include <sys/types.h> 
#include <sys/wait.h> 

#define clear() printf("\033[H\033[J") 

#define MAX 100

void initialize_shell_window()
{
    clear(); 
    printf("LSH SHELL\n");
    printf("CURRENT USER: @%s", getenv("USER")); 
    printf("\n"); 
}

void sig_handler()
{
    printf("\n");
}

void background_handler()
{
    wait(NULL);
}

int command_lenght(char** command)
{
    int i = 0;

	while(command[i] != NULL)
	{
		i++;
	}
	return i;
}

char** split_command(char* input)
{
    char* p = strtok(input, " ");
	char** s = malloc(1024 * sizeof(char*));
	int i = 0;
	while(p != NULL)
	{
		s[i++] = strdup(p);
		p = strtok(NULL, " ");
	}
    s[i] = NULL;
    return s;
}

char** split_pipes(char* input)
{
	char* p = strtok(input, "|");
	char** s = malloc(1024 * sizeof(char*));
	int i = 0;
	while(p != NULL)
	{
		s[i++] = strdup(p);
		p = strtok(NULL, "|");
	}
    s[i] = NULL;
	return s;
}

char** split_out(char* input)
{
    char* p = strtok(input, " > ");
	char** s = malloc(1024 * sizeof(char*));
	int i = 0;
	while(p != NULL)
	{
		s[i++] = strdup(p);
		p = strtok(NULL, " > ");
	}
    s[i] = NULL;
	return s;   
}

char** split_in(char* input)
{
    char* p = strtok(input, " < ");
	char** s = malloc(1024 * sizeof(char*));
	int i = 0;
	while(p != NULL)
	{
		s[i++] = strdup(p);
		p = strtok(NULL, " < ");
	}
    s[i] = NULL;
	return s;
}

char** split_err(char* input)
{
    char* p = strtok(input, " 2> ");
	char** s = malloc(1024 * sizeof(char*));
	int i = 0;
	while(p != NULL)
	{
		s[i++] = strdup(p);
		p = strtok(NULL, " 2> ");
	}
    s[i] = NULL;
	return s;
}
void execute(char** parsed, int bg_flag)
{
    pid_t id;
   
    id = fork();

    if(id == 0)
    {
        signal(SIGINT, SIG_DFL);
        
        signal(SIGTSTP, SIG_DFL);

        int k = execvp(parsed[0], parsed);

        exit(EXIT_FAILURE);
    }
    else if(id < 0)
    {
        perror("LSH");
    }
    else
    {
        if(bg_flag == 0)
        {
            wait(NULL);
        }
        else
        {
            signal(SIGCHLD, background_handler);
        }
    }

}

void execute_pipe(char** command)
{
    int fd[2];
    int tempin = dup(STDIN_FILENO);			
	int tempout = dup(STDOUT_FILENO);

    fd[0] = dup(tempin);

    for(int i = 0; i < command_lenght(command); i++)
    {
        char** s_command = split_command(command[i]);

        dup2(fd[0], STDIN_FILENO);
		close(fd[0]);

        if(i == command_lenght(command) - 1)
        {
            fd[1] = dup(tempout);
        }
        else
        {
            int fd_copy[2];
		    pipe(fd_copy);
		    fd[1] = fd_copy[1];
		    fd[0] = fd_copy[0];
        }
        

        dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);

        pid_t id = fork();
        if(id == 0)
        {
            signal(SIGINT, SIG_DFL);
            
            signal(SIGTSTP, SIG_DFL);

            execvp(s_command[0], s_command);

            exit(EXIT_FAILURE);
        }
        wait(NULL);
    }

    dup2(tempin, STDIN_FILENO);
	dup2(tempout, STDOUT_FILENO);
	close(tempin);
	close(tempout);
}

/*
void execute_pipe_TODO(char** command)
{
    int tempin = dup(0);			
	int tempout = dup(1);			
	int flag = 0;
	int fdin = 0, fdout;

	for(int j = 0; j < command_lenght(command); j++)
	{
		
		if(strcmp(command[j], "<") == 0)
		{
			fdin = open(command[j + 1], O_RDONLY);
			flag += 2;
		}
	}

	if(!fdin)
    {
        fdin = dup(tempin);
    }
	
    int id;
    for(int i = 0; i < command_lenght(command); i++)
    {
        char** s_command = split_command(command[i]);

        dup2(fdin, 0);
		close(fdin);
		if(i == command_lenght(command) - 3 && strcmp(command[i + 1], ">") == 0)
		{	
			if((fdout = open(command[i + 2], O_WRONLY)))
            {
                i++;
            }
		}
		else if(i == command_lenght(command) - flag - 1)
        {
            fdout = dup(tempout);
        }
		else
        {
            printf("standard case\n");
            int fd[2];
		    pipe(fd);
		    fdout = fd[1];
		    fdin = fd[0];
        }

        dup2(fdout, 1);
		close(fdout);

        id = fork();
        if(id == 0)
        {
            execvp(s_command[0], s_command);

            exit(EXIT_FAILURE);
        }
        wait(NULL);
    }

    dup2(tempin, 0);
	dup2(tempout, 1);
	close(tempin);
	close(tempout);

}
*/

int main()
{
    char* buffer;
    char** parsed;
    size_t buffsize = 1024;
    char* commands[2];

    commands[0] = "exit";
    commands[1] = "cd";

    initialize_shell_window();

    signal(SIGINT, sig_handler);
    signal(SIGTSTP, sig_handler);

    while(1)
    {
        int i = 0;
        int bg_flag = 0;
        int pipe_flag = 0;
        int flag_itr = 0;
        int in_flag = 0;
        int out_flag = 0;
        int err_flag = 0;

        
    
        printf(">> ");

        buffer = (char*)malloc(buffsize * sizeof(char));

        if(getline(&buffer, &buffsize, stdin) == -1)
        {
            if(feof(stdin))
            {
                free(buffer);
                exit(EXIT_SUCCESS);
            }
            else
            {
                free(buffer);
                perror("error while reading the line from stdin\n");
                exit(EXIT_FAILURE);
            }
        }

        int size = strlen(buffer);

        if(buffer[size-1] == '\n')
        {
            buffer[size-1] = '\0';
        }

        if(buffer[size-2] == '&')
        {
            bg_flag = 1;
            buffer[size-2] = '\0';
        }

        if(strcmp(buffer, commands[0]) == 0)
        {
            printf("Shutting down...\n");
            break;
        }

        while(buffer[flag_itr] != '\0')
        {
            if(buffer[flag_itr] == '|')
			{	
				pipe_flag = 1;
				break;
			}
			flag_itr++;
        }

        flag_itr = 0;
        while(buffer[flag_itr] != '\0')
        {
            if(buffer[flag_itr] == '>')
			{	
				out_flag = 1;
				break;
			}
			flag_itr++;
        }

        flag_itr = 0;
        while(buffer[flag_itr] != '\0')
        {
            if(buffer[flag_itr] == '<')
			{	
				in_flag = 1;
				break;
			}
			flag_itr++;
        }

        flag_itr = 0;
        while(buffer[flag_itr] != '\0')
        {
            if(buffer[flag_itr] == '2' && buffer[flag_itr + 1] == '>')
			{	
				err_flag = 1;
				break;
			}
			flag_itr++;
        }

        if(pipe_flag)
        {
            parsed = split_pipes(buffer);
            execute_pipe(parsed);
            free(parsed);
        }
        else if(out_flag)
        {
            parsed = split_out(buffer);

            char** temp = split_command(parsed[0]);
            
            pid_t id = fork();

            if(id == 0)
            {
                signal(SIGINT, SIG_DFL);

                int file = open(parsed[1], O_WRONLY | O_CREAT, 0777);
                dup2(file, STDOUT_FILENO);
                close(file);    
                execvp(temp[0], temp);
            }
            else
            {
                wait(NULL);
            }
            
            free(temp);
            free(parsed);

        }
        else if(in_flag)
        {
            parsed = split_in(buffer);

            char** temp = split_command(parsed[0]);

            pid_t id = fork();

            if(id == 0)
            {
                signal(SIGINT, SIG_DFL);

                int file = open(parsed[1], O_RDONLY);
                dup2(file, STDIN_FILENO);
                close(file);
                execvp(temp[0], temp);
            }
            else
            {
                wait(NULL);
            }

            free(temp);
            free(parsed);

        }
        else if (err_flag)
        {
            parsed = split_err(buffer);

            char** temp = split_command(parsed[0]);
        
            pid_t id = fork();

            if(id == 0)
            {
                signal(SIGINT, SIG_DFL);

                int file = open(parsed[1], O_WRONLY | O_CREAT, 0777);
                dup2(file, STDERR_FILENO);
                close(file);
                execvp(temp[0], temp);
            }
            else
            {
                wait(NULL);
            }

            free(temp);
            free(parsed);

        }
        else
        {
            parsed = split_command(buffer);

            if(strcmp(parsed[0], commands[1]) == 0)
            {
                chdir(parsed[1]);
            }
            else
            {
                execute(parsed, bg_flag);
            }

            free(parsed);

        }
        free(buffer);
    }

    return 0;
}
