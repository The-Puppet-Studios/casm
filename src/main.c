#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_VARS 100

typedef enum {
    TYPE_INT, // Integer type
    TYPE_STR, // String type
    TYPE_SML  // Small integer (0 or 1) type
} VarType;

typedef struct {
    char name[50]; // Variable name
    VarType type;  // Variable type
    union {
        int intValue;          // Integer value
        char strValue[256];    // String value
        int smlValue;         // Small integer value
    } value;
} Variable;

Variable variables[MAX_VARS]; // Array of variables
int var_count = 0; // Current count of variables

// Function to find a variable by name
Variable* find_variable(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i]; // Return pointer to variable if found
        }
    }
    return NULL; // Return NULL if not found
}

// Function to add a new variable
void add_variable(const char *name, VarType type, const char *value) {
    if (var_count >= MAX_VARS) {
        printf("Variable limit exceeded\n"); // Check for maximum variable limit
        return;
    }

    Variable *var = &variables[var_count++]; // Get the next variable slot
    strcpy(var->name, name); // Copy the variable name
    var->type = type; // Set the variable type

    // Assign the value based on the variable type
    if (type == TYPE_INT) {
        var->value.intValue = atoi(value);
    } else if (type == TYPE_STR) {
        strcpy(var->value.strValue, value);
    } else if (type == TYPE_SML) {
        var->value.smlValue = (strcmp(value, "1") == 0) ? 1 : 0;
    }
}

// Function to trim whitespace from a string
char *trim_whitespace(const char *str) {
    char *copy = strdup(str); // Duplicate the string
    if (!copy) {
        perror("Failed to allocate memory"); // Check memory allocation
        exit(EXIT_FAILURE);
    }

    char *start = copy; // Start pointer for trimming
    char *end;

    // Trim leading whitespace
    while (isspace((unsigned char)*start)) start++;

    if (*start == 0) { // If the string is empty
        free(copy);
        return strdup(""); // Return an empty string
    }

    end = start + strlen(start) - 1; // End pointer for trimming
    // Trim trailing whitespace and semicolons
    while (end > start && (isspace((unsigned char)*end) || *end == ';')) end--;

    *(end + 1) = 0; // Null-terminate the trimmed string

    char *trimmed = strdup(start); // Duplicate the trimmed string
    if (!trimmed) {
        perror("Failed to allocate memory"); // Check memory allocation
        exit(EXIT_FAILURE);
    }

    free(copy); // Free the original copy
    return trimmed; // Return trimmed string
}

// Function to get user input
void get_input(char *line) {
    char *saveptr;
    char *type = strtok_r(line, " ", &saveptr); // Extract type
    char *name = strtok_r(NULL, " ", &saveptr); // Extract variable name
    char *prompt = saveptr; // Remaining line as prompt

    if (!type || !name || !prompt) {
        printf("Syntax error in input statement\n");
        return;
    }

    // Remove leading and trailing quotes from prompt
    prompt = strtok(prompt, "\"");
    if (!prompt) {
        printf("Syntax error: missing prompt in input statement\n");
        return;
    }

    printf("%s", prompt); // Print the prompt for user input
    char input[256];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Error reading input\n");
        return;
    }
    input[strcspn(input, "\n")] = 0; // Remove newline character

    // Store input based on variable type
    if (strcmp(type, "str") == 0) {
        add_variable(name, TYPE_STR, input);
    } else if (strcmp(type, "int") == 0) {
        char *endptr;
        long value = strtol(input, &endptr, 10);
        if (*endptr != '\0') {
            printf("Error: Input for %s must be an integer\n", name);
            return;
        }
        char intStr[20];
        snprintf(intStr, sizeof(intStr), "%ld", value);
        add_variable(name, TYPE_INT, intStr);
    } else if (strcmp(type, "sml") == 0) {
        if (strcmp(input, "0") == 0 || strcmp(input, "1") == 0) {
            add_variable(name, TYPE_SML, input);
        } else {
            printf("Error: Input for %s must be 0 or 1\n", name);
            return;
        }
    } else {
        printf("Unknown type: %s\n", type);
    }
}

// Function to run a command from a line
void run_command(char *line, int line_num) {
    char *cmd = strtok(line, " "); // Extract command
    if (cmd != NULL && strcmp(cmd, "out") == 0) {
        char *rest = line + strlen(cmd) + 1;  // Point to the rest of the line after "out "
        char *trimmed_msg = trim_whitespace(rest);
        if (trimmed_msg[0] == '"' && trimmed_msg[strlen(trimmed_msg) - 1] == '"') {
            // It's a string literal, print it directly
            trimmed_msg[strlen(trimmed_msg) - 1] = '\0';  // Remove the closing quote
            printf("%s\n", trimmed_msg + 1);  // Print from after the opening quote
        } else {
            // It's a variable name
            Variable *var = find_variable(trimmed_msg);
            if (var) {
                // Print the value based on variable type
                if (var->type == TYPE_INT) {
                    printf("%d\n", var->value.intValue);
                } else if (var->type == TYPE_STR) {
                    printf("%s\n", var->value.strValue);
                } else if (var->type == TYPE_SML) {
                    printf("%d\n", var->value.smlValue);
                }
            } else {
                printf("Error: Unknown variable '%s' used on line %d\n", trimmed_msg, line_num);
            }
        }
        free(trimmed_msg); // Free allocated memory for trimmed message
    } else if (cmd != NULL && strcmp(cmd, "in") == 0) {
        get_input(line + 3); // Skip "in " and pass the rest of the line
    } else {
        printf("Unknown command: %s\n", cmd);
    }
}

// Function to process an if block
void process_if_block(FILE *file, char *condition_line, int *line_num) {
    char *if_keyword = strtok(condition_line, " ");
    char *var_name = strtok(NULL, " ");
    char *operator = strtok(NULL, " ");
    char *value = strtok(NULL, " ");

    if (!if_keyword || !var_name || !operator || !value) {
        printf("Syntax error in if statement\n");
        return;
    }

    Variable *var = find_variable(var_name);
    if (!var) {
        printf("Error: Variable %s not found on line %d\n", var_name, *line_num);
        return;
    }

    // Check if the condition is met
    int condition_met = (strcmp(operator, "==") == 0) && (var->value.intValue == atoi(value));
    int inside_else_block = 0;

    char line[256];
    int end_found = 0;

    // Read lines until "end" or EOF
    while (fgets(line, sizeof(line), file)) {
        (*line_num)++;
        char *trimmed = trim_whitespace(line);

        if (strcmp(trimmed, "end") == 0) {
            end_found = 1; // Mark end of if block
            free(trimmed);
            break;
        }

        if (strcmp(trimmed, "else") == 0) {
            inside_else_block = 1; // Mark entry into else block
            free(trimmed);
            continue;
        }

        // Execute commands based on condition
        if ((condition_met && !inside_else_block) || (!condition_met && inside_else_block)) {
            run_command(trimmed, *line_num);
        }

        free(trimmed); // Free trimmed line memory
    }

    if (!end_found) {
        printf("Error: Missing end statement after if else on line %d\n", *line_num);
    }
}

// Function to interpret a file
void interpret_file(const char *filename) {
    FILE *file = fopen(filename, "r"); // Open file for reading
    if (!file) {
        printf("Could not open file: %s\n", filename);
        return;
    }

    char line[256];
    int line_num = 0;
    // Read lines from file
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        char *trimmed = trim_whitespace(line); // Trim the line
        
        // Check if the line is a comment
        if (strncmp(trimmed, "#", 1) == 0) {
            continue; // Skip comment lines
        }

        if (strlen(trimmed) > 0) { // Process non-empty lines
            if (strncmp(trimmed, "int ", 4) == 0 || strncmp(trimmed, "str ", 4) == 0 || strncmp(trimmed, "sml ", 4) == 0) {
                char *type = strtok(trimmed, " ");
                char *name = strtok(NULL, " ");
                char *equals = strtok(NULL, " ");
                char *value = strtok(NULL, "\"");
                if (value) {
                    add_variable(name, (strcmp(type, "int") == 0) ? TYPE_INT : (strcmp(type, "str") == 0) ? TYPE_STR : TYPE_SML, value);
                }
            } else if (strncmp(trimmed, "if ", 3) == 0) {
                process_if_block(file, trimmed, &line_num); // Process if statement
            } else {
                run_command(trimmed, line_num); // Run other commands
            }
        }
        free(trimmed); // Free trimmed line memory
    }

    fclose(file); // Close the file
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: casm <file.casm>\n"); // Check command line arguments
        return 1;
    }

    interpret_file(argv[1]); // Interpret the given file
    return 0;
}