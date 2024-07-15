#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "panic.h"

char* opcode[] ={
    "mov", "cmp", "add", "sub",
    "lea", "clr", "not", "inc", 
    "dec", "jmp", "bne", "red",
    "prn", "jsr", "rts", "stop"
};

char* register_names[] = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
};

char* directive_list[] = {
    ".data", ".string"
};

char* labels[] = {
    "MAIN:", "LOOP:", "END:"
};

int line = 0, collumn;

char* strndup(const char* src, size_t n) {
    size_t len = strlen(src);
    size_t i;
    char* dup;
    if (n < len) {
        len = n;
    }

    dup = (char*)malloc(len + 1); /* Allocate memory for the new string (+1 for the null terminator) */
    if (dup == NULL) {
        return NULL; /* Allocation failed */
    }

    for (i = 0; i < len; ++i) {
        dup[i] = src[i];
    }
    dup[len] = '\0'; /* Null-terminate the duplicated string */

    return dup;
}

Token* tokenize(char* input, int line_number) {
    int i = 0;
    char* ptr = input;
    Token* tokens = malloc(sizeof(Token) * 256);
    if (!tokens) {
        memory_allocation_failure();
    }
    collumn = 0;
    while(*ptr) {
        collumn++;
        
        if (*ptr == '\n') line++;
        
        if (isspace(*ptr)) {
            ptr++;
            continue;
        }
        /* Id instructions */
        if (islower(*ptr)) {
            char* start = ptr;
            char* temp;
            while (isalpha(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            if (is_opcocde(temp)) {
                Token token = create_token(TOKEN_INSTRUCTION, temp, line_number, collumn);
                tokens[i++] = token;
                continue;
            }
            ptr = start;
        }
        /* Id regiesters */
        if (islower(*ptr)) {
            char* start = ptr;
            char* temp;
            while (isdigit(*ptr) || islower(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            if (is_register(temp)) {
                Token token = create_token(TOKEN_REGISTER, temp, line_number, collumn);
                tokens[i++] = token;
                continue;
            }
            ptr = start;
        }
        /* Id labels */
        if (isalnum(*ptr)) {
            char* start = ptr;
            char* temp; 
            while (*ptr == ':' || isalnum(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            if (is_label(temp)) {
                Token token = create_token(TOKEN_LABEL, temp, line_number, collumn);
                tokens[i++] = token;
                ptr++;
                continue; 
            }
            ptr = start;
        }
        /* Id directives*/
        if (*ptr == '.') {
            char* start = ptr;
            char* temp;
            while (!isspace(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            if (is_directive(temp)) {
                Token token = create_token(TOKEN_DIRECTIVE, temp, line_number, collumn);
                tokens[i++] = token;
                continue;
            }
            ptr = start;
        }
        /* Id number */
        if (*ptr == '-' || isdigit(*ptr)) {
            char* start = ptr;
            char* temp;
            while (!isspace(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            if (is_valid_int(temp)) {
                Token token = create_token(TOKEN_NUMBER, temp, line_number, collumn);
                tokens[i++] = token;
                continue;
            }
            ptr = start;
        }
        /* Id string */
        if (*ptr == '"') {
            char* start = ptr;
            char* temp;
            while (*ptr != '"' && *ptr != EOF) ptr++;
            temp = strndup(start, ptr - start);
            if (is_valid_string(temp)) {
                Token token = create_token(TOKEN_STRING, temp, line_number, collumn);
                tokens[i++] = token;
                continue;
            }
            ptr = start;
        }
        /* Id comma */
        if (*ptr == ',') {
            tokens[i++] = create_token(TOKEN_COMMA, ",", line_number, collumn);
            ptr++;
            continue;
        }
        /* This is a temporery function for identification of unknowen labels
           this needs to be changed  */
        if (isupper(*ptr)) {
            char* start = ptr;
            char* temp;
            while (!isspace(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            if (1) {
                Token token = create_token(TOKEN_LABEL, temp, line_number, collumn);
                tokens[i++] = token;
                continue;
            }
        }
        // Nedd to add a function to id "*" and "#"

    }
    tokens[i] = create_token(TOKEN_EOF, "", line, collumn);
    return tokens;
}

int is_opcocde(char* temp) {
    int i, len = sizeof(opcode) / sizeof(opcode[0]);
    for (i = 0; i < len; i++) { 
        if (strcmp(temp, opcode[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_register(char* temp) {
    int i, len = sizeof(register_names) / sizeof(register_names[0]);
    for (i = 0; i < len; i++) {
        if (strcmp(temp, register_names[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_label(char* temp) {
    int i, len = sizeof(labels) / sizeof(labels[0]);
    if (!is_register(temp)) {
        for (i = 0; i < len; i++) {
            if (strcmp(temp, labels[i]) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

int is_directive(char* temp) {
    int i, len = sizeof(directive_list) / sizeof(directive_list[0]);
    for (i = 0; i < len; i++) {
        if (strcmp(temp, directive_list[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_valid_int(char* temp) {
    if (strstr(temp, ".") == NULL) {
        return 1;
    }
    return 0;
}

int is_valid_string(char* temp) {
    if (strstr(temp, "\"") != NULL) {
        return 1;
    }
    return 0;
}

Token create_token(TokenType type, char* val, int line, int collumn) {
    Token *token = malloc(sizeof(Token));
    if (token == NULL) {
        memory_allocation_failure();
    }
    token->type = type;
    token->val = val;
    token->line = line;
    token->collumn = collumn;

    return *token;
}

void print_token_list(Token* tokens) {
    int curr_line = 0, i;
    for (i = 0; tokens[i].type != TOKEN_EOF ; i++) {
        if (tokens[i].line > curr_line) {
            curr_line++;
            printf("========== Line number %d ==========\n", curr_line);
        }
        printf("%s (%u)\n", tokens[i].val, tokens[i].type);
    }
}

char* read_file(const char* filename) {
    FILE *file = fopen(filename, "r");
    long file_size;
    char *buffer; 
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    buffer = (char*)malloc((file_size + 1) * sizeof(char));
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    fread(buffer, sizeof(char), file_size, file);
    buffer[file_size] = '\0';

    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    const char* filename;
    char line_content[MAX_LINE_LENGTH];
    Token* tokens;
    Token* current;
    FILE *file;
    int line_number = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    filename = argv[1];
    file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    while (fgets(line_content, sizeof(line_content), file)) {
        line_number++;
        tokens = tokenize(line_content, line_number);

        current = tokens;
        while (current && current->type != TOKEN_EOF) {
            printf("Token Type: %d, Value: \"%s\", Line: %d, Column: %d\n",
                   current->type, current->val, current->line, current->collumn);
            current++;
        }

        free(tokens);
    }

    fclose(file);
    return EXIT_SUCCESS;
}
