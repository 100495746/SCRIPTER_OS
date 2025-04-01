#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINESIZE 500
#define MAX_LINENUMB 30

int main(int argc, char **argv) {
	if (argc != 3) {
    	printf("Usage: %s <file_path> <string_to_search>\n", argv[0]);
    	return -1;
	}

	FILE *file = fopen(argv[1], "r");
	if (!file) {
    	perror("Error opening file");
    	return -1;
	}

	char buffer[MAX_LINESIZE];
	char in_lines[MAX_LINESIZE * MAX_LINENUMB] = "";
	char string[strlen(argv[2]) + 1];
	strcpy(string, argv[2]);

	while (fgets(buffer, MAX_LINESIZE, file)) {
    	if (strstr(buffer, string)) {
        	printf("%s", buffer);
        	strcat(in_lines, buffer);
    	}
	}

	if (strlen(in_lines) == 0) {
    	printf("%s not found.\n", string);
	}

	fclose(file);
	return 0;
}
