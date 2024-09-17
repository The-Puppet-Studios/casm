#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>  // Include ctype.h for isspace()

#define MAX_VARS 100

// Enumeration for variable types
typedef enum {
    TYPE_INT,
    TYPE_STR,
    TYPE_SML
} VarType;

// Structure to store a variable
typedef struct {
    char name[50];
    VarType type;
    union {
        int intValue;
        char strValue[256];
        int smlValue;
    } value;
} Variable;

// Array to store variables
Variable variables[MAX_VARS];
int var_count = 0;

// Function to remove leading and trailing whitespace (updated)
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

// Function to find a variable by name
Variable* find_variable(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i];
        }
    }
    return NULL;
}

// Function to add a new variable
void add_variable(const char *name, VarType type, const char *value) {
    if (var_count >= MAX_VARS) {
        printf("Variable limit exceeded\n");
        return;
    }

    Variable *var = &variables[var_count++];
    strcpy(var->name, name);
    var->type = type;

    if (type == TYPE_INT) {
        var->value.intValue = atoi(value);
    } else if (type == TYPE_STR) {
        strcpy(var->value.strValue, value);
    } else if (type == TYPE_SML) {
        var->value.smlValue = (strcmp(value, "1") == 0) ? 1 : 0;
    }
}

// Function to declare variables
void declare_variable(char *line) {
    char *type = strtok(line, " ");
    char *name = strtok(NULL, " ");
    char *equals = strtok(NULL, " ");
    char *value = strtok(NULL, "\"");

    if (!type || !name || !equals || !value) {
        printf("Syntax error in variable declaration\n");
        return;
    }

    if (strcmp(type, "int") == 0) {
        add_variable(name, TYPE_INT, value);
    } else if (strcmp(type, "str") == 0) {
        add_variable(name, TYPE_STR, value);
    } else if (strcmp(type, "sml") == 0) {
        add_variable(name, TYPE_SML, value);
    } else {
        printf("Unknown type: %s\n", type);
    }
}

// Function to print a variable value
void print_variable_value(Variable *var) {
    if (var->type == TYPE_INT) {
        printf("%d\n", var->value.intValue);
    } else if (var->type == TYPE_STR) {
        printf("%s\n", var->value.strValue);
    } else if (var->type == TYPE_SML) {
        printf("%d\n", var->value.smlValue);
    }
}

// Function to interpret the 'out' command
void run_command(char *line) {
    char *cmd = strtok(line, " ");
    if (cmd != NULL && strcmp(cmd, "out") == 0) {
        char *msg = strtok(NULL, "\"");
        if (msg != NULL) {
            char *trimmed_msg = trim_whitespace(msg);
            Variable *var = find_variable(trimmed_msg);
            if (var) {
                print_variable_value(var);
            } else {
                printf("%s\n", trimmed_msg);
            }
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
            if (strncmp(trimmed, "int ", 4) == 0 || strncmp(trimmed, "str ", 4) == 0 || strncmp(trimmed, "sml ", 4) == 0) {
                declare_variable(trimmed);
            } else {
                run_command(trimmed);
            }
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