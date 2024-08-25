/**
 * @file ast.c
 * @author David Israel
 * @brief 
 * @version Final
 * @date 2024-08-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "ast.h"
#include "error_flags.h"
#include "lexer.h"
#include <string.h>
#include "panic.h"
#include "symbol_table.h"


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

/**
 * @brief This function takes the first token and based on its type, it refers the rest of the array to the relevant node creation function. 
 * 
 * @param tokens the token array.
 * @param node_list node list for storage.
 * @param directive_table a hash table for extern/entry calls to be used in the translation, and file creation process.
 * @return ASTNode* 
 */
ASTNode* create_AST_node(Token *tokens, ASTNodeList* node_list, DirectiveTable* directive_table) {
    int index;
    Token* curr_token; 
        curr_token = tokens;
        if (curr_token->type == TOKEN_INSTRUCTION) {
            ASTNode* instruction_node;
            instruction_node = create_instrucion_node(curr_token);
            insert_node(node_list, instruction_node);
        }
        else if (curr_token->type == TOKEN_LABEL_DEFENITION) {
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
        }
        else if (curr_token->type == TOKEN_DIRECTIVE) {
            ASTNode* directive_node = create_directive_node(tokens, &index, directive_table);
            if (directive_node == NULL) {
                return NULL;
            }
            insert_node(node_list, directive_node);
        }
        else {
            return NULL;
        } /* Potentially change the signiture to "void" */
        return NULL;
    }
/*} */ 

/**
 * @brief This function checks the syntax and builds an instruction AST node if no error was found.
 * 
 * @param token token array
 * @return ASTNode* a pointer to the newly created node
 */
ASTNode* create_instrucion_node(Token* token) {
    ASTNode* node = malloc(sizeof(ASTNode));
    int arg_num, i;
    int first_arg = 1;
    int found_comma = 0;

    if (node == NULL) {
        memory_allocation_failure();
    }

    node->type = AST_INSTRUCTION;
    node->data.instruction.name = strdup(token->val); 
    node->line = token->line;
    
    arg_num = id_arg_count(token);
    for (i = 0; i < arg_num; i++) {
        token = fetch_next_token(token + 1, &first_arg, &found_comma);
        if (token == NULL) {
            printf("\033[31mpanic!\033[0m at line %d: %s requires %d arguments\n", node->line, node->data.instruction.name, arg_num);
            ast_creation_error = 1;
            return NULL;
        }
  
        if (token->type == TOKEN_LABEL_DEFENITION) {
            printf("\033[31mpanic!\033[0m at line %d: you must define a label at the start of the line. If you didn't inted to define a label please remove: \":\" from %s \n", token->line, token->val);
            ast_creation_error = 1;
            return NULL;
        }
        (i == 0) ? (node->data.instruction.arg1 = strdup(token->val)) : (node->data.instruction.arg2 = strdup(token->val));
        (i == 0) ? (node->data.instruction.arg1_addressing_mode = token->mode) : (node->data.instruction.arg2_addressing_mode = token->mode);
    }

    check_addressing_mode(node);
    
    /* checking if their is any tokens left in the array */
    token++;
    if (token->type != TOKEN_EOF) {
        excess_code(node, token, arg_num);
        return NULL;
    }

    return node;
}

/**
 * @brief This function returns the amount of valid arguments for each known command.
 * 
 * @param token the command token
 * @return int the number of permited arguments.
 */
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

/**
 * @brief This function fetches the next token while checking for consecutive commas.
 * 
 * @param token a pointer to the current token.
 * @return Token* the token.
 */
/*Token* fetch_next_token(Token* token, int* is_first_arg) {
    Token* curr = token;
    while (curr->type != TOKEN_EOF) {
        if (curr->type == TOKEN_COMMA) {
            if (!*is_first_arg) {    
                if ((++curr)->type == TOKEN_COMMA) {
                    printf("\033[31mpanic!\033[0m at line %d: consecutive commas\n", token->line);
                    ast_creation_error = 1;
                    return NULL;
                }
            continue;
            }
        }
        else {
            return curr;
        }
    }
    return NULL;
}*/

Token* fetch_next_token(Token* token, int* is_first_arg, int* found_comma) {
    Token* curr = token;

    while (curr->type != TOKEN_EOF) {
        if (curr->type == TOKEN_COMMA) {
            if (*is_first_arg) {
                printf("\033[31mpanic!\033[0m at line %d: unexpected comma before the first argument\n", curr->line);
                ast_creation_error = 1;
                return NULL;
            } else if (*found_comma) {
                printf("\033[31mpanic!\033[0m at line %d: consecutive commas found\n", curr->line);
                ast_creation_error = 1;
                return NULL;
            }
            *found_comma = 1;  
        } else {
            if (!*is_first_arg && !*found_comma) {
                printf("\033[31mpanic!\033[0m at line %d: missing comma between arguments\n", curr->line);
                ast_creation_error = 1;
                return NULL;
            }
            *is_first_arg = 0;  
            *found_comma = 0;  
            return curr;
        }
        curr++;
    }

    return NULL;
}


/**
 * @brief This function checks if excess code is left after a valid node was created. If so, it will be printed.
 * 
 * @param node the node that was created.
 * @param token any tokens after the the valid tokens.
 * @param arg_num permited argument count.
 */
void excess_code(ASTNode* node, Token* token, int arg_num) {
    char* buffer = malloc(MAX_ERROR_SIZE);
    if (node->type == AST_INSTRUCTION) {
        sprintf(buffer, "%s instruction can only have %d arguments. Please remove the following characters:", node->data.instruction.name, arg_num);
        print_remaining_tokens(buffer, token);
        node->status.type = STATUS_ERROR;
        node->status.error_code = buffer;
        node->line = token->line;
        ASTNode_error(node);
        ast_creation_error  = 1;
        return;
    }
    else if (node->type == AST_DIRECTIVE) {
        sprintf(buffer, "%s directive can only have %d arguments. Please remove the following characters:", node->data.directive.value, 1);
        print_remaining_tokens(buffer, token);
        node->status.type = STATUS_ERROR;
        node->status.error_code = buffer;
        node->line = token->line;
        ASTNode_error(node);
        ast_creation_error = 1;
        return;
    }
}

/**
 * @brief This function prints the remaining tokens left after the valid tokens in order to show the user what he needs to remove to make his command legal.
 * 
 * @param buffer 
 * @param tokens 
 */
void print_remaining_tokens(char* buffer, Token* tokens) {
    char* buffer_end = buffer + strlen(buffer);
    size_t buffer_size = MAX_ERROR_BUFFER_SIZE;
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
    strcat(buffer_end, "\"");
}

/**
 * @brief This function identify the type of label that needs to be created and delegates the creation task to the relevant functions.
 * 
 * @param token the token array. 
 * @param index can be ignored. mainly used for testing.
 * @param directive_table 
 * @return ASTNode* a poiter to the newly created label node and it's defenition node.
 */
ASTNode* create_label_node(Token* token, int* index, DirectiveTable* directive_table) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (node == NULL) {
        memory_allocation_failure();
    }

    node->data.label.name = strdup(token->val);
    node->type = AST_LABEL;
    node->line = token->line;

    token++;
    if (token->type == TOKEN_INSTRUCTION) {
        ASTNode* definition_node;
        definition_node = create_instrucion_node(token);

        node->data.label.definition_node = definition_node;
        node->line = token->line;
        node->type = AST_LABEL;
        return node;
    }
    else if (token->type == TOKEN_DIRECTIVE) {
        ASTNode* defenition_node;
        defenition_node = create_directive_node(token, index, directive_table);
        node->data.label.definition_node = defenition_node;
        return node;
    }
 
    else {
        printf("\033[31mpanic!\033[0m at line %d: expected a label defenition after: %s\n", node->line, (token-1)->val);
        ast_creation_error = 1;
        return NULL;
    }
    return NULL;
}

/**
 * @brief This function creates a directive node. it also adds any extern/entry directive call to the directive table.
 * 
 * @param token the token array.
 * @param index can be ignored. used for testing.
 * @param directive_table 
 * @return ASTNode* a pointer to the newly created directive node.
 */
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

        node->data.directive.value = combine_tokens(token, COMBINE_DATA);
        return node; 
    }
    else if (strcmp(token->val,".string") == 0) {
        token++;
        node->data.directive.value = combine_tokens(token, COMBINE_STRING);
        return node;
    }
    else if (strcmp(token->val,".entry") == 0) {
       Token* temp;
       token++;
        temp = token+1;
       if (temp->type == TOKEN_EOF) {
        if (token->type == TOKEN_LABEL) {
            directiveRef* found = lookup_directive(directive_table, token->val);
            node->data.directive.value = strdup(token->val);
            if (node->data.directive.value == NULL) {
                memory_allocation_failure();
            }
            if (found != NULL) {
                if (found->directive_type != ENTRY_DIRECTTIVE) {
                    printf("\033[31mpanic!\033[0m at line %d: the label %s was already defined as \"extern\"\n", token->line, token->val);
                    ast_creation_error = 1;
                }
            }
            else {
                insert_directive(directive_table, node->data.directive.value, ENTRY_DIRECTTIVE);
            }
        }
        else {
            printf("\033[31mpanic!\033[0m at line %d: the use of the keyword \"%s\" can only be used with labels\n", token->line, (token-1)->val);
            ast_creation_error = 1;
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
            /*node->data.directive.value = token->val;*/
            node->data.directive.value = strdup(token->val);
            if (node->data.directive.value == NULL) {
                memory_allocation_failure();
            }
            if (found != NULL) {
                if (found->directive_type != EXTERN_DIRECTIVE) {
                    printf("\033[31mpanic!\033[0m at line %d: the label %s was already defined as \"entry\"\n", token->line, token->val);
                    ast_creation_error = 1;
                }
            }
            else {
                insert_directive(directive_table, node->data.directive.value, EXTERN_DIRECTIVE);
            }
        }
        else {
            printf("\033[31mpanic!\033[0m at line %d: the use of the keyword \"%s\" can only be used on labels\n", token->line, (token-1)->val);
            ast_creation_error = 1;
        }
       }
    }
    return node;
}

/**
 * @brief This function takes individual tokens that can be bunched up together. Numbers form a data directive call, or several strings in a string directive call.
 * 
 * @param tokens the token array.
 * @param c_type string/data
 * @return char* a pointer to the combined string.
 */
char* combine_tokens(Token* tokens, combine_type c_type) {
    int total_length = 0;
    Token* curr = tokens;
    char* combined_string;
    char* ptr;
    if (c_type == COMBINE_STRING) {
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
            ptr++;
            curr++;
        }
        *ptr = '\0';
        return combined_string;
    }
    else { 
        while (curr->type != TOKEN_EOF) {
            if (curr->type == TOKEN_NUMBER || curr->type == TOKEN_COMMA) {
                total_length += strlen(curr->val) + 1;
            }else{
                printf("Error: Invalid token encountered: %s\n", curr->val);
                ast_creation_error = 1;
                return NULL; 
            }
            curr++;
        }

        combined_string = malloc(total_length + 1); 
        if (combined_string == NULL) {
            memory_allocation_failure();
        }

        ptr = combined_string;
        curr = tokens;
        while (curr->type != TOKEN_EOF) {
            if (curr->type == TOKEN_NUMBER || curr->type == TOKEN_COMMA) {
                strcpy(ptr, curr->val);
                ptr += strlen(curr->val);
            }
            curr++;
        }

        if (ptr > combined_string && *(ptr - 1) == ' ') {
            ptr--;
        }
        *ptr = '\0';

        return combined_string;
    }
}

/**
 * @brief This function verifies the comma placement in a data directive call. 
 * 
 * @param token the token array.
 * @return int 1 for succses.
 */
int verify_comma_seperation_for_data_directive(Token* token) {
    while (token->type != TOKEN_EOF) {
        if (token->type == TOKEN_NUMBER) {
            token++;
            if (token->type == TOKEN_EOF) {
                return 1;
            }
            if (token->type != TOKEN_COMMA) { 
                printf("\033[31mpanic!\033[0m at line: %d. data syntax isn't valid, expected a comma after: %s\n", token->line, (token-1)->val);
                ast_creation_error = 1;
                return ERROR;
            }
            token++;
        }
        else {
            printf("\033[31mpanic!\033[0m at line %d. data syntax isn't valid, expected a number after: %s\n", token->line, (token-1)->val);
            ast_creation_error = 1;
            return ERROR;
        }
    }
    return 1;
}

/**
 * @brief This function creats a node list.
 * 
 * @return ASTNodeList* a pointer to the node list.
 */
ASTNodeList* create_node_list() {
    ASTNodeList* list = malloc(sizeof(ASTNodeList));
    if (!list) {
        memory_allocation_failure();
    }
    list->head = NULL;
    return list;
}

/**
 * @brief Node insertion function.
 * 
 * @param list the node list.
 * @param node the node to be inserted.
 */
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
}

/**
 * @brief This function frees the node list.
 * 
 * @param list the node list.
 */
void free_node_list(ASTNodeList* list) {
    ASTNode* curr = list->head;
    ASTNode* temp; 

    while (curr != NULL) {
        temp = curr;
        curr = curr->next;
        if (temp->type == AST_INSTRUCTION) {
           free_instruction_node(temp); 
        }
        else if (temp->type == AST_DIRECTIVE) {
            free_directive_node(temp);
        }
        else if (temp->type == AST_LABEL) {
           if (temp->data.label.definition_node == NULL) {
                free(temp->data.label.name);
                free(temp);
           }
           else if (temp->data.label.definition_node->type == AST_INSTRUCTION) {
            free_instruction_node(temp->data.label.definition_node);
            free(temp->data.label.name);
            free(temp);
           }
           else if (temp->data.label.definition_node->type == AST_DIRECTIVE) {
            free_directive_node(temp->data.label.definition_node);
            free(temp->data.label.name);
            free(temp);
           }
        }
    }
}

/**
 * @brief This function frees an instruction node,
 * 
 * @param node 
 */
void free_instruction_node(ASTNode* node) {
    free(node->data.instruction.name);
    if (node->data.instruction.arg1 != NULL) {
        free(node->data.instruction.arg1);
    }
    if (node->data.instruction.arg2 != NULL) {
        free(node->data.instruction.arg2);
    }
    free(node);
}

/**
 * @brief This function frees a directive node.
 * 
 * @param node 
 */
void free_directive_node(ASTNode* node) {
    free(node->data.directive.directive);
    if ((node->data.directive.value) != NULL) {
        free(node->data.directive.value);
    }
    free(node);
}

/**
 * @brief This function prints the node list (used for testing).
 * 
 * @param list the node list.
 */
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

/**
 * @brief This function checks if the addressing modes of of the arguments are actually permitied based on the the opcode used.
 * 
 * @param node the node being checked.
 * @return int 0 if all is well, 1 if an ilegal combenation was rquested.
 */
int check_addressing_mode(ASTNode* node) {
    int i, len = sizeof(opcode_list) / sizeof(opcode_list[0]);
    int err = 0;
    for (i = 0; i < len; i++) {
        if (strcmp(node->data.instruction.name, opcode_list[i].opcode) == 0) {
            if (!(node->data.instruction.arg1_addressing_mode & opcode_list[i].arg1_allowed_modes) && node->data.instruction.arg1 != NULL) {
                printf("\033[31mpanic!\033[0m at line %d: The addressing mode for the first argument: %s isn't vaild\n", node->line, node->data.instruction.arg1);
                ast_creation_error = 1;
                err = 1;
            }
            if (!(node->data.instruction.arg2_addressing_mode & opcode_list[i].arg2_allowed_modes) && node->data.instruction.arg2 != NULL) {
                printf("\033[31mpanic!\033[0m at line %d: The addressing mode for the second argument: %s isn't valid\n", node->line, node->data.instruction.arg2);
                ast_creation_error = 1;
                err = 1;
            }
        }
    }
    return err;
}