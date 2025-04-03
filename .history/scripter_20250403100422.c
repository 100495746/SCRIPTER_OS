#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

/* CONST VARS */
const int max_line = 1024;
const int max_commands = 10;
#define max_redirections 3 //stdin, stdout, stderr
#define max_args 15

/* VARS TO BE USED FOR THE STUDENTS */
char * argvv[max_args];
char * filev[max_redirections];
int background = 0; 

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
    if (strchr(comandos[num_comandos - 1], '&')) {
        background = 1;
        char *pos = strchr(comandos[num_comandos - 1], '&'); 
        //remove character 
        *pos = '\0';
    }

    //Finish processing
    for (int i = 0; i < num_comandos; i++) {
        int args_count = tokenizar_linea(comandos[i], " \t\n", argvv, max_args);
        procesar_redirecciones(argvv);

    if (num_comandos == 1){
        execute_simple_comand(argvv,filev, background);
    }
    else{
        
    }









        /********* This piece of code prints the command, args, redirections and background. **********/
        /*********************** REMOVE BEFORE THE SUBMISSION *****************************************/
        /*********************** IMPLEMENT YOUR CODE FOR PROCESSES MANAGEMENT HERE ********************/
        printf("Comando = %s\n", argvv[0]);
        for(int arg = 1; arg < max_args; arg++)
            if(argvv[arg] != NULL)
                printf("Args = %s\n", argvv[arg]); 
                
        printf("Background = %d\n", background);
        if(filev[0] != NULL)
            printf("Redir [IN] = %s\n", filev[0]);
        if(filev[1] != NULL)
            printf("Redir [OUT] = %s\n", filev[1]);
        if(filev[2] != NULL)
            printf("Redir [ERR] = %s\n", filev[2]);
        /**********************************************************************************************/
        //filev --> number of redirections


    }
    if(num_comandos==1){
        execute_simple_comand(argvv, filev, background);
    }

    return num_comandos;
}



void execute_pipes(char *cmds[])


void execute_simple_comand(char *argvv[], char *filev[], int background){
    int pid;
    if ((pid = fork()) < 0) {
        perror("Fork error");
        return;
    }
    if (pid!=0){
        if(background==0){
            wait(NULL);
        }
        return;
     
    }
    
    if (filev[0]!=NULL){
        int fd;
        if((fd=open(filev[0], O_RDONLY))<0){
            perror("open error");
        };
        dup2(fd, 0);
        close(fd);
    }
    if (filev[1]!= NULL){
        int fd;
        if((fd=open(filev[1], O_WRONLY|O_CREAT|O_TRUNC))<0){
            perror("open error");
        };
        dup2(fd, 1);
        close(fd);
    }
    if (filev[2]!=NULL){
        int fd; 
        if((fd=open(filev[2], O_RDONLY))<0){
            perror("open error");
        };
        dup2(fd, 2);
        close(fd);
    }

    if((execvp(argvv[0], argvv))<0){
        perror("Error executing");
    };

}

int main(int argc, char *argv[1]) {

    /* STUDENTS CODE MUST BE HERE */
    if (argc!=2){
        perror("Incorrect number of arguments. Must be: ./scripter <file to read>");
        return -1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f){
        perror("File does not exist");
        return -1;
    }

    char line[max_line];
    int i=0; // contador de linea
    int c=0;
    int first=1;
    while(c != EOF){
        c = fgetc(f);
        if (c== '\n'|| i==max_line-1){
            line[i]='\0';

            if (strlen(line)==0){
                fprintf(stderr, "Error: empty line");
                break;
            }

            if (first ==1){
                if (strcmp(line, "## Script de SSOO")!=0){
                    fprintf(stderr, "Error: first line is invalid");
                    break;
                }
                first = 0;
            }
            
            else{
                procesar_linea(line);
            }
            i=0;
        }
        else{
            line[i++]=c;
        }
    }
    fclose(f);





    //char example_line[] = "ls -l | grep scripter | wc -l > redir_out.txt &";
    //int n_commands = procesar_linea(example_line);
    
    return 0;
}
