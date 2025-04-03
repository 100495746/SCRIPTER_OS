
	

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

/* CONST VARS */
const int max_line = 1024;
const int max_commands = 10;
#define max_redirections 3 //stdin, stdout, stderr
#define max_args 15

/* VARS TO BE USED FOR THE STUDENTS */
char * argvv[max_args];
char * filev[max_redirections];
int background = 0; 
int zombie_pids[100]; // Array para almacenar PIDs de procesos zombies
int zombie_count = 0; // Contador de procesos zombies

// Manejador de señal para SIGCHLD
void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    
    // Recoger todos los procesos hijos que han terminado sin bloquear
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // No hacemos nada especial, solo evitamos zombies
    }
}

/**
 * This function splits a char* line into different tokens based on a given character
 * @return Number of tokens 
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
 * This function processes the command line to evaluate if there are redirections. 
 * If any redirection is detected, the destination file is indicated in filev[i] array.
 * filev[0] for STDIN
 * filev[1] for STDOUT
 * filev[2] for STDERR
 */
void procesar_redirecciones(char *args[]) {
    //initialization for every command
    filev[0] = NULL;
    filev[1] = NULL;
    filev[2] = NULL;
    //Store the pointer to the filename if needed.
    //args[i] set to NULL once redirection is processed
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            filev[0] = args[i+1];
            args[i] = NULL;
            args[i + 1] = NULL;
        } else if (strcmp(args[i], ">") == 0) {
            filev[1] = args[i+1];
            args[i] = NULL;
            args[i + 1] = NULL;
        } else if (strcmp(args[i], "!>") == 0) {
            filev[2] = args[i+1];
            args[i] = NULL; 
            args[i + 1] = NULL;
        }
    }
}

// Función para ejecutar un comando simple
void execute_simple_comand(char *argvv[], char *filev[], int background);

// Función para ejecutar comandos con pipes
void execute_piped_commands(char *commands[], int num_commands);

// Función para detectar procesos zombies
void detect_zombies(pid_t parent_pid);

/**
 * This function processes the input command line and returns in global variables: 
 * argvv -- command an args as argv 
 * filev -- files for redirections. NULL value means no redirection. 
 * background -- 0 means foreground; 1 background.
 */
int procesar_linea(char *linea) {
    char *comandos[max_commands];
    int num_comandos = tokenizar_linea(linea, "|", comandos, max_commands);

    //Check if background is indicated
    background = 0; // Reiniciar el valor para cada línea
    if (strchr(comandos[num_comandos - 1], '&')) {
        background = 1;
        char *pos = strchr(comandos[num_comandos - 1], '&'); 
        //remove character 
        *pos = '\0';
        // Eliminar espacios en blanco después de eliminar &
        int len = strlen(comandos[num_comandos - 1]);
        while (len > 0 && (comandos[num_comandos - 1][len-1] == ' ' || 
                           comandos[num_comandos - 1][len-1] == '\t')) {
            comandos[num_comandos - 1][len-1] = '\0';
            len--;
        }
    }

    //Finish processing
    if (num_comandos == 1) {
        // Procesar un solo comando
        int args_count = tokenizar_linea(comandos[0], " \t\n", argvv, max_args);
        procesar_redirecciones(argvv);
        execute_simple_comand(argvv, filev, background);
    } else {
        // Procesar múltiples comandos con pipes
        execute_piped_commands(comandos, num_comandos);
    }

    return num_comandos;
}

// Función para ejecutar un comando simple
void execute_simple_comand(char *argvv[], char *filev[], int background) {
    int pid;
    if ((pid = fork()) < 0) {
        perror("Fork error");
        return;
    }
    
    if (pid == 0) { // Proceso hijo
        // Manejar redirecciones
        if (filev[0] != NULL) {
            int fd;
            if ((fd = open(filev[0], O_RDONLY)) < 0) {
                perror("open error");
                exit(1);
            }
            if (dup2(fd, 0) < 0) {
                perror("dup2 error");
                exit(1);
            }
            close(fd);
        }
        
        if (filev[1] != NULL) {
            int fd;
            // Asegurar que los permisos son correctos (0644)
            if ((fd = open(filev[1], O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
                perror("open error");
                exit(1);
            }
            if (dup2(fd, 1) < 0) {
                perror("dup2 error");
                exit(1);
            }
            close(fd);
        }
        
        if (filev[2] != NULL) {
            int fd; 
            // Asegurar que los permisos son correctos (0644)
            if ((fd = open(filev[2], O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
                perror("open error");
                exit(1);
            }
            if (dup2(fd, 2) < 0) {
                perror("dup2 error");
                exit(1);
            }
            close(fd);
        }

        // Tratamiento especial para el comando sort
        if (argvv[0] != NULL && strcmp(argvv[0], "sort") == 0) {
            // Asegurar que el buffer de salida no esté almacenado en caché
            setbuf(stdout, NULL);
        }

        // Ejecutar el comando
        if (execvp(argvv[0], argvv) < 0) {
            perror("Error executing");
            exit(1);
        }
    } else { // Proceso padre
        if (background == 0) {
            // Esperar si es un proceso en primer plano
            int status;
            waitpid(pid, &status, 0);
        } else {
            // Si es un proceso en background, solo imprimir el PID
            printf("%d\n", pid);
        }
    }
}

// Función para ejecutar comandos con pipes
void execute_piped_commands(char *commands[], int num_commands) {
    int i;
    int pipes[max_commands-1][2]; // Arreglo de pipes
    pid_t pids[max_commands]; // Arreglo para almacenar PIDs

    // Crear los pipes necesarios
    for (i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe error");
            return;
        }
    }

    // Procesar cada comando
    for (i = 0; i < num_commands; i++) {
        // Tokenizar el comando actual
        char *args[max_args];
        tokenizar_linea(commands[i], " \t\n", args, max_args);
        
        // Procesar redirecciones para este comando
        filev[0] = NULL;
        filev[1] = NULL;
        filev[2] = NULL;
        procesar_redirecciones(args);

        // Crear un proceso hijo para ejecutar el comando
        if ((pids[i] = fork()) < 0) {
            perror("fork error");
            return;
        }

        if (pids[i] == 0) { // Proceso hijo
            // Configurar entrada desde el pipe anterior (excepto el primer comando)
            if (i < num_commands - 1) {
                if (dup2(pipes[i][1], 1) < 0) {
                    perror("dup2 error");
                    exit(1);
                }
            }
            else if (filev[1] != NULL) {
                // Redirección de entrada para el primer comando
                int fd =  open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open error");
                    exit(1);
                }
                if (dup2(fd, 0) < 0) {
                    perror("dup2 error");
                    exit(1);
                }
                close(fd);
            }

            // Configurar salida hacia el siguiente pipe (excepto el último comando)
            if (i < num_commands - 1) {
                if (dup2(pipes[i][1], 1) < 0) {
                    perror("dup2 error");
                    exit(1);
                }
           
            
            } else if (filev[1] != NULL) {
                // Redirección de salida para el último comando
                int fd = open(filev[1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open error");
                    exit(1);
                }
                if (dup2(fd, 1) < 0) {
                    perror("dup2 error");
                    exit(1);
                }
                close(fd);
            }

            // Redirección de error si está especificada
            if (filev[2] != NULL) {
                int fd = open(filev[2], O_WRONLY|O_CREAT|O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open error");
                    exit(1);
                }
                if (dup2(fd, 2) < 0) {
                    perror("dup2 error");
                    exit(1);
                }
                close(fd);
            }

            // Cerrar todos los descriptores de pipe
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Tratamiento especial para el comando sort
            if (args[0] != NULL && strcmp(args[0], "sort") == 0) {
                // Asegurar que el buffer de salida no esté almacenado en caché
                setbuf(stdout, NULL);
            }

            // Ejecutar el comando
            if (execvp(args[0], args) < 0) {
                perror("execvp error");
                exit(1);
            }
        }
    }

    // Cerrar todos los descriptores de pipe en el proceso padre
    for (i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Esperar a que todos los procesos hijos terminen si no es background
    if (background == 0) {
        for (i = 0; i < num_commands; i++) {
            waitpid(pids[i], NULL, 0);
        }
    } else {
        // Si es background, imprimir el PID del último proceso
        printf("%d\n", pids[num_commands-1]);
    }
}

// Función para detectar procesos zombies
void detect_zombies(pid_t parent_pid) {
    char command[100];
    char buffer[1024];
    int fd[2];
    
    // Crear un pipe para capturar la salida del comando
    if (pipe(fd) == -1) {
        perror("pipe error");
        return;
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork error");
        return;
    }
    
    if (pid == 0) { // Proceso hijo
        // Redirigir la salida estándar al pipe
        if (dup2(fd[1], 1) < 0) {
            perror("dup2 error");
            exit(1);
        }
        close(fd[0]);
        close(fd[1]);
        
        // Ejecutar el comando ps para detectar zombies
        sprintf(command, "ps --ppid %d -o pid,stat,cmd | grep Z", parent_pid);
        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl error");
        exit(1);
    } else { // Proceso padre
        close(fd[1]);
        
        // Leer la salida del comando
        int n = read(fd[0], buffer, sizeof(buffer) - 1);
        close(fd[0]);
        
        if (n > 0) {
            buffer[n] = '\0';
            // Procesar la salida para encontrar PIDs de procesos zombies
            char *line = strtok(buffer, "\n");
            while (line != NULL) {
                pid_t zombie_pid = 0;
                sscanf(line, "%d", &zombie_pid);
                if (zombie_pid > 0) {
                    // Almacenar el PID del proceso zombie
                    zombie_pids[zombie_count++] = zombie_pid;
                    if (zombie_count >= 100) break;
                }
                line = strtok(NULL, "\n");
            }
        }
        
        // Esperar a que el proceso hijo termine
        waitpid(pid, NULL, 0);
    }
}

int main(int argc, char *argv[]) {
    // Configurar el manejador de señal para SIGCHLD
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction error");
        return -1;
    }

    /* STUDENTS CODE MUST BE HERE */
    if (argc != 2) {
        write(2, "Incorrect number of arguments. Must be: ./scripter <file to read>\n", 66);
        return -1;
    }

    // Usar open y read en lugar de fopen y fread para cumplir con los requisitos
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        write(2, "File does not exist\n", 20);
        return -1;
    }

    char line[max_line];
    int i = 0; // contador de linea
    char buffer[1];
    int first = 1;
    
    while (read(fd, buffer, 1) > 0) {
        char c = buffer[0];
        if (c == '\n' || i == max_line - 1) {
            line[i] = '\0';

            if (strlen(line) == 0) {
                write(2, "Error: empty line\n", 18);
                break;
            }

            if (first == 1) {
                if (strcmp(line, "## Script de SSOO") != 0) {
                    write(2, "Error: first line is invalid\n", 29);
                    break;
                }
                first = 0;
            } else {
                procesar_linea(line);
                
                // Detectar procesos zombies después de cada comando
                detect_zombies(getpid());
            }
            i = 0;
        } else {
            line[i++] = c;
        }
    }
    
    close(fd);
    
    return 0;
}
