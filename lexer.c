#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "error_flags.h"
#include "preprocessor.h"

/* a list of opcodes, register names and directives to be used throughout the program */
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

int collumn;

/**
 * @brief This function duplicates part of a string and returns a dynamically allocated version of it.
 *        This function is my interpretation of the original "strndup" 
 * @param str thestring. 
 * @param n limiter.
 * @return char* a pointer to the duplicated string. 
 */
char* strndup(const char* str, size_t n) {
    size_t len = strlen(str);
    size_t i;
    char* dup;
    if (n < len) {
        len = n;
    }

    dup = (char*)malloc(len + 1); 
    if (dup == NULL) {
        return NULL; 
    }

    for (i = 0; i < len; ++i) {
        dup[i] = str[i];
    }
    dup[len] = '\0'; 

    return dup;
}
/**
 * @brief This function takes a code line and divides it into identifiable tokens.
 * 
 * @param input the code line.
 * @param line_number for error handling.
 * @param macro_list for testing if a label shares the same name as a pre-defined macro.
 * @param tokens an array of tokens for storage 
 * 
 */
void tokenize(char* input, int line_number, MacroList* macro_list, Token* tokens) {
    int i = 0;
    char* ptr = input;
  
    addressing_mode mode = TBD;

    collumn = 0;
    while(*ptr) {
        collumn++;
        mode = TBD; 
        
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
                    tokens[i++] = token;
                    ptr++;
                    continue; 
                }
                printf("\033[31mpanic!\033[0m at line %d: a label can't be the same name as an opcode, register or a macro\n", line_number);
                token_creation_error = 1;
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
                    tokens[i++] = token;
                    ptr++;
                    continue;
                }
                free(temp);
            }
            ptr = start-1;
        }
        /* Id comma */
        if (*ptr == ',') {
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
                tokens[i++] = token;
                continue;
            }
            free(temp);
            ptr = start;

        }
        /* id a potential label call */
        if (isalnum(*ptr)) {
            char* start = ptr;
            char* temp;
            while (!isspace(*ptr) && (*ptr) != ',') ptr++;
            temp = strndup(start, ptr - start);
            if (1 && is_label(macro_list, temp) && mode == TBD) {
                Token token;
                (mode == TBD) ? (mode = DIRECT) : (mode = mode);
                token = create_token(TOKEN_LABEL, temp, line_number, collumn, mode);
                tokens[i++] = token;
                continue;
            }
            free(temp);
            ptr = start;
        }
        /* undefined word */
        if (1) {
           char* start = ptr;
            char* temp;
            while (!isspace(*ptr)) ptr++;
            temp = strndup(start, ptr - start);
            token_creation_error = 1;
            printf("\033[31mpanic!\033[0m at line %d: %s is an undefined word and/or uses the wrong addressing mode \n", line_number, temp);
            token_creation_error = 1;
        }

    }
    tokens[i] =create_token(TOKEN_EOF, NULL, line_number, collumn, DIRECT);
    return;
}

/**
 * @brief This function compares a potential opcode to all known opcodes in order to find a match.
 * 
 * @param temp the potential opcode.
 * @return int 1 if a match was found, 0 if it waasn't.
 */
int is_opcocde(char* temp) {
    int i, len = sizeof(opcode) / sizeof(opcode[0]);
    for (i = 0; i < len; i++) { 
        if (strcmp(temp, opcode[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief This function compares a potential register to all known registers in order to find a match.
 * 
 * @param temp the potential register. 
 * @return int 1 if a match was found, 0 if it waasn't.
 */
int is_register(char* temp) {
    int i, len = sizeof(register_names) / sizeof(register_names[0]);
    for (i = 0; i < len; i++) {
        if (strcmp(temp, register_names[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief This function compares a potential label to all known registers, opcodes and macros in order to find a match.
 * 
 * @param macro_list 
 * @param temp the potential label.
 * @return int 0 if a match was found, rendering the label unusable, 1 if its use is possible. 
 */
int is_label(MacroList* macro_list, char* temp) {
    if (is_register(temp) == 1) {
        return 0;
    }
    if (is_opcocde(temp) == 1) {
        return 0;
    }
    if (is_macro(macro_list, temp) == 1) {
        return 0;
    }
    return 1;
}

/**
 * @brief  This function compares a potential directive to all known directives in order to find a match.
 * 
 * @param temp the potential directive.
 * @return int 1 if a match was found, 0 if it wasn't.
 */
int is_directive(char* temp) {
    int i, len = sizeof(directive_list) / sizeof(directive_list[0]);
    for (i = 0; i < len; i++) {
        if (strcmp(temp, directive_list[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief This function checks if the number entred is valid
 * 
 * @param temp the number
 * @return int 1 if the number is valid, 0 if not.
 */
int is_valid_int(char* temp) {
    int val = atoi(temp);
    if (val > MAX_INT || val < MIN_INT) {
        return 0;
    }
    if (strstr(temp, ".") == NULL) {
        return 1;
    }
    
    return 0;
}

/**
 * @brief This function checks if a given string is valid.
 * 
 * @param temp the string
 * @return int 
 */
int is_valid_string(char* temp) {
    if (strchr(temp, '"') == NULL) {
        return  1;
    }
    return 0;
}

/**
 * @brief Create a token objectThis function creates a token 
 * 
 * @param type token type
 * @param val token value
 * @param line 
 * @param collumn 
 * @param mode addresing mode
 * @return Token 
 */
Token create_token(TokenType type, char* val, int line, int collumn, addressing_mode mode) {
    Token token;
    
    token.type = type;
    token.val = val;
    token.line = line;
    token.collumn = collumn;
    token.mode = mode;

    return token;
}

/**
 * @brief This was an old function for testing 
 * 
 * @param tokens 
 */
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

/**
 * @brief This function clears the tokens array
 * 
 * @param tokens 
 */
void clear_token_arr(Token *tokens) {
    int i;
    for (i = 0; tokens[i].type != TOKEN_EOF; i++) {
        if (tokens[i].val != NULL && tokens[i].type != TOKEN_COMMA) {
        free(tokens[i].val); 
        }
    }
    free(tokens);  
}

/**
 * @brief This function is used to remove the collon for label testing
 * 
 * @param str 
 */
void remove_collon(char* str) {
    int len = strlen(str);
    str[len-1] = '\0';
}

