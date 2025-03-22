#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char ** argv) {
    // search for text string in a file and displays the lines where that str appears
    //On error, it gives: perror ("text describing error) and returns -1") Also,
    //If the str is not found, it will display the message "%s not found \n"


    // 3 args are expected
    if (argc != 3) {
        printf("Usage: %s <ruta_fichero> <cadena_busqueda>\n", argv[0]);
        return -1;
    }

    return 0;
}
