#ifndef AST_H
#define AST_H

#include "lexer.h"
#include "symbol_table.h" 


typedef enum {
    AST_INSTRUCTION, AST_LABEL, AST_DIRECTIVE
} AST_node_type;

typedef enum {
    STATUS_ERROR, STATUS_FIND_LABEL
} status_code;

typedef struct ASTNode {
    AST_node_type type;
    union {
        struct {
            char* name;
            char* arg1;
            char* arg2;
            addressing_mode mode;
        } instruction;
        char* label;
        struct {
            char* directive;
            char* value;
        } directive;
        char* staus;
    } data;
    char* machine_code;
    struct AST_node* next;
} ASTNode;

typedef struct {
    ASTNode* head;
} ASTNodeList;

ASTNode* create_instrucion_node(Token* token, int* index);
int id_arg_count(Token* token); 
ASTNode* create_AST_node(Token *tokens);
Token* fetch_next_token(Token* token);
Token* excess_code(ASTNode* node, Token* token, int arg_num);
void print_remaning_tokens(Token* tokens); 


#endif