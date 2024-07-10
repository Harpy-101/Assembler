#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOKEN_LABEL, TOKEN_INSTRUCTION, TOKEN_REGISTER,
    TOKEN_NUMBER, TOKEN_COMMA, TOKEN_COLON,
    TOKEN_DIRECTIVE, TOKEN_STRING, TOKEN_EOF
} TokenType;

typedef enum {
    TOKEN_MOV, TOKEN_CMP, TOKEN_ADD, TOKEN_SUB,
    TOKEN_LEA, TOKEN_CLR, TOKEN_NOT, TOKEN_INC, 
    TOKEN_DEC, TOKEN_JMP, TOKEN_BNE, TOKEN_RED, 
    TOKEN_PRN, TOKEN_JSR, TOKEN_RTS, TOKEN_STOP
} directiveType;

typedef struct {
    TokenType type;
    char* val;
} Token;

char *commands[16] = {"mov", "cmp", "add", "sub",
                  "lea", "clr", "not", "inc", 
                  "dec", "jmp", "bne", "red",
                  "prn", "jsr", "rts", "stop"};

char* strdup(const char* src) {
    /*Allocate memory for the duplicate string*/
    char* dup = (char*)malloc(strlen(src) + 1); /*. +1 for the null terminator*/
    if (dup == NULL) {
        return NULL; /*Allocation failed*/
    }

    /*Copy the string */
    char* ptr = dup;
    while (*src) {
        *ptr++ = *src++;
    }
    *ptr = '\0'; /* Null-terminate the duplicated string*/

    return dup;
}

/* Manual implementation of strndup */
char* strndup(const char* src, size_t n) {
    size_t len = strlen(src);
    size_t i;
    if (n < len) {
        len = n;
    }

    char* dup = (char*)malloc(len + 1); /* Allocate memory for the new string (+1 for the null terminator) */
    if (dup == NULL) {
        return NULL; /* Allocation failed */
    }

    /* Copy up to n characters from src to dup */
    for (i = 0; i < len; ++i) {
        dup[i] = src[i];
    }
    dup[len] = '\0'; /* Null-terminate the duplicated string */

    return dup;
}

Token* create_token(TokenType tType, char *val) {
    Token *token = (Token*)malloc(sizeof(Token));
    token->type = tType;
    token->val = strdup(val);;
    return token;
}

Token* tokenize(const char* sourceCode) {
    Token* tokens = (Token*)malloc(sizeof(Token) * 256); /* Change the size */
    int tokenCount = 0;

    const char* ptr = sourceCode;
    while (*ptr) {
        if (isspace(*ptr)) {
            ptr++;
            continue;
        }

        if (isalpha(*ptr)) {
            const char* start = ptr;
            while (isalnum(*ptr) || *ptr == '_') ptr++;
            char* value = strndup(start, ptr - start);
            tokens[tokenCount++] = *create_token(TOKEN_LABEL, value); /* Change the token type */
            free(value);
            continue;
        }

        if (isalnum(*ptr)) {
            const char* start = ptr;
            while (isalnum(*ptr)) ptr++;
            char* value = strndup(start, ptr - start);
            tokens[tokenCount++] = *create_token(TOKEN_NUMBER, value);
            free(value);
            continue;
        }
        /* Add the other cases (numbers, instructions, commas, etc.)*/

        ptr++;
    }

    tokens[tokenCount++] = *create_token(TOKEN_EOF, "");
    return tokens;
}

void print_tokens(Token* tokens) {
    int i = 0;
    while (tokens[i].type != TOKEN_EOF) {
        printf("Token type: %d\n", tokens[i].type);
        printf("Token value: %s\n", tokens[i++].val);
    }
}

/*
int id_func(char **command_list, char *input, char **marker) {
    int i;
    for (i = 0; i < 17; i++) {
        char *res = strstr(input, command_list[i]);
        if (res != NULL) {
            int name_len = strlen(command_list[i]);
            if (res == input || isspace((unsigned char)*(res - 1))) {
                if (*(res + name_len) == '\0' || isspace((unsigned char)*(res + name_len))) {
                    create_token(i, res+name_len);
                    //marker = res + name_len;
                    return i;
                }
                else {
                    return -2;
                }
            }
        }
    }
    return -1;
}
*/
int main() {
    const char *code = "add *r6,K\n"
                 "sub r1, r4\n"; 

    Token* token_list = tokenize(code);
    print_tokens(token_list);

    return 0;
}
