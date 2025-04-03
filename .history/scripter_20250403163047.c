#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

/* CONSTANTES */
const int max_line = 1024;
const int max_commands = 10;
#define max_redirections 3 // stdin, stdout, stderr
#define max_args 15

/* VARIABLES GLOBALES */
char *argvv[max_args];
char *filev[max_redirections];
int background = 0; 

/**
 * Tokeniza una línea en base a un delimitador.
 * @return Número de tokens.
 */
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

/**
 * Procesa las redirecciones en la línea de comandos.
 */
void procesar_redirecciones(char *args[]) {
    filev[0] = NULL;
    filev[1] = NULL;
    filev[2] = NULL;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            filev[0] = args[i + 1];
            args[i] = NULL;
        } else if (strcmp(args[i], ">") == 0) {
            filev[1] = args[i + 1];
            args[i] = NULL;
        } else if (strcmp(args[i], "!>") == 0) {
            filev[2] = args[i + 1];
            args[i] = NULL;
        }
    }
}

/**
 * Procesa la línea de entrada y ejecuta los comandos con redirecciones y procesos en segundo plano.
 */
int procesar_linea(char *linea) {
    char *comandos[max_commands];
    int num_comandos = tokenizar_linea(linea, "|", comandos, max_commands);

    if (strchr(comandos[num_comandos - 1], '&')) {
        background = 1;
        char *pos = strchr(comandos[num_comandos - 1], '&'); 
        *pos = '\0'; // Elimina el '&' de la línea
    } else {
        background = 0;
    }

    for (int i = 0; i < num_comandos; i++) {
        int args_count = tokenizar_linea(comandos[i], " \t\n", argvv, max_args);
        procesar_redirecciones(argvv);

        int pid = fork();
        if (pid == 0) {
            // Redirecciones
            if (filev[0] != NULL) {
                int fd = open(filev[0], O_RDONLY);
                if (fd < 0) {
                    perror("Error abriendo archivo de entrada");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            if (filev[1] != NULL) {
                int fd = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("Error abriendo archivo de salida");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            if (filev[2] != NULL) {
                int fd = open(filev[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("Error abriendo archivo de error");
                    exit(1);
                }
                dup2(fd, STDERR_FILENO);
                close(fd);
            }

            execvp(argvv[0], argvv);
            perror("Error ejecutando comando");
            exit(1);
        } else if (pid > 0) {
            if (!background) {
                waitpid(pid, NULL, 0);
            } else {
                printf("[Proceso en segundo plano: %d]\n", pid);
            }
        } else {
            perror("Error en fork");
        }
    }

    return num_comandos;
}

int main() {
    char linea[max_line];

    while (1) {
        printf("myshell> ");
        fflush(stdout);

        if (fgets(linea, max_line, stdin) == NULL) {
            printf("\nSaliendo...\n");
            break;
        }

        if (strcmp(linea, "exit\n") == 0) {
            printf("Saliendo...\n");
            break;
        }

        procesar_linea(linea);
    }

    return 0;
}


