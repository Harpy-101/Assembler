#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "panic.h"
#include "preprocessor.h"


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
    ".data", ".string", ".entry", ".extern"
};

/* Remove this after testing */
char* labels[] = {
    "MAIN:", "LOOP:", "END:"
};

int line = 0, collumn;

int panic = 0;

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
/**
 * @brief 
 * 
 * @param input 
 * @param line_number 
 * @param symbol_table 
 * @param unresolved_list 
 * @param file_name 
 * @return Token*
 * 
 * TODO: Add a case for handling comments at the start of the line (valid)
 * TODO: Add a case for handling comments in the middle of a line (not-valid) */
void tokenize(char* input, int line_number, MacroList* macro_list, int* token_error_flag, Token* tokens) {
    int i = 0;
    char* ptr = input;
    /*Token* tokens = malloc(sizeof(Token) * 256); Chnage this into a dynamic structure after tezsting 
    */
    addressing_mode mode = TBD;
    /*if (!tokens) {
        memory_allocation_failure();
    }*/
    collumn = 0;
    while(*ptr) {
        collumn++;
        mode = TBD; 
        if (*ptr == '\n') line++;
        
        if (isspace(*ptr)) {
            ptr++;
            continue;
        }

        /* Addressing modes */
        if (*ptr == '#') {
            mode = IMIDIATE;
            ptr++;
        }
        else if (*ptr == '*') {
            mode = INDIRECT_REGISTER;
            ptr++;
        }

        /* Id instructions */
        if (islower(*ptr)) {
            char* start = ptr;
            char* temp;
            while (isalpha(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            if (is_opcocde(temp)) {
                Token token;
                (mode == TBD) ? (mode = TBD) : (mode = mode);
                token = create_token(TOKEN_INSTRUCTION, temp, line_number, collumn, mode);
                /*add_token(token_list, token);*/
                tokens[i++] = token;
                continue;
            }
            free(temp);
            ptr = start;
        }
        /* Id regiesters */
        if (islower(*ptr)) {
            char* start = ptr;
            char* temp;
            while (isdigit(*ptr) || islower(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            if (is_register(temp)) {
                Token token;
                (mode == TBD) ? (mode = DIRECT_REGISTER) : (mode = INDIRECT_REGISTER);
                token = create_token(TOKEN_REGISTER, temp, line_number, collumn, mode);
                /*add_token(token_list, token);*/
                tokens[i++] = token;
                continue;
            }
            free(temp);
            ptr = start;
        }
        /* Id labels */
        if (isalpha(*ptr)) {
            char* start = ptr;
            char* temp; 
            while (*ptr == ':' || isalnum(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            if (*(ptr - 1) == ':') {
                remove_collon(temp);
                if (is_label(macro_list, temp)) {
                    Token token;
                    (mode == TBD) ? (mode = DIRECT) : (mode = mode);
                    token = create_token(TOKEN_LABEL_DEFENITION, temp, line_number, collumn, mode);
                    /*add_token(token_list, token);*/
                    tokens[i++] = token;
                    ptr++;
                    /*insert_symbol(symbol_table, token.val, line_number); */
                    continue; 
                }
                printf("panic! at line %d: a label can't be the same name as an opcode, register or a macro\n", line_number);
                *token_error_flag = 1;
            }
            free(temp);
            ptr = start;
        }
        /* Id directives*/
        if (*ptr == '.') {
            char* start = ptr;
            char* temp;
            while (!isspace(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            if (is_directive(temp)) {
                Token token = create_token(TOKEN_DIRECTIVE, temp, line_number, collumn, mode);
                /*add_token(token_list, token);*/
                tokens[i++] = token;
                continue;
            }
            ptr = start;
        }
        /* Id number */
        if (*ptr == '-' || *ptr == '+' || isdigit(*ptr)) {
            char* start = ptr;
            char* temp;
            while (!isspace(*ptr) && (*ptr) != ',') ptr++;
            temp = strndup(start, ptr - start);
            if (is_valid_int(temp)) {
               Token token;
                token = create_token(TOKEN_NUMBER, temp, line_number, collumn, mode);
                /*add_token(token_list, token);*/
                tokens[i++] = token;
                continue;
            }
            free(temp);
            ptr = start;
        }
        /* Id string */
        if (*ptr == '"') {
            char* start = ++ptr;
            char* temp;
            ptr++;
            while (*ptr != '"' && *ptr != EOF) ptr++;
            if (*ptr == '"') {
                temp = strndup(start, ptr - start);
                if (is_valid_string(temp)) {
                    Token token = create_token(TOKEN_STRING, temp, line_number, collumn, mode);
                    /*add_token(token_list, token);*/
                    tokens[i++] = token;
                    ptr++;
                    continue;
                }
            }
            free(temp);
            ptr = start-1;
        }
        /* Id comma */
        if (*ptr == ',') {
            /*Token token = create_token(TOKEN_COMMA, ",", line_number, collumn, mode);*/
            /*add_token(token_list, token);*/
            tokens[i++] = create_token(TOKEN_COMMA, ",", line_number, collumn, mode);
            ptr++;
            continue;
        }
        /* Id directives */
        if (*ptr == '.') {
            char* start = ptr;
            char* temp;
            while (!isspace(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            if (is_directive(temp)) {
                Token token = create_token(TOKEN_DIRECTIVE, temp, line_number, collumn, mode);
                /*add_token(token_list, token);*/
                tokens[i++] = token;
                continue;
            }
            free(temp);
            ptr = start;

        }
        /* This is a temporery function for identification of unknowen labels
           this needs to be changed  */
        if (isalnum(*ptr)) {
            char* start = ptr;
            char* temp;
            while (!isspace(*ptr) && (*ptr) != ',') ptr++;
            temp = strndup(start, ptr - start);
            if (1 && !is_macro(macro_list, temp)) {
                Token token;
                (mode == TBD) ? (mode = DIRECT) : (mode = mode);
                token = create_token(TOKEN_LABEL, temp, line_number, collumn, mode);
                /*add_token(token_list, token);*/
                tokens[i++] = token;
                /*add_unresolved_label(token.val, line_number, unresolved_list, file_name);*/
                continue;
            }
            free(temp);
            ptr = start;
        }
        else {
           char* start = ptr;
            char* temp;
            while (!isspace(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            *token_error_flag = 1;
            printf("panic! at line %d: %s is an undefined word\n", line_number, temp);
            /* Add a way to signal there was an error in the tokenizetion stage while build the AST */
        }

    }
    /*token = create_token(TOKEN_EOF, NULL, line, collumn, DIRECT);*/
    /*add_token(token_list, token);*/
    tokens[i] =create_token(TOKEN_EOF, NULL, line, collumn, DIRECT);
    return;
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

/* Need to add mnoore testing for labels and other reserved words */
int is_label(MacroList* macro_list, char* temp) {
    int i, len = sizeof(labels) / sizeof(labels[0]);
    if (!is_register(temp)) {
        for (i = 0; i < len; i++) {
            if (strcmp(temp, labels[i]) == 0) {
                return 0;
            }
        }
    }
    if (is_opcocde(temp) == 1) {
        return 0;
    }
    if (is_macro(macro_list, temp) == 1) {
        return 0;
    }
    return 1;
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
    if (strchr(temp, '"') == NULL) {
        return  1;
    }
    return 0;
}

Token create_token(TokenType type, char* val, int line, int collumn, addressing_mode mode) {
    Token token;
    
    token.type = type;
    token.val = val;
    token.line = line;
    token.collumn = collumn;
    token.mode = mode;

    return token;
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

void clear_token_arr(Token *tokens) {
    int i;
    for (i = 0; tokens[i].type != TOKEN_EOF; i++) {
        if (tokens[i].val != NULL && tokens[i].type != TOKEN_COMMA) {
        free(tokens[i].val); 
        }
    }
    free(tokens);  
}
    
   /* Token* curr = tokens;
    while (curr->type != TOKEN_EOF) {
        if (curr->type == TOKEN_COMMA) {
            curr++;
            continue;
        }
        free(curr->val);
        curr++;
    }
    free(tokens);*/

TokenList* create_token_list() {
    TokenList* list = malloc(sizeof(TokenList));
    if (list == NULL) {
        memory_allocation_failure();
    }
    list->head = NULL;
    return list;
}

void add_token(TokenList* token_list, Token* token) {
    if (token_list->head == NULL) {
        token_list->head = token;
    } else {
        Token* current = token_list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = token;
    }
    token->next = NULL;  
}
    
void clear_token_list(TokenList* token_list) {
    Token* curr = token_list->head;
    Token* next;
    while (curr != NULL) {
        next = curr->next;
        if (curr->type == TOKEN_COMMA) {
            free(curr);
            curr = next;
            continue;
        }
        else {
            free(curr->val);
            free(curr);
            curr = next;
        }
    }
    free(token_list);
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

void remove_collon(char* str) {
    int len = strlen(str);
    str[len-1] = '\0';
}

/*
int main(int argc, char *argv[]) {
    const char* filename;
    char line_content[MAX_LINE_LENGTH];
    Token* tokens;
    Token* current;
    FILE *file;
    int line_number = 0;
    HashTable* symbol_table = create_hash_table(INITIAL_HASH_TABLE_SIZE);
    unresolvedLabelRefList* unresolved_list = create_unresolved_label_list();

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
        tokens = tokenize(line_content, line_number, symbol_table, unresolved_list);
        
        create_instrucion_node(tokens);

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
*/