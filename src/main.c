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

// Function to evaluate a condition in an if statement
int evaluate_condition(const char *var_name, const char *operator, const char *value) {
    Variable *var = find_variable(var_name);
    if (!var) {
        printf("Variable %s not found\n", var_name);
        return 0;
    }

    int int_value = atoi(value);

    if (strcmp(operator, "==") == 0) {
        return var->value.intValue == int_value;
    }

    return 0;
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

// Function to check if a string is a number
int is_number(const char *str) {
    while (*str) {
        if (!isdigit(*str) && *str != '-') {
            return 0;
        }
        str++;
    }
    return 1;
}

// Function to interpret the 'out' command with handling for quoted strings and numbers
void run_command(char *line, int line_num) {
    char *cmd = strtok(line, " ");
    if (cmd != NULL && strcmp(cmd, "out") == 0) {
        char *msg = strtok(NULL, "");
        if (msg != NULL) {
            char *trimmed_msg = trim_whitespace(msg);

            // Check if the message is in double quotes
            if (trimmed_msg[0] == '"' && trimmed_msg[strlen(trimmed_msg) - 1] == '"') {
                // Print the string without the quotes
                trimmed_msg[strlen(trimmed_msg) - 1] = '\0';  // Remove the closing quote
                printf("%s\n", trimmed_msg + 1);  // Skip the opening quote
            } 
            // Check if the message is a number
            else if (is_number(trimmed_msg)) {
                printf("%s\n", trimmed_msg);
            } 
            // Otherwise, treat it as a variable
            else {
                Variable *var = find_variable(trimmed_msg);
                if (var) {
                    print_variable_value(var);
                } else {
                    printf("Error: Unknown variable used on line %d\n", line_num);
                }
            }
            free(trimmed_msg);
        } else {
            printf("Syntax error: expected string after 'out'\n");
        }
    } else {
        printf("Unknown command: %s\n", cmd);
    }
}

// Function to process an if-else block and ensure proper line number tracking
void process_if_block(FILE *file, char *condition_line, int *line_num) {
    // Parse the condition
    char *if_keyword = strtok(condition_line, " ");
    char *var_name = strtok(NULL, " ");
    char *operator = strtok(NULL, " ");
    char *value = strtok(NULL, " ");

    if (!if_keyword || !var_name || !operator || !value) {
        printf("Syntax error in if statement\n");
        return;
    }

    int condition_met = evaluate_condition(var_name, operator, value);
    int inside_else_block = 0;

    char line[256];
    int end_found = 0;  // Flag to check if 'end' is found

    // Process the lines within the if-else block
    while (fgets(line, sizeof(line), file)) {
        (*line_num)++;  // Increment the line number as we read each line

        char *trimmed = trim_whitespace(line);

        // Check for the end of the if-else block
        if (strcmp(trimmed, "end") == 0) {
            end_found = 1;  // Set the flag to true if 'end' is found
            free(trimmed);
            break;
        }

        if (strcmp(trimmed, "else") == 0) {
            inside_else_block = 1;
            free(trimmed);
            continue;
        }

        // Only run commands inside the correct block
        if ((condition_met && !inside_else_block) || (!condition_met && inside_else_block)) {
            run_command(trimmed, *line_num);  // Pass line_num by reference
        }

        free(trimmed);
    }

    // Check if 'end' was found
    if (!end_found) {
        printf("Error: Missing end statement after if else\n");
    }
}

// Function to read and interpret a .casm file with proper line tracking
void interpret_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file: %s\n", filename);
        return;
    }

    char line[256];
    int line_num = 0;  // Initialize the line number
    while (fgets(line, sizeof(line), file)) {
        line_num++;  // Increment line number for each line read
        char *trimmed = trim_whitespace(line);
        if (strlen(trimmed) > 0) {
            if (strncmp(trimmed, "int ", 4) == 0 || strncmp(trimmed, "str ", 4) == 0 || strncmp(trimmed, "sml ", 4) == 0) {
                declare_variable(trimmed);
            } else if (strncmp(trimmed, "if ", 3) == 0) {
                process_if_block(file, trimmed, &line_num);  // Pass line_num by reference
            } else {
                run_command(trimmed, line_num);  // Pass current line number to run_command
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