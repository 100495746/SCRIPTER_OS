1 de 866
Fwd:
Externo
Recibidos
Alberto Álvarez <albertoharing@gmail.com>
	
Adjuntos10:32 (hace 1 minuto)
	
para mí


---------- Forwarded message ---------
De: Alberto Álvarez <albertoharing@gmail.com>
Date: jue, 3 abr 2025 a las 10:25
Subject: Re:
To: Alberto <aabinternet@gmail.com>




El jue, 3 abr 2025 a las 9:23, Alberto Álvarez (<albertoharing@gmail.com>) escribió:



    El mié, 2 abr 2025 a las 23:07, Alberto Álvarez (<albertoharing@gmail.com>) escribió:



        El mié, 2 abr 2025 a las 23:03, Alberto Álvarez (<albertoharing@gmail.com>) escribió:



            El mié, 2 abr 2025 a las 23:38, Alberto Álvarez (<albertoharing@gmail.com>) escribió:



                El mié, 2 abr 2025 a las 22:01, Alberto Álvarez (<albertoharing@gmail.com>) escribió:


 4 archivos adjuntos•  Analizado por Gmail
	

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINESIZE 512

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
	char buffer[MAX_LINESIZE];
	char string[strlen(argv[2]) + 1];
	strcpy(string, argv[2]);
	int instance = 0;//Variable stores number of times string is found
	FILE *file = fopen(argv[1], "r");
	//Error handling
	if (!file) {
        perror("Error opening file");
        return -1;
	}
	//Main function
	while (fgets(buffer,MAX_LINESIZE,file)){
		if (strstr(buffer, string)) {//Current line = given line
			printf("%s",buffer);
			instance = instance++;
		}
	}
	if (instance == 0) {//No line was concatenated after reading the file
		printf("%s not found.\n",string);
	}
	fclose(file);
	return 0;
}
