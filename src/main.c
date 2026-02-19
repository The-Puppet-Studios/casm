#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ─────────────────────────────────────────────
   CONSTANTS
   ───────────────────────────────────────────── */

#define MAX_VARIABLES 100   /* Maximum number of variables a script can declare */
#define MAX_NAME_LEN  50    /* Maximum length of a variable name */
#define MAX_STR_LEN   256   /* Maximum length of a string variable's value */

/* ─────────────────────────────────────────────
   TYPES
   ───────────────────────────────────────────── */

/* The three variable types supported by CASM:
     int  — a whole number (e.g. 42)
     str  — a piece of text (e.g. "hello")
     sml  — a tiny flag, either 0 or 1 */
typedef enum {
    TYPE_INT,
    TYPE_STR,
    TYPE_SML
} VarType;

/* One variable: a name, a type, and a value. */
typedef struct {
    char   name[MAX_NAME_LEN];
    VarType type;
    union {
        int  intValue;          /* used when type == TYPE_INT */
        char strValue[MAX_STR_LEN]; /* used when type == TYPE_STR */
        int  smlValue;          /* used when type == TYPE_SML (0 or 1) */
    } value;
} Variable;

/* ─────────────────────────────────────────────
   GLOBAL VARIABLE STORE
   ───────────────────────────────────────────── */

Variable variables[MAX_VARIABLES]; /* flat array holding all declared variables */
int      variable_count = 0;       /* how many variables have been declared so far */

/* ─────────────────────────────────────────────
   VARIABLE HELPERS
   ───────────────────────────────────────────── */

/* Look up a variable by name.
   Returns a pointer to the Variable if found, or NULL if it doesn't exist. */
Variable *find_variable(const char *name) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i];
        }
    }
    return NULL;
}

/* Add a new variable to the store.
   - name  : the variable's identifier
   - type  : TYPE_INT, TYPE_STR, or TYPE_SML
   - value : the initial value as a raw string (e.g. "42", "hello", "1") */
void add_variable(const char *name, VarType type, const char *value) {
    if (variable_count >= MAX_VARIABLES) {
        printf("Error: Too many variables (limit is %d)\n", MAX_VARIABLES);
        return;
    }

    Variable *var = &variables[variable_count++];
    strcpy(var->name, name);
    var->type = type;

    if (type == TYPE_INT) {
        var->value.intValue = atoi(value);
    } else if (type == TYPE_STR) {
        strcpy(var->value.strValue, value);
    } else if (type == TYPE_SML) {
        /* Only "1" counts as true; everything else is 0. */
        var->value.smlValue = (strcmp(value, "1") == 0) ? 1 : 0;
    }
}

/* ─────────────────────────────────────────────
   STRING UTILITY
   ───────────────────────────────────────────── */

/* Return a newly-allocated copy of str with:
     - leading whitespace removed
     - trailing whitespace AND trailing semicolons removed
   Caller is responsible for free()-ing the result. */
char *trim_whitespace(const char *str) {
    /* Work on a duplicate so we don't modify the caller's string. */
    char *copy = strdup(str);
    if (!copy) {
        perror("trim_whitespace: strdup failed");
        exit(EXIT_FAILURE);
    }

    /* Skip past any leading spaces/tabs/newlines. */
    char *start = copy;
    while (isspace((unsigned char)*start)) {
        start++;
    }

    /* If we hit the end of the string it was all whitespace. */
    if (*start == '\0') {
        free(copy);
        return strdup("");
    }

    /* Walk back from the end, stripping trailing whitespace and semicolons. */
    char *end = start + strlen(start) - 1;
    while (end > start && (isspace((unsigned char)*end) || *end == ';')) {
        end--;
    }
    *(end + 1) = '\0'; /* null-terminate the trimmed region */

    char *trimmed = strdup(start);
    if (!trimmed) {
        perror("trim_whitespace: second strdup failed");
        exit(EXIT_FAILURE);
    }

    free(copy);
    return trimmed;
}

/* ─────────────────────────────────────────────
   BUILT-IN COMMANDS
   ───────────────────────────────────────────── */

/* Handle the "in" command, which prompts the user for input and stores it.
   Format in script:  in <type> <varname> "prompt text"
   Example:           in int age "Enter your age: " */
void cmd_input(char *args) {
    char *saveptr;

    char *type_str = strtok_r(args,   " ", &saveptr); /* e.g. "int"          */
    char *var_name = strtok_r(NULL,   " ", &saveptr); /* e.g. "age"          */
    char *prompt   = saveptr;                         /* e.g. "Enter your age: " */

    if (!type_str || !var_name || !prompt) {
        printf("Syntax error: 'in' expects: in <type> <name> \"prompt\"\n");
        return;
    }

    /* Strip the surrounding quotes from the prompt string. */
    prompt = strtok(prompt, "\"");
    if (!prompt) {
        printf("Syntax error: prompt string must be surrounded by quotes\n");
        return;
    }

    /* Show the prompt and read a line of input from the user. */
    printf("%s", prompt);
    char input[MAX_STR_LEN];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Error: could not read input\n");
        return;
    }
    input[strcspn(input, "\n")] = '\0'; /* strip the trailing newline */

    /* Validate and store the input according to the requested type. */
    if (strcmp(type_str, "str") == 0) {
        add_variable(var_name, TYPE_STR, input);

    } else if (strcmp(type_str, "int") == 0) {
        char *endptr;
        long num = strtol(input, &endptr, 10);
        if (*endptr != '\0') {
            printf("Error: '%s' is not a valid integer for variable '%s'\n", input, var_name);
            return;
        }
        char num_str[32];
        snprintf(num_str, sizeof(num_str), "%ld", num);
        add_variable(var_name, TYPE_INT, num_str);

    } else if (strcmp(type_str, "sml") == 0) {
        if (strcmp(input, "0") == 0 || strcmp(input, "1") == 0) {
            add_variable(var_name, TYPE_SML, input);
        } else {
            printf("Error: sml variable '%s' must be 0 or 1, got '%s'\n", var_name, input);
        }

    } else {
        printf("Error: unknown type '%s' in 'in' command\n", type_str);
    }
}

/* Handle the "out" command, which prints a value to the screen.
   Format:  out "some literal text"
       or:  out <varname>
   line_num is used only for error messages. */
void cmd_output(char *args, int line_num) {
    char *trimmed = trim_whitespace(args);

    if (trimmed[0] == '"' && trimmed[strlen(trimmed) - 1] == '"') {
        /* It's a string literal — print the text between the quotes. */
        trimmed[strlen(trimmed) - 1] = '\0'; /* remove closing quote */
        printf("%s\n", trimmed + 1);          /* skip opening quote  */
    } else {
        /* It's a variable name — look it up and print its value. */
        Variable *var = find_variable(trimmed);
        if (var) {
            if      (var->type == TYPE_INT) printf("%d\n", var->value.intValue);
            else if (var->type == TYPE_STR) printf("%s\n", var->value.strValue);
            else if (var->type == TYPE_SML) printf("%d\n", var->value.smlValue);
        } else {
            printf("Error: unknown variable '%s' on line %d\n", trimmed, line_num);
        }
    }

    free(trimmed);
}

/* ─────────────────────────────────────────────
   COMMAND DISPATCHER
   ───────────────────────────────────────────── */

/* Parse the first word of a line to decide which command to run. */
void run_command(char *line, int line_num) {
    /* strtok modifies the string, so grab the command word first. */
    char *command = strtok(line, " ");
    if (command == NULL) return;

    if (strcmp(command, "out") == 0) {
        /* Everything after "out " is the argument. */
        char *args = line + strlen(command) + 1;
        cmd_output(args, line_num);

    } else if (strcmp(command, "in") == 0) {
        /* Everything after "in " is the argument (type, name, prompt). */
        cmd_input(line + strlen(command) + 1);

    } else {
        printf("Error: unknown command '%s' on line %d\n", command, line_num);
    }
}

/* ─────────────────────────────────────────────
   IF / ELSE / END BLOCK
   ───────────────────────────────────────────── */

/* Process an if-block read from the file.
   Format in script:
       if <varname> == <value>
           ...true branch commands...
       else
           ...false branch commands...
       end

   Only the "==" operator is currently supported.
   line_num is updated as new lines are read so error messages stay accurate. */
void process_if_block(FILE *file, char *condition_line, int *line_num) {
    /* Parse:  if  <varname>  ==  <value> */
    strtok(condition_line, " ");           /* discard "if" keyword */
    char *var_name  = strtok(NULL, " ");   /* variable to test     */
    char *op        = strtok(NULL, " ");   /* operator (must be ==) */
    char *cmp_value = strtok(NULL, " ");   /* value to compare against */

    if (!var_name || !op || !cmp_value) {
        printf("Syntax error: 'if' expects: if <var> == <value>\n");
        return;
    }

    Variable *var = find_variable(var_name);
    if (!var) {
        printf("Error: variable '%s' not found (line %d)\n", var_name, *line_num);
        return;
    }

    /* Evaluate the condition.
       Currently only integer equality (==) is supported. */
    int condition_is_true = (strcmp(op, "==") == 0)
                            && (var->value.intValue == atoi(cmp_value));

    int in_else_branch = 0; /* becomes 1 after we see the "else" keyword */
    int found_end      = 0;
    char line[MAX_STR_LEN];

    /* Read lines until we hit "end" (or run out of file). */
    while (fgets(line, sizeof(line), file)) {
        (*line_num)++;
        char *trimmed = trim_whitespace(line);

        if (strcmp(trimmed, "end") == 0) {
            found_end = 1;
            free(trimmed);
            break;
        }

        if (strcmp(trimmed, "else") == 0) {
            in_else_branch = 1;
            free(trimmed);
            continue;
        }

        /* Run this line only if we're in the matching branch:
             - true branch  → condition passed and we haven't hit else yet
             - false branch → condition failed and we're past else          */
        int should_run = (condition_is_true && !in_else_branch)
                      || (!condition_is_true && in_else_branch);

        if (should_run) {
            run_command(trimmed, *line_num);
        }

        free(trimmed);
    }

    if (!found_end) {
        printf("Error: if-block starting before line %d has no matching 'end'\n", *line_num);
    }
}

/* ─────────────────────────────────────────────
   MAIN INTERPRETER LOOP
   ───────────────────────────────────────────── */

/* Open a .casm file and execute it line by line. */
void interpret_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: could not open file '%s'\n", filename);
        return;
    }

    char line[MAX_STR_LEN];
    int  line_num = 0;

    while (fgets(line, sizeof(line), file)) {
        line_num++;
        char *trimmed = trim_whitespace(line);

        /* Skip blank lines and comment lines (lines starting with #). */
        if (strlen(trimmed) == 0 || trimmed[0] == '#') {
            free(trimmed);
            continue;
        }

        /* ── Variable declaration ─────────────────────────────────────
           Format:  <type> <name> = <value>
           Example: int score = 0
                    str greeting = "hello"
                    sml flag = 1                                        */
        if (strncmp(trimmed, "int ", 4) == 0 ||
            strncmp(trimmed, "str ", 4) == 0 ||
            strncmp(trimmed, "sml ", 4) == 0) {

            char *type_str = strtok(trimmed, " "); /* "int" / "str" / "sml" */
            char *var_name = strtok(NULL,   " "); /* variable name          */
                             strtok(NULL,   " "); /* skip the "=" sign      */
            char *value    = strtok(NULL,  "\""); /* value (strips quotes)  */

            if (value) {
                VarType type;
                if      (strcmp(type_str, "int") == 0) type = TYPE_INT;
                else if (strcmp(type_str, "str") == 0) type = TYPE_STR;
                else                                    type = TYPE_SML;
                add_variable(var_name, type, value);
            }

        /* ── If statement ─────────────────────────────────────────────
           Hands control over to process_if_block() which reads lines
           from the file until it finds the matching "end".            */
        } else if (strncmp(trimmed, "if ", 3) == 0) {
            process_if_block(file, trimmed, &line_num);

        /* ── Everything else is treated as a command (out, in, …) ─── */
        } else {
            run_command(trimmed, line_num);
        }

        free(trimmed);
    }

    fclose(file);
}

/* ─────────────────────────────────────────────
   ENTRY POINT
   ───────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: casm <file.casm>\n");
        return 1;
    }

    const char *filename = argv[1];

    /* Find the file extension (the last '.' in the filename). */
    const char *extension = strrchr(filename, '.');

    if (extension == NULL) {
        printf("Error: file has no extension — please provide a .casm file\n");
        return 1;
    }

    if (strcmp(extension, ".casmpp") == 0) {
        printf("Error: this is a CASM++ file — use the casm++ interpreter instead\n");
        return 1;
    }

    if (strcmp(extension, ".casm") != 0) {
        printf("Error: '%s' is not a .casm file\n", filename);
        return 1;
    }

    interpret_file(filename);
    return 0;
}