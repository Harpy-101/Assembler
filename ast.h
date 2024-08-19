#ifndef AST_H
#define AST_H

#include <stdio.h>
#include "lexer.h"
#include "symbol_table.h"
/*#include "symbol_table.h" */

#define MAX_ERROR_SIZE 1024
#define ERROR 0

typedef enum {
    AST_INSTRUCTION, AST_LABEL, AST_DIRECTIVE
} AST_node_type;

typedef enum {
    STATUS_ERROR, STATUS_FIND_LABEL, STATUS_OK
} status_code;

typedef struct ASTNode {
    AST_node_type type;
    union {
        struct {
            char* name;
            char* arg1;
            char* arg2;
            TokenType arg1_type;
            TokenType arg2_type;
            addressing_mode arg1_addressing_mode;
            addressing_mode arg2_addressing_mode;
        } instruction;
        struct {
            char* name;
            struct ASTNode* definition_node;
        } label;
        struct {
            char* directive;
            char* value;
        } directive;
        struct { 
            char* value;
        } imidiate_value;
    } data;
    struct {
        status_code type;
        char* error_code;
    } status;
    int line;
    char* machine_code;
    struct ASTNode* next;
} ASTNode;

typedef struct {
    ASTNode* head;
} ASTNodeList;

typedef struct Opcode{
    char* opcode;
    addressing_mode arg1_allowed_modes;
    addressing_mode arg2_allowed_modes;
} Opcode;

extern int panic;

ASTNode* create_instrucion_node(Token* token);
int id_arg_count(Token* token); 
ASTNode* create_AST_node(Token *tokens, ASTNodeList* node_list, DirectiveTable* directive_table);
Token* fetch_next_token(Token* token);
void excess_code(ASTNode* node, Token* token, int arg_num);
void print_remaining_tokens(char* buffer, Token* tokens);
/*ASTNode* create_label_node(HashTable* symbol_table, unresolvedLabelRefList* unresolved_list, Token* token, int* index); */
int verify_comma_seperation_for_data_directive(Token* token); 
char* combine_tokens(Token* tokens); 
ASTNode* create_label_node(Token* token, int* index, DirectiveTable* directive_table); 
ASTNode* create_directive_node(Token* token, int* index, DirectiveTable* directive_table);
ASTNodeList* create_node_list();
void insert_node(ASTNodeList* list, ASTNode* node); 
void print_node_list(ASTNodeList* list);
int check_addressing_mode(ASTNode* node);

#endif