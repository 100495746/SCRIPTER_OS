#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINENUMB 30

int main(int argc, char ** argv) {
    // search for text string in a file and displays the lines where that str appears
    //On error, it gives: perror ("text describing error) and returns -1") Also,
    //If the str is not found, it will display the message "%s not found \n"


    // 3 args are expected no more, no less
    	if (argc != 3) {
        	printf("Usage: %s <ruta_fichero> <cadena_busqueda>\n", argv[0]);
        	return -1;
    	}

	//Set up
	char buffer[strlen(argv[2]) + 1];
	char string[strlen(argv[2]) + 1];
	strcpy(string, argv[2]);
	int occurrence = 0;//Variable stores number of times string is found
	FILE *file = fopen(argv[1], "r");
	//Error handling
	if (!file) {
        perror("Error opening file");
        return -1;
	}
	//Main function
	while (fgets(buffer,strlen(argv[2]) + 1,file)){
		if (strstr(buffer, string)) {//Current line = given line
			printf("%s",buffer);
			occurrence = occurrence++;
		}
	}
	if (ocurrence == 0) {//No line was concatenated after reading the file
		printf("%s not found.\n",string);
	}
	fclose(file);
	return 0;
}
