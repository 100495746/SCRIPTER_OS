#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_COMMANDS 10
#define MAX_ARGS 15
#define MAX_REDIRECTIONS 3

char *filev[MAX_REDIRECTIONS];
int background = 0;

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
//compact

int tokenizar_linea(char *linea, char *delim, char *tokens[], int max_tokens) {
    int i = 0;
    char *token = strtok(linea, delim);
    while (token != NULL && i < max_tokens - 1) {
        tokens[i++] = token;
        token = strtok(NULL, delim);
    }
    tokens[i] = NULL;
    return i;
}

void procesar_redirecciones(char *args[]) {
    filev[0] = NULL;
    filev[1] = NULL;
    filev[2] = NULL;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            filev[0] = args[i+1];
            args[i] = NULL; args[i+1] = NULL;
        } else if (strcmp(args[i], ">") == 0) {
            filev[1] = args[i+1];
            args[i] = NULL; args[i+1] = NULL;
        } else if (strcmp(args[i], "!>") == 0) {
            filev[2] = args[i+1];
            args[i] = NULL; args[i+1] = NULL;
        }
    }
}

void procesar_linea(char *linea) {
    char *comandos[MAX_COMMANDS];
    int num_comandos = tokenizar_linea(linea, "|", comandos, MAX_COMMANDS);

    background = 0;
    if (strchr(comandos[num_comandos - 1], '&')) {
        background = 1;
        char *pos = strchr(comandos[num_comandos - 1], '&');
        *pos = '\0';
    }

    int pipes[MAX_COMMANDS - 1][2];
    for (int i = 0; i < num_comandos - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < num_comandos; i++) {
        char *args[MAX_ARGS];
        tokenizar_linea(comandos[i], " \t\n", args, MAX_ARGS);
        procesar_redirecciones(args);

        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) {
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            } else if (filev[0]) {
                int fd = open(filev[0], O_RDONLY);
                if (fd < 0) { perror("open input"); exit(1); }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (i < num_comandos - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            } else if (filev[1]) {
                int fd = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror("open output"); exit(1); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (filev[2]) {
                int fd = open(filev[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror("open error"); exit(1); }
                dup2(fd, STDERR_FILENO);
                close(fd);
            }

            for (int j = 0; j < num_comandos - 1; j++) {
                close(pipes[j][0]); close(pipes[j][1]);
            }

            if (args[0] && strcmp(args[0], "sort") == 0) {
                setbuf(stdout, NULL);
            }

            execvp(args[0], args);
            perror("execvp");
            exit(1);
        }
    }

    for (int i = 0; i < num_comandos - 1; i++) {
        close(pipes[i][0]); close(pipes[i][1]);
    }

    if (!background) {
        for (int i = 0; i < num_comandos; i++) {
            wait(NULL);
        }
    } else {
        printf("[background execution]\n");
    }
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <script_file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open script");
        return 1;
    }

    char line[MAX_LINE];
    char c;
    int i = 0;
    int first = 1;

    while (read(fd, &c, 1) > 0) {
        if (c == '\n') {
            line[i] = '\0';
            if (first) {
                if (strcmp(line, "## Script de SSOO") != 0) {
                    fprintf(stderr, "Invalid header line\n");
                    break;
                }
                first = 0;
            } else if (strlen(line) > 0) {
                procesar_linea(line);
            }
            i = 0;
        } else {
            if (i < MAX_LINE - 1)
                line[i++] = c;
        }
    }

    close(fd);
    return 0;
}
