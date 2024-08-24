#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "preprocessor.h"
#include "symbol_table.h"
#include "error_flags.h"

#define MAX_LINE_LENGTH 256

typedef enum {
    TOKEN_LABEL, TOKEN_LABEL_DEFENITION, TOKEN_INSTRUCTION, TOKEN_REGISTER,
    TOKEN_NUMBER, TOKEN_COMMA, TOKEN_COLON,
    TOKEN_DIRECTIVE, TOKEN_STRING, TOKEN_EOF
} TokenType;

/* Still need to add more operands for labels */
typedef enum {
    IMIDIATE = 1 << 0,
    DIRECT = 1 << 1, 
    INDIRECT_REGISTER = 1 << 2, 
    DIRECT_REGISTER = 1 << 3, 
    TBD = 0, 
    IGNORE = 0
} addressing_mode;

typedef struct Token {
    TokenType type;
    char* val;
    int line;
    int collumn;
    addressing_mode mode;
    char* origin;
    struct Token* next;
} Token;

typedef struct {
    Token* head;
} TokenList;


int is_special_char(int c);


int is_opcocde(char* temp); 
Token create_token(TokenType type, char* val, int line, int collumn, addressing_mode mode);
int is_register(char* temp);
int is_label(MacroList* macro_list, char* temp);
int is_directive(char* temp); 
int is_valid_int(char* temp);
int is_valid_string(char* temp);
void tokenize(char* input, int line_number, MacroList* macro_list, Token* tokens); 
void print_token_list(Token* tokens);
void remove_collon(char* str);
void clear_token_arr(Token* tokens); 
void clear_token_list(TokenList* token_list); 
void add_token(TokenList* token_list, Token* token);
TokenList* create_token_list(); 

#endif