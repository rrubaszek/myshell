// Extended shell with input/output/error redirection, pipe chaining, and job control

#include <fcntl.h>
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <signal.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <errno.h>

#define clear() printf("\033[H\033[J") 
#define MAX 100

void initialize_shell_window() {
    clear(); 
    printf("LSH SHELL\n");
    printf("CURRENT USER: @%s\n", getenv("USER")); 
}

void sig_handler(int sig) {
    printf("\n");
    fflush(stdout);
}

void background_handler(int sig) {
    waitpid(-1, NULL, WNOHANG);
}

int command_length(char** command) {
    int i = 0;
    while (command[i] != NULL) i++;
    return i;
}

void free_command(char** command) {
    for (int i = 0; command[i]; i++) free(command[i]);
    free(command);
}

char** split_command(const char* input) {
    char* dup = strdup(input);
    char* p = strtok(dup, " \t\n");
    char** s = malloc(1024 * sizeof(char*));
    int i = 0;
    while (p != NULL) {
        s[i++] = strdup(p);
        p = strtok(NULL, " \t\n");
    }
    s[i] = NULL;
    free(dup);
    return s;
}

char** split_delim(char* input, const char* delim) {
    char* p = strtok(input, delim);
    char** s = malloc(1024 * sizeof(char*));
    int i = 0;
    while (p != NULL) {
        s[i++] = strdup(p);
        p = strtok(NULL, delim);
    }
    s[i] = NULL;
    return s;
}

void execute(char** args, int bg_flag) {
    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        execvp(args[0], args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        if (!bg_flag) {
            waitpid(pid, NULL, 0);
        } else {
            signal(SIGCHLD, background_handler);
        }
    } else {
        perror("fork failed");
    }
}

void execute_redirection(char* buffer, int bg_flag) {
    char* cmd = strdup(buffer);
    int in_redirect = 0, out_redirect = 0, err_redirect = 0;
    char *in_file = NULL, *out_file = NULL, *err_file = NULL;

    char* input_part = strtok(cmd, ">\n");
    char* output_part = strstr(buffer, ">");
    char* error_part = strstr(buffer, "2>");
    char* input_redir = strstr(buffer, "<");

    if (input_redir) {
        in_redirect = 1;
        *input_redir = 0;
        in_file = strtok(input_redir + 1, " \t");
    }
    if (error_part) {
        err_redirect = 1;
        *error_part = 0;
        err_file = strtok(error_part + 2, " \t");
    }
    if (output_part && (!err_redirect || output_part < error_part)) {
        out_redirect = 1;
        *output_part = 0;
        out_file = strtok(output_part + 1, " \t");
    }

    char** args = split_command(input_part);
    pid_t pid = fork();
    if (pid == 0) {
        if (in_redirect) {
            int fd = open(in_file, O_RDONLY);
            if (fd == -1) { perror("open input"); exit(1); }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (out_redirect) {
            int fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) { perror("open output"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (err_redirect) {
            int fd = open(err_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) { perror("open error"); exit(1); }
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        if (!bg_flag) waitpid(pid, NULL, 0);
        else signal(SIGCHLD, background_handler);
    } else {
        perror("fork");
    }

    free(args);
    free(cmd);
}

void execute_pipe(char** commands) {
    int n = command_length(commands);
    int tempin = dup(STDIN_FILENO);
    int tempout = dup(STDOUT_FILENO);
    int fdin = dup(STDIN_FILENO);

    for (int i = 0; i < n; i++) {
        int fd[2];
        pipe(fd);

        dup2(fdin, STDIN_FILENO);
        close(fdin);

        if (i == n - 1) {
            dup2(tempout, STDOUT_FILENO);
        } else {
            dup2(fd[1], STDOUT_FILENO);
        }

        close(fd[1]);
        fdin = fd[0];

        pid_t pid = fork();
        if (pid == 0) {
            char** args = split_command(commands[i]);
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            execvp(args[0], args);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
        } else {
            waitpid(pid, NULL, 0);
        }
    }

    dup2(tempin, STDIN_FILENO);
    dup2(tempout, STDOUT_FILENO);
    close(tempin);
    close(tempout);
}

int main() {
    char* buffer = NULL;
    size_t buffsize = 0;
    initialize_shell_window();
    signal(SIGINT, sig_handler);
    signal(SIGTSTP, sig_handler);

    while (1) {
        printf(">> ");
        if (getline(&buffer, &buffsize, stdin) == -1) {
            if (feof(stdin)) break;
            perror("getline failed");
            continue;
        }

        size_t size = strlen(buffer);
        if (size > 0 && buffer[size - 1] == '\n') buffer[size - 1] = '\0';

        int bg_flag = 0;
        if (size >= 2 && buffer[size - 2] == '&') {
            bg_flag = 1;
            buffer[size - 2] = '\0';
        }

        if (strcmp(buffer, "exit") == 0) {
            printf("Shutting down...\n");
            break;
        }

        if (strncmp(buffer, "cd", 2) == 0) {
            char** args = split_command(buffer);
            if (args[1]) chdir(args[1]);
            else chdir(getenv("HOME"));
            free_command(args);
            continue;
        }

        if (strchr(buffer, '|')) {
            char** piped = split_delim(buffer, "|");
            execute_pipe(piped);
            free_command(piped);
        } else if (strchr(buffer, '<') || strstr(buffer, ">") || strstr(buffer, "2>")) {
            execute_redirection(buffer, bg_flag);
        } else {
            char** args = split_command(buffer);
            execute(args, bg_flag);
            free_command(args);
        }
    }

    free(buffer);
    return 0;
}