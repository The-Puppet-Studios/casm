#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Function to remove leading and trailing whitespace
char *trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while(*str == ' ') str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && (*end == ' ' || *end == ';')) end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

// Function to interpret and run the commands
void run_command(char *line) {
    char *cmd = strtok(line, " ");
    if (cmd != NULL && strcmp(cmd, "out") == 0) {
        char *msg = strtok(NULL, "\"");
        if (msg != NULL) {
            printf("%s\n", trim_whitespace(msg));
        } else {
            printf("Syntax error: expected string after 'out'\n");
        }
    } else {
        printf("Unknown command: %s\n", cmd);
    }
}

// Function to read and interpret a .casm file
void interpret_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file: %s\n", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *trimmed = trim_whitespace(line);
        if (strlen(trimmed) > 0) {
            run_command(trimmed);
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: casm <file.casm>\n");
        return 1;
    }

    interpret_file(argv[1]);
    return 0;
}