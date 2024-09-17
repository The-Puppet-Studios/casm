#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_STRING_LENGTH 255
#define INITIAL_CAPACITY 16

typedef enum {
    PRINT, NUMBER, STRING, IDENTIFIER, END, UNKNOWN
} Token;

typedef struct {
    char *name;
    int value;
} Variable;

typedef struct {
    Variable *variables;
    size_t size;
    size_t capacity;
} VariableTable;

typedef struct {
    const char *source;
    size_t index;
    char strValue[MAX_STRING_LENGTH + 1];
    int numValue;
    char idValue[MAX_STRING_LENGTH + 1];
    VariableTable varTable;
} Lexer;

static void initVariableTable(VariableTable *table) {
    table->variables = malloc(INITIAL_CAPACITY * sizeof(Variable));
    table->size = 0;
    table->capacity = INITIAL_CAPACITY;
}

static void freeVariableTable(VariableTable *table) {
    for (size_t i = 0; i < table->size; ++i) {
        free(table->variables[i].name);
    }
    free(table->variables);
}

static Variable *findVariable(VariableTable *table, const char *name) {
    for (size_t i = 0; i < table->size; ++i) {
        if (strcmp(table->variables[i].name, name) == 0) {
            return &table->variables[i];
        }
    }
    return NULL;
}

static void addVariable(VariableTable *table, const char *name, int value) {
    if (table->size == table->capacity) {
        table->capacity *= 2;
        table->variables = realloc(table->variables, table->capacity * sizeof(Variable));
        if (!table->variables) {
            perror("Error reallocating memory");
            exit(EXIT_FAILURE);
        }
    }
    table->variables[table->size].name = strdup(name);
    if (!table->variables[table->size].name) {
        perror("Error duplicating string");
        exit(EXIT_FAILURE);
    }
    table->variables[table->size].value = value;
    table->size++;
}

static Token nextToken(Lexer *lexer) {
    while (isspace(lexer->source[lexer->index])) {
        lexer->index++;
    }

    if (lexer->source[lexer->index] == '\0') return END;

    char current = lexer->source[lexer->index];
    
    if (current == '"') {
        lexer->index++;
        size_t start = lexer->index;
        while (lexer->source[lexer->index] != '"' && lexer->source[lexer->index] != '\0') {
            lexer->index++;
        }
        if (lexer->source[lexer->index] == '"') {
            if (lexer->index - start > MAX_STRING_LENGTH) {
                fprintf(stderr, "Error: String length exceeds maximum allowed length.\n");
                return UNKNOWN;
            }
            strncpy(lexer->strValue, &lexer->source[start], lexer->index - start);
            lexer->strValue[lexer->index - start] = '\0';
            lexer->index++;
            return STRING;
        }
        return UNKNOWN;
    }
    
    if (isdigit(current)) {
        size_t start = lexer->index;
        while (isdigit(lexer->source[lexer->index])) {
            lexer->index++;
        }
        if (lexer->index - start > MAX_STRING_LENGTH) {
            fprintf(stderr, "Error: Number length exceeds maximum allowed length.\n");
            return UNKNOWN;
        }
        strncpy(lexer->idValue, &lexer->source[start], lexer->index - start);
        lexer->idValue[lexer->index - start] = '\0';
        lexer->numValue = atoi(lexer->idValue);
        return NUMBER;
    }
    
    if (isalpha(current)) {
        size_t start = lexer->index;
        while (isalnum(lexer->source[lexer->index])) {
            lexer->index++;
        }
        if (lexer->index - start > MAX_STRING_LENGTH) {
            fprintf(stderr, "Error: Identifier length exceeds maximum allowed length.\n");
            return UNKNOWN;
        }
        strncpy(lexer->idValue, &lexer->source[start], lexer->index - start);
        lexer->idValue[lexer->index - start] = '\0';
        return IDENTIFIER;
    }
    
    if (strncmp(&lexer->source[lexer->index], "out ", 4) == 0) {
        lexer->index += 4;
        return PRINT;
    }
    
    return UNKNOWN;
}

static void interpret(const char *source) {
    Lexer lexer = { source, 0, "", 0, "", {NULL, 0, 0} };
    initVariableTable(&lexer.varTable);
    
    Token token = nextToken(&lexer);
    
    while (token != END) {
        switch (token) {
            case PRINT:
                token = nextToken(&lexer);
                if (token == STRING) {
                    printf("%s\n", lexer.strValue);
                } else if (token == IDENTIFIER) {
                    Variable *var = findVariable(&lexer.varTable, lexer.idValue);
                    if (var) {
                        printf("Variable '%s': %d\n", lexer.idValue, var->value);
                    } else {
                        printf("Variable '%s' not found\n", lexer.idValue);
                    }
                } else if (token == NUMBER) {
                    printf("%d\n", lexer.numValue);
                } else {
                    fprintf(stderr, "Error: Unexpected token after 'out' at index %zu\n", lexer.index);
                }
                break;
            case IDENTIFIER:
                token = nextToken(&lexer);
                if (token == NUMBER) {
                    Variable *var = findVariable(&lexer.varTable, lexer.idValue);
                    if (var) {
                        var->value = lexer.numValue;
                    } else {
                        addVariable(&lexer.varTable, lexer.idValue, lexer.numValue);
                    }
                    printf("Variable '%s' assigned value %d\n", lexer.idValue, lexer.numValue);
                } else {
                    fprintf(stderr, "Error: Invalid assignment for variable '%s' at index %zu\n", lexer.idValue, lexer.index);
                }
                break;
            case UNKNOWN:
                fprintf(stderr, "Error: Unknown token at index %zu\n", lexer.index);
                break;
            default:
                fprintf(stderr, "Error: Unexpected token at index %zu\n", lexer.index);
                break;
        }
        token = nextToken(&lexer);
    }

    freeVariableTable(&lexer.varTable);
}

static char *readFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    if (length < 0) {
        perror("Error determining file size");
        fclose(file);
        return NULL;
    }
    
    fseek(file, 0, SEEK_SET);

    char *content = malloc(length + 1);
    if (!content) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }
    
    size_t readBytes = fread(content, 1, length, file);
    if (readBytes != (size_t)length) {
        perror("Error reading file");
        free(content);
        fclose(file);
        return NULL;
    }
    
    content[length] = '\0';
    fclose(file);
    return content;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.casm>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *filename = argv[1];
    char *code = readFile(filename);

    if (!code) {
        fprintf(stderr, "Error: Could not read file or file is empty.\n");
        return EXIT_FAILURE;
    }

    interpret(code);
    free(code);

    return EXIT_SUCCESS;
}
