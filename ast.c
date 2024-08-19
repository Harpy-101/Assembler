#include "ast.h"
#include "lexer.h"
#include <string.h>
#include "panic.h"
#include "symbol_table.h"
/*#include "symbol_table.h"*/

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

Opcode opcode_list[] = {
    {"mov", IMIDIATE | DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER, DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER},
    {"cmp", IMIDIATE | DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER, IMIDIATE | DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER},
    {"add", IMIDIATE | DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER, DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER},
    {"sub", IMIDIATE | DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER, DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER},
    {"lea", DIRECT, DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER}, 
    {"clr", DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER}, 
    {"not", DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER}, 
    {"inc", DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER}, 
    {"dec", DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER}, 
    {"jmp", DIRECT | INDIRECT_REGISTER}, 
    {"bne", DIRECT | INDIRECT_REGISTER}, 
    {"red", DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER}, 
    {"prn", IMIDIATE | DIRECT | INDIRECT_REGISTER | DIRECT_REGISTER}, 
    {"jsr", DIRECT | INDIRECT_REGISTER}, 
    {"rts"},
    {"stop"}, 


};

ASTNode* create_AST_node(Token *tokens, ASTNodeList* node_list, DirectiveTable* directive_table) {
    int index;
    Token* curr_token; 
    /*for (index = 0; tokens[index].type != TOKEN_EOF; index++) { */
        curr_token = tokens;
        if (curr_token->type == TOKEN_INSTRUCTION) {
            ASTNode* instruction_node = malloc(sizeof(ASTNode));
            /* Add testing to verify the alocation */
            instruction_node = create_instrucion_node(curr_token);
            insert_node(node_list, instruction_node);
        }
        else if (curr_token->type == TOKEN_LABEL_DEFENITION) {
            /* If the token is a  */
            ASTNode* label_node = create_label_node(tokens, &index, directive_table);
            if (label_node == NULL) {
                return NULL;
            }
            insert_node(node_list, label_node);
        }
        else if (curr_token->type == TOKEN_LABEL) {
            ASTNode* label_node = malloc(sizeof(ASTNode));
                if (label_node == NULL) {
                    memory_allocation_failure();
                }
                label_node->type = AST_LABEL;
                label_node->data.label.name = strdup(curr_token->val);
                insert_node(node_list, label_node);
            /*Symbol* found;
            found = lookup_symbol(symbol_table, curr_token->val);
            if (found == NULL) {
                add_unresolved_label(curr_token->val, curr_token->line, unresolved_list, curr_token->origin);
            }
            else {
                ASTNode* label_node = malloc(sizeof(ASTNode));
                if (label_node == NULL) {
                    memory_allocation_failure();
                }
                label_node->type = AST_LABEL;
                label_node->data.label.name = strdup(curr_token->val);
               label_node->data.label.definition_node = found->dfinition_node;
                insert_node(node_list, label_node);
                */
        }
        else if (curr_token->type == TOKEN_DIRECTIVE) {
            ASTNode* directive_node = create_directive_node(tokens, &index, directive_table);
            if (directive_node == NULL) {
                return NULL;
            }
            insert_node(node_list, directive_node);
        }
        else {
            /* return error */
        } /* Potentially change the signiture to "void" */
        return NULL;
    }
/*} */ 

ASTNode* create_instrucion_node(Token* token) {
    ASTNode* node = malloc(sizeof(ASTNode));
    int arg_num, i;
    if (node == NULL) {
        memory_allocation_failure();
    }
    node->type = AST_INSTRUCTION;
    node->data.instruction.name = strdup(token->val); 
    node->line = token->line;
    
    arg_num = id_arg_count(token);
    for (i = 0; i < arg_num; i++) {
        token = fetch_next_token(token+1);
        /* Change this if statment to handle consecutive commas*/
        if (token == NULL) {
            return NULL;
        }
        if (token->type == TOKEN_LABEL_DEFENITION) {
            printf("Panic at line: %d: you must define a label in the start of the line. If you didn't inted to define a label please remove: \":\" from %s \n", token->line, token->val);
            return NULL;
        }
        (i == 0) ? (node->data.instruction.arg1 = strdup(token->val)) : (node->data.instruction.arg2 = strdup(token->val));
        (i == 0) ? (node->data.instruction.arg1_addressing_mode = token->mode) : (node->data.instruction.arg2_addressing_mode = token->mode);
        /* If the addressing mode isn't valid for this instruction */
    }
    /*printf("Instruction: %s, Arg1: %s, Arg1 Mode: %d\n", node->data.instruction.name,
       node->data.instruction.arg1,
       node->data.instruction.arg01_addressing_mode);*/
    check_addressing_mode(node);
    
    token++;
    if (token->type != TOKEN_EOF) {
        excess_code(node, token, arg_num);
        return NULL;
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
            if ((++curr)->type == TOKEN_COMMA) {
                printf("panic! consecutive commas\n");
                return NULL;
            }
            continue;
        }
        else {
            return curr;
        }
    }
    return NULL;
}



void excess_code(ASTNode* node, Token* token, int arg_num) {
    char* buffer = malloc(MAX_ERROR_SIZE);
    if (node->type == AST_INSTRUCTION) {
        /* snprintf(buffer, MAX_ERROR_SIZE, "%s instruction can only have %d arguments. Please remove the following characters:", node->data.instruction.name, arg_num);*/
        sprintf(buffer, "%s instruction can only have %d arguments. Please remove the following characters:", node->data.instruction.name, arg_num);
        print_remaining_tokens(buffer, token);
        node->status.type = STATUS_ERROR;
        node->status.error_code = buffer;
        node->line = token->line;
        ASTNode_error(node);
        return;
    }
    else if (node->type == AST_DIRECTIVE) {
        /* snprintf(buffer, MAX_ERROR_SIZE, "%s directive can only have %d arguments. Please remove the following characters:", node->data.directive.value, 1); */
        sprintf(buffer, "%s directive can only have %d arguments. Please remove the following characters:", node->data.directive.value, 1);
        print_remaining_tokens(buffer, token);
        node->status.type = STATUS_ERROR;
        node->status.error_code = buffer;
        node->line = token->line;
        ASTNode_error(node);
        return;
    }
    /* check if there is a need to add a case for labels */
}

void print_remaining_tokens(char* buffer, Token* tokens) {
    char* buffer_end = buffer + strlen(buffer);
    size_t buffer_size = 1024;
    size_t remaining_size = buffer_size - (buffer_end - buffer);
    
    strcat(buffer, "\"");
    for (; tokens->type != TOKEN_EOF; tokens++) {
        size_t token_length = strlen(tokens->val);
        if (remaining_size < token_length + 2) { /* +2 for space and null terminator */
            size_t used_length;
            buffer_size *= 2;
            used_length = buffer_end - buffer;
            buffer = realloc(buffer, buffer_size);
            if (!buffer) {
                memory_allocation_failure();
            }
            buffer_end = buffer + used_length;
            remaining_size = buffer_size - used_length;
        }

        strcat(buffer_end, " ");
        strcat(buffer_end, tokens->val);
        buffer_end += token_length + 1;
        remaining_size -= token_length + 1;
    }
    strcat(buffer_end, "\"\n");
}

/**
 * @brief Create a label node object
 * 
 * @param symbol_table 
 * @param unresolved_list 
 * @param token 
 * @param index 
 * @return ASTNode*
 * 
 * todo Add a handling function for excess code for all of the directive options
 */
ASTNode* create_label_node(Token* token, int* index, DirectiveTable* directive_table) {
    ASTNode* node = malloc(sizeof(ASTNode));
    /*Token* label; */
    if (node == NULL) {
        memory_allocation_failure();
    }

    /* label = token; */
    node->data.label.name = strdup(token->val);
    node->type = AST_LABEL;

    token++;
    if (token->type == TOKEN_INSTRUCTION) {
    /* Add testing in order to identify the kind of token and process it acordingly
       After doing so, link the definition node to the symbol table. */
        /* Symbol* found; */
        /*Symbol* symbol;*/
        ASTNode* definition_node = malloc(sizeof(ASTNode));
        if (definition_node == NULL) {
            memory_allocation_failure();
        }
        /* Old implementation: 
        definition_node = create_instrucion_node(token, index);
        found = lookup_symbol(symbol_table, label->val);
        found->dfinition_node = definition_node;
        node->data.label.definition_node = definition_node;
        node->line = token->line;
        node->data.label.name = strdup(token->val);
        node->type = AST_LABEL;
        return node;
        */
        /* New implementation: */
        definition_node = create_instrucion_node(token);
        /*
        insert_symbol(symbol_table, node->data.label.name, token->line);
        symbol = lookup_symbol(symbol_table, node->data.label.name);
        symbol->dfinition_node = definition_node;
        node->data.label.definition_node = symbol->dfinition_node;
        */
        node->data.label.definition_node = definition_node;
        node->line = token->line;
        node->type = AST_LABEL;
        return node;
    }
    else if (token->type == TOKEN_DIRECTIVE) {
        ASTNode* defenition_node = malloc(sizeof(ASTNode));
        defenition_node = create_directive_node(token, index, directive_table);
        node->data.label.definition_node = defenition_node;
        return node;
    }
 
    else {
        /* Return error */
        printf("panic! at line: %d: expected a label defenition after: %s\n", token->line, (token-1)->val);
        return NULL;
    }
    return NULL;
}

ASTNode* create_directive_node(Token* token, int* index, DirectiveTable* directive_table) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->data.directive.directive = strdup(token->val);
    node->type = AST_DIRECTIVE;
    if (strcmp(token->val,".data") == 0) {
        int synatx_test;
        token++;
        synatx_test = verify_comma_seperation_for_data_directive(token);
        if (synatx_test == ERROR) {
            free(node);
            return NULL;
        }
        /*token++; */
        node->data.directive.value = combine_tokens(token);
        return node; 
    }
    else if (strcmp(token->val,".string") == 0) {
        token++;
        node->data.directive.value = combine_tokens(token);
        return node;
    }
    else if (strcmp(token->val,".entry") == 0) {
       Token* temp;
       token++;
        temp = token+1;
       if (temp->type == TOKEN_EOF) {
        if (token->type == TOKEN_LABEL) {
            directiveRef* found = lookup_directive(directive_table, token->val);
            node->data.directive.value = token->val;
            if (found != NULL) {
                if (found->directive_type != ENTRY_DIRECTTIVE) {
                    printf("panic! at line %d: the label %s was already defined as \"extern\"\n", token->line, token->val);
                }
            }
            else {
                insert_directive(directive_table, node->data.directive.value, ENTRY_DIRECTTIVE);
            }
            /*Symbol* found = lookup_symbol(symbol_table, token->val);
            if (found->scope == UNDEFINED) {
                found->scope = ENTRY;   
            }
            else{
                printf("panic! at line %d: redefenition of the scope for label: %s\n", token->line, token->val);
            }*/
        }
        else {
            printf("panic! at line %d: the use of the keyword \"%s\" can only be used on labels\n", token->line, (token-1)->val);
        }
       }
    }
    else if (strcmp(token->val,".extern") == 0) {
       Token* temp;
       token++;
        temp = token+1;
       if (temp->type == TOKEN_EOF) {
        if (token->type == TOKEN_LABEL) {
            directiveRef* found = lookup_directive(directive_table, token->val);
            node->data.directive.value = token->val;
            if (found != NULL) {
                if (found->directive_type != EXTERN_DIRECTIVE) {
                    printf("panic! at line %d: the label %s was already defined as \"entry\"\n", token->line, token->val);
                }
            }
            else {
                insert_directive(directive_table, node->data.directive.value, EXTERN_DIRECTIVE);
            }
            /*Symbol* found = lookup_symbol(symbol_table, token->val);
            if (found == NULL) {
                return NULL;
            }
            else if (found->scope == UNDEFINED || found->scope == EXTERN) {
                found->scope = EXTERN;   
            }
            else{
                printf("panic! at line %d: redefenition of the scope for label: %s\n", token->line, token->val);
            }*/
        }
        else {
            printf("panic! at line %d: the use of the keyword \"%s\" can only be used on labels\n", token->line, (token-1)->val);
        }
       }
    }
    return node;
}

char* combine_tokens(Token* tokens) {
    int total_length;
    Token* curr = tokens;
    char* combined_string;
    char* ptr;
    while (curr->type != TOKEN_EOF) {
        total_length += strlen(curr->val) + 1;
        curr++;
    }
    
    combined_string = malloc(total_length + 1);
    if (combined_string == NULL) {
        memory_allocation_failure();
    }

    ptr = combined_string;
    curr = tokens;
    while (curr->type != TOKEN_EOF) {
        strcpy(ptr, curr->val);
        ptr += strlen(curr->val);
        /**ptr = ' '; */
        ptr++;
        curr++;
    }
    *ptr = '\0';
    return combined_string;
}

int verify_comma_seperation_for_data_directive(Token* token) {
    while (token->type != TOKEN_EOF) {
        if (token->type == TOKEN_NUMBER) {
            token++;
            if (token->type == TOKEN_EOF) {
                return 1;
            }
            if (token->type != TOKEN_COMMA) {
                /* Add a propper error handling function */
                printf("panic! at line: %d. data syntax isn't valid, expected a comma after: %s\n", token->line, (token-1)->val);
                return ERROR;
            }
            token++;
        }
        else {
            printf("panic! at line %d. data syntax isn't valid, expected a number after: %s\n", token->line, (token-1)->val);
            return ERROR;
        }
    }
    return 1;
}
/* Check if there is a need to handle several strings */
int verify_string_directive_syntax(Token* token) {
    while(token->type != TOKEN_EOF) {

    }
    return 1;
}

ASTNodeList* create_node_list() {
    ASTNodeList* list = malloc(sizeof(ASTNodeList));
    if (!list) {
        memory_allocation_failure();
    }
    list->head = NULL;
    return list;
}

void insert_node(ASTNodeList* list, ASTNode* node) {
    if (!list->head) {
        list->head = node;
    }
    else {
        ASTNode* temp = list->head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = node;
    }
    /*node->next = NULL; */
}

void free_node_list(ASTNodeList* list) {
    
}

void print_node_list(ASTNodeList* list) {
    ASTNode* curr = list->head;
    for (; curr != NULL; curr = curr->next) {
        switch (curr->type) {
            case AST_INSTRUCTION:
                printf("Node type: Instruction, opcode: %s, arg1: %s, arg2: %s\n", curr->data.instruction.name, curr->data.instruction.arg1, curr->data.instruction.arg2);
                break;
            case AST_DIRECTIVE:
                printf("Node type: Directive, directive: %s, argument: %s\n", curr->data.directive.directive, curr->data.directive.value);
                break;
            case AST_LABEL:
                printf("Node type: Labal, label name: %s, ", curr->data.label.name);  
                if (curr->data.label.definition_node->type == AST_INSTRUCTION) {
                    printf("Action type: Instruction, opcode: %s, arg1: %s, arg2: %s\n", curr->data.label.definition_node->data.instruction.name, curr->data.label.definition_node->data.instruction.arg1, curr->data.label.definition_node->data.instruction.arg2); 
                }
                else if (curr->data.label.definition_node->type == AST_DIRECTIVE) {
                    printf("Action type: Directive, directive: %s, argument: %s\n", curr->data.label.definition_node->data.directive.directive, curr->data.label.definition_node->data.directive.value);
                }
                else {
                    printf("Unknowen label defenition\n");
                }
                break;
        }
    }
}

int check_addressing_mode(ASTNode* node) {
    int i, len = sizeof(opcode_list) / sizeof(opcode_list[0]);
    int err = 0;
    for (i = 0; i < len; i++) {
        if (strcmp(node->data.instruction.name, opcode_list[i].opcode) == 0) {
            if (!(node->data.instruction.arg1_addressing_mode & opcode_list[i].arg1_allowed_modes) && node->data.instruction.arg1 != NULL) {
                printf("panic! at line %d: The addressing mode for the first argument: %s isn't vaild\n", node->line, node->data.instruction.arg1);
                err = 1;
            }
            if (!(node->data.instruction.arg2_addressing_mode & opcode_list[i].arg2_allowed_modes) && node->data.instruction.arg2 != NULL) {
                printf("panic! at line %d: The addressing mode for the second argument: %s isn't valid\n", node->line, node->data.instruction.arg2);
                err = 1;
            }
        }
    }
    return err;
}/*
    if (opcode_list[i].arg1_allowed_modes == 0 && node->data.instruction.arg1 != NULL) {
        printf("panic! at line %d: Unexpected argument for instruction %s\n", node->line, opcode_list[i].opcode);
        err = 1;
        continue;
    }

    if (node->data.instruction.arg2 != NULL && opcode_list[i].arg2_allowed_modes == 0) {
        printf("panic! at line %d: Unexpected second argument for instruction %s\n", node->line, opcode_list[i].opcode);
        err = 1;
        continue;
    }

    if (!(node->data.instruction.arg1_addressing_mode & opcode_list[i].arg1_allowed_modes) && node->data.instruction.arg1 != NULL) {
        printf("panic! at line %d: The addressing mode for the first argument: %s isn't valid\n", node->line, node->data.instruction.arg1);
        err = 1;
    }

    if (!(node->data.instruction.arg2_addressing_mode & opcode_list[i].arg2_allowed_modes) && node->data.instruction.arg2 != NULL) {
        printf("panic! at line %d: The addressing mode for the second argument: %s isn't valid\n", node->line, node->data.instruction.arg2);
        err = 1;
    }
}
}
*/