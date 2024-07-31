#include "ast.h"
#include "lexer.h"
#include <string.h>
#include "panic.h"

#define TWO_ARGS_OPCODE_LIMIT 5
#define ONE_ARG_OPCODE_LIMIT 9
#define NO_ARGS_OPCODE_LIMIT 2

char* two_args_opcode[] = {
    "mov", "cmp", "add", "sub", "lea"
};

char* one_arg_opcode[] = {
    "not", "clr", "inc", "dec", "jmp", "bne", "red", "prn", "jsr"
};

char* no_args_opcode[] = {
    "rts", "stop"
};

ASTNode* create_AST_node(Token *tokens) {
    int index;
    Token* curr_token; 
    for (index = 0; tokens[index].type != TOKEN_EOF; index++) {
        curr_token = &tokens[index];
        if (curr_token->type == TOKEN_INSTRUCTION) {
            ASTNode* instruction_node = malloc(sizeof(ASTNode));
            /* Add testing to verify the alocation */
            instruction_node = create_instrucion_node(curr_token, &index);
            
        }
        else if (curr_token->type == TOKEN_LABEL) {
            /* create_lable_node */
            if (tokens[++index].type != TOKEN_EOF) {

            }
        }
        else if (curr_token->type == TOKEN_DIRECTIVE) {
            /* create_directive node; */
        }
        else {
            /* return error */
        }
    }
}

ASTNode* create_instrucion_node(Token* token, int* index) {
    ASTNode* node = malloc(sizeof(ASTNode));
    /* Add testinf to make sure the alocation was successful */
    int arg_num, i;
    node->type = AST_INSTRUCTION;
    node->data.instruction.name = strdup(token->val); 
    
    arg_num = id_arg_count(token);
    for (i = 0; i < arg_num; i++) {
        token = fetch_next_token(token+1);
        (i == 0) ? (node->data.instruction.arg1 = strdup(token->val)) : (node->data.instruction.arg2 = strdup(token->val));
    }
    token++;
    if (token->type != TOKEN_EOF) {
                excess_code(node, token, arg_num);
            }

    return node;
}

int id_arg_count(Token* token) {
    char* opcode = token->val;
    int i;
    for (i = 0; i < TWO_ARGS_OPCODE_LIMIT; i++) {
        if (strcmp(opcode, two_args_opcode[i]) == 0) {
            return 2;
        }
    }
    for (i = 0; i < ONE_ARG_OPCODE_LIMIT; i++) {
        if (strcmp(opcode, one_arg_opcode[i]) == 0) {
            return 1;
        }
    }
    for (i = 0; i < NO_ARGS_OPCODE_LIMIT; i++) {
        if (strcmp(opcode, no_args_opcode[i]) == 0) {
            return 0;
        }
    }
    return -1;
}

Token* fetch_next_token(Token* token) {
    Token* curr = token;
    while (curr->type != TOKEN_EOF) {
        if (curr->type == TOKEN_COMMA) {
            if (++curr->type == TOKEN_COMMA) {
                printf("panic! consecutive commas\n");
                return NULL;
            }
            curr++;
            continue;
        }
        else {
            return curr;
        }
    }
    return NULL;
}



Token* excess_code(ASTNode* node, Token* token, int arg_num) {
    if (node->type == AST_INSTRUCTION) {
        printf("panic! %s instruction can only have %d arguments. Please remove the following characters:", node->data.instruction.name, arg_num);
        print_remaning_tokens(token);
    }
}

void print_remaning_tokens(Token* tokens) {
    printf("\"");
    for (; tokens->type != TOKEN_EOF; tokens++) {
        printf(" %s", tokens->val);
    }
    printf("\"\n");
}


