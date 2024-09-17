#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>  // Include ctype.h for isspace()

// Function to remove leading and trailing whitespace
char *trim_whitespace(const char *str) {
    // Create a writable copy of the input string
    char *copy = strdup(str);
    if (!copy) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    char *start = copy;
    char *end;

    // Trim leading whitespace
    while (isspace((unsigned char)*start)) start++;

    if (*start == 0) {  // All spaces?
        free(copy);
        return strdup("");  // Return an empty string
    }

    // Trim trailing whitespace
    end = start + strlen(start) - 1;
    while (end > start && (isspace((unsigned char)*end) || *end == ';')) end--;

    // Write new null terminator
    *(end + 1) = 0;

    // Copy the trimmed string into a new buffer and free the original copy
    char *trimmed = strdup(start);
    if (!trimmed) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    free(copy);
    return trimmed;
}

// Function to interpret and run the commands
void run_command(char *line) {
    char *cmd = strtok(line, " ");
    if (cmd != NULL && strcmp(cmd, "out") == 0) {
        char *msg = strtok(NULL, "\"");
        if (msg != NULL) {
            char *trimmed_msg = trim_whitespace(msg);
            printf("%s\n", trimmed_msg);
            free(trimmed_msg);
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
        free(trimmed);
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