#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "symbol_table.h"

#define MAX_LINE_LENGTH 256

typedef enum {
    TOKEN_LABEL, TOKEN_INSTRUCTION, TOKEN_REGISTER,
    TOKEN_NUMBER, TOKEN_COMMA, TOKEN_COLON,
    TOKEN_DIRECTIVE, TOKEN_STRING, TOKEN_EOF
} TokenType;

/* Still need to add more operands for labels */
typedef enum {
    IMIDIATE, INDIRECT, DIRECT
} addressing_mode;

typedef struct {
    TokenType type;
    char* val;
    int line;
    int collumn;
    addressing_mode mode;
} Token;


int is_special_char(int c);


int is_opcocde(char* temp); 
Token create_token(TokenType type, char* val, int line, int collumn, addressing_mode mode);
int is_register(char* temp);
int is_label(char* temp);
int is_directive(char* temp); 
int is_valid_int(char* temp);
int is_valid_string(char* temp);
Token* tokenize(char* input, int line_number, HashTable* symbol_table, unresolvedLabelRefList* unresolved_list ); 
void print_token_list(Token* tokens);
void remove_collon(char* str); 

#endif