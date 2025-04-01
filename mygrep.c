#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINESIZE 500
#define MAX_LINENUMB 30

int main(int argc, char ** argv) {
    // search for text string in a file and displays the lines where that str appears
    //On error, it gives: perror ("text describing error) and returns -1") Also,
    //If the str is not found, it will display the message "%s not found \n"


    // 3 args are expected no more, no less
    //explain this exactly with gpt...
    if (argc != 3) {
        printf("Usage: %s <ruta_fichero> <cadena_busqueda>\n", argv[0]);
        return -1;
    }

	//Set up
	char buffer[MAX_LINESIZE];
	char in_lines[MAX_LINESIZE*MAX_LINENUMB] = "";
	char string[strlen[argv[2]]] = argv[2];
	int file = open(argv[1],O_RDONLY);
	
	if (file < 0) {
        perror("Error opening file");
        return -1;
	}
	
	//Main function
	while not feof(file){ //fgets could be used directly but feof is used for clarity 
		fgets(buffer,MAX_LINESIZE,file);
		if (buffer == string) {//current line = given line
			printf("%s\n",buffer)
			in_lines = strcat(in_lines, buffer) //store ocurrence in in_lines
		}
		//continue
	}
	
	if in_lines == "" {//no line was concatenated after reading the file
		printf("%s not found.\n",string);
		close(file);
		return 0;
	}
	
	close(file);
    return 0;
}
