#include "transletor.h"
#include "ast.h"
#include "lexer.h"
#include "panic.h"
#include "symbol_table.h"
#include <string.h>

Word* word_arr[3] = {0,0,0};

/* Creata a function that can send the node divide a into helper functions that can handke it's conetents */
void delegate_node(ASTNode* node, char* file_name, Shed* shed) {
    while (node->next != NULL) {
        if (node->type == AST_INSTRUCTION) {
            /* Make sure all alocations were checked! */
            firstWord* first_word = malloc(sizeof(firstWord));
            translate_instruction_first(first_word, node, shed);
            node = node->next;
        }
        else if (node->type == AST_DIRECTIVE) {
            if (strcmp(node->data.directive.directive, ".entry") == 0) {
                return;
            }
            else if (strcmp(node->data.directive.directive, ".extern") == 0) {
                return;
            }
            else if (strcmp(node->data.directive.directive, ".data") == 0) {
                translate_data_directive(node, shed);
            }
            else if (strcmp(node->data.directive.directive, ".string") == 0) {
                translate_string_directive(node, shed);
                node = node->next;
            }
        }
        else if (node->type == AST_LABEL) {
        
        }
    }
}

/* Creata a function that can translate an instruction */
firstWord* translate_instruction_first(firstWord* first_word ,ASTNode* node, Shed* shed) {
    int instruction_opcode;
    int source_addressing_mode = IGNORE;
    int destenation_addressing_mode = IGNORE;
    int source_value;
    int destenation_value;

    memset(first_word, 0, sizeof(firstWord));
    instruction_opcode = id_opcode(node->data.instruction.name);

    if (node->data.instruction.arg2 != NULL) {
        source_addressing_mode = node->data.instruction.arg1_addressing_mode;
        source_value = id_addressing_mode(node->data.instruction.arg1_addressing_mode);
        first_word->source_address = source_value;
        destenation_addressing_mode = node->data.instruction.arg2_addressing_mode;
        destenation_value = id_addressing_mode(node->data.instruction.arg2_addressing_mode);
        first_word->destenation_address = destenation_value;
    }
    else if (node->data.instruction.arg1 != NULL) {
        source_addressing_mode = node->data.instruction.arg1_addressing_mode;
        source_value = id_addressing_mode(node->data.instruction.arg1_addressing_mode);
        first_word->destenation_address = source_value;
    }

    first_word->are = 4;
    first_word->opcode = instruction_opcode;

    print_first_word(first_word);

    /* an instruction that has at least one operand */
    if (source_addressing_mode != IGNORE) {
        /* an instruction that has atleast two operands */
        /* First operand is a register */
        if (source_addressing_mode == INDIRECT_REGISTER || source_addressing_mode == DIRECT_REGISTER) {
               registerWord* register_word = malloc(sizeof(registerWord));
               if (register_word == NULL) {
                memory_allocation_failure();
            }
            if (destenation_addressing_mode == IGNORE) {
                translate_indirect_register(register_word, node->data.instruction.arg1, DESTENATION);
            }
            else {
                translate_indirect_register(register_word, node->data.instruction.arg1, SOURCE);
            }
               /* Handle the second word */
               if (destenation_addressing_mode != IGNORE) {
                shed->register_word = register_word;
                handle_second_word(node, source_addressing_mode, destenation_addressing_mode, shed);
            }
            print_register_word(register_word);
            return first_word;
         }
        /* First operand is a label */


        else if (source_addressing_mode == DIRECT) {
            Word* word = malloc(sizeof(Word));
            if (word == NULL) {
                memory_allocation_failure();
           }
            translate_direct(&word->type.direct_word, node->data.label.name, *shed->ic, shed);
            if (destenation_addressing_mode != IGNORE) {
                handle_second_word(node, source_addressing_mode, destenation_addressing_mode, shed);
            }
            return first_word; 
        }
        /* First operand is a number */
        else if (source_addressing_mode == IMIDIATE) {
            Word* word = malloc(sizeof(Word));
            if (word == NULL) {
                memory_allocation_failure();
            }
            translate_imidiate(&word->type.imidiate_word, node->data.imidiate_value.value);
            if (destenation_addressing_mode != IGNORE) {
                handle_second_word(node, source_addressing_mode, destenation_addressing_mode, shed);
            }
            return first_word;
        }
    }    

    return first_word;
}

void handle_second_word(ASTNode* node, int source_addressing_mode, int destenation_addressing_mode, Shed* shed) {
    /* Second operand is a register */
    if (destenation_addressing_mode == INDIRECT_REGISTER || destenation_addressing_mode == DIRECT_REGISTER) {
        translate_indirect_register(shed->register_word, node->data.instruction.arg2, DESTENATION);
    }
    /* IF the second operand is a label */
    if (destenation_addressing_mode == DIRECT) {
        directWord* direct_word = malloc(sizeof(directWord));
        if (direct_word == NULL) {
            memory_allocation_failure();
        }
        translate_direct(direct_word, node->data.label.name, *shed->ic, shed);
    }
    /* Second operand is a directive */
    if (destenation_addressing_mode == IMIDIATE) {
        imidiateWord* imidiate_word = malloc(sizeof(imidiateWord));
        if (imidiate_word == NULL) {
            memory_allocation_failure();
        }
        imidiate_word = 0;
        translate_imidiate(imidiate_word, node->data.directive.value);
    }
} 

int id_opcode(char* instruction_name) {
    if (strcmp(instruction_name, "mov") == 0) return 0;
    if (strcmp(instruction_name, "cmp") == 0) return 1;
    if (strcmp(instruction_name, "add") == 0) return 2;
    if (strcmp(instruction_name, "sub") == 0) return 3;
    if (strcmp(instruction_name, "lea") == 0) return 4;
    if (strcmp(instruction_name, "clr") == 0) return 5;
    if (strcmp(instruction_name, "not") == 0) return 6;
    if (strcmp(instruction_name, "inc") == 0) return 7;
    if (strcmp(instruction_name, "dec") == 0) return 8;
    if (strcmp(instruction_name, "jmp") == 0) return 9;
    if (strcmp(instruction_name, "bne") == 0) return 10;
    if (strcmp(instruction_name, "red") == 0) return 11;
    if (strcmp(instruction_name, "prn") == 0) return 12;
    if (strcmp(instruction_name, "jsr") == 0) return 13;
    if (strcmp(instruction_name, "rts") == 0) return 14;
    if (strcmp(instruction_name, "stop") == 0) return 15;
    return -1;
}

int id_addressing_mode(int addressing_mode) {
    if (addressing_mode == IMIDIATE) {
        return 1;
    }
    else if (addressing_mode == DIRECT) {
        return 2;
    }
    else if (addressing_mode == INDIRECT_REGISTER) {
        return 4;
    }
    else if (addressing_mode == DIRECT_REGISTER) {
        return 8;
    }
    return -1;
}

void translate_direct(directWord* word, char* label_name, int address, Shed* shed) {
    Symbol* found;
    found  = lookup_symbol(shed->symbol_table, label_name);
    if (found == NULL) {
        add_unresolved_label(label_name, address, shed->unresolved_list, shed->file_name);
    }
    
    word = 0;
    
    /* create an if statment that checks if a label was defined in the current file 
       based on that sewt the "are" portiion of the word*/
    word->are = 4;
    word->address = found->address;

}

void translate_indirect_register(registerWord* word, char* register_name, mode mode) {
    if (mode == SOURCE) {
        char* ptr = register_name;
        word->are = 4;
        word->source_regiester_name = atoi((ptr+1));
    }
    else if (mode == DESTENATION) {
        char* ptr = register_name;
        word->are = 4;
        word->destination_register_name = atoi((ptr+1));
    }
    return;
}

void translate_imidiate(imidiateWord* word, char* value) {
    word->are = 4;
    word->value = atoi(value);
    return;
}

void translate_label_defenition(ASTNode* node, Shed* shed) {
    node = node->data.label.definition_node;
    if (node->type == AST_INSTRUCTION) {
        Word* first_word = malloc(sizeof(Word));
        if (first_word == NULL) {
            memory_allocation_failure();
        }

        translate_instruction_first(&first_word->type.first_word, node, shed);
    }
}

void translate_string_directive (ASTNode* node, Shed* shed) { 
    char* ptr = node->data.directive.value;
    directiveWord* directive_word;
    for (; *(ptr) != '\0'; ptr++) {
        directive_word = malloc(sizeof(directWord));
        if (directive_word == NULL) {
            memory_allocation_failure();
        }
        memset(directive_word, 0, sizeof(directiveWord));
        directive_word->value = *ptr;
        (*shed->dc)++;
        print_directive_word(directive_word);
    }
    (*shed->dc)++;
    directive_word->value = 0;
    print_directive_word(directive_word);
}

void translate_data_directive(ASTNode* node, Shed* shed) {
    char* ptr = node->data.directive.value;
    for (; *(ptr) != '\0'; ptr++) {
        directiveWord* directive_word;
        if (*ptr == ',') continue;
        directive_word = malloc(sizeof(directWord));
        if (directive_word == NULL) {
            memory_allocation_failure();
        }
        memset(directive_word, 0, sizeof(directiveWord));
        /* remove atoi */
        directive_word->value = atoi(ptr);
        (*shed->dc)++;
        print_directive_word(directive_word);
    }
}

void print_word(Word* word) {
    switch (word->word_type) {
        case FIRST_WORD: 
            print_first_word(&word->type.first_word);
        break;
        case DIRECT_WORD:
            print_direct_word(&word->type.direct_word);
        break;
        case IMIDIATE_WORD:
            print_imidiate_word(&word->type.imidiate_word);
        break;
        case REGISTER_WORD:
            print_register_word(&word->type.register_word);
        break;
        case DIRECTIVE_WORD:

    }
}
/*
void print_first_word(firstWord* word) {
    printf("Word: %u%u%u%u\n", word->opcode, word->source_address, word->destenation_address, word->are);
}

void print_direct_word(directWord* word) {
    printf("Word: %u%u\n", word->address, word->are);
} 

void print_imidiate_word(imidiateWord* word) {
    printf("Word: %u%u\n", word->value, word->are);
}

void print_register_word(registerWord* word) {
    printf("Word: %u%u%u%u\n", word->padding, word->source_regiester_name, word->destination_register_name, word->are);
}
*/
void translate(Shed* shed) {
    while (shed->node_list->head != NULL) {
        delegate_node(shed->node_list->head, shed->file_name, shed);
        shed->node_list->head = shed->node_list->head->next;
    }
}

void print_first_word(firstWord* word) {
    int i;
    printf("Word: ");
    for (i = 3; i >= 0; --i) { 
        printf("%u", (word->opcode >> i) & 1);
    }
    for (i = 3; i >= 0; --i) { 
        printf("%u", (word->source_address >> i) & 1);
    }
    for (i = 3; i >= 0; --i) { 
        printf("%u", (word->destenation_address >> i) & 1);
    }
    for (i = 2; i >= 0; --i) { 
        printf("%u", (word->are >> i) & 1);
    }
    printf("\n");
}

void print_direct_word(directWord* word) {
    int i;
    printf("Word: ");
    for (i = 11; i >= 0; --i) { 
        printf("%u", (word->address >> i) & 1);
    }
    for (i = 2; i >= 0; --i) {
        printf("%u", (word->are >> i) & 1);
    }
    printf("\n");
} 

void print_imidiate_word(imidiateWord* word) {
    int i;
    printf("Word: ");
    for (i = 11; i >= 0; --i) { 
        printf("%u", (word->value >> i) & 1);
    }
    for (i= 2; i >= 0; --i) { 
        printf("%u", (word->are >> i) & 1);
    }
    printf("\n");
}

void print_register_word(registerWord* word) {
    int i;
    printf("Word: ");
    for (i = 5; i >= 0; --i) {
        printf("%u", (word->padding >> i) & 1);
    }
    for (i = 2; i >= 0; --i) { 
        printf("%u", (word->source_regiester_name >> i) & 1);
    }
    for (i = 2; i >= 0; --i) { 
        printf("%u", (word->destination_register_name >> i) & 1);
    }
    for (i = 2; i >= 0; --i) { 
        printf("%u", (word->are >> i) & 1);
    }
    printf("\n");
}

void print_directive_word(directiveWord* word) {
    int i;
    printf("Word: ");
    for (i = 14; i >= 0; --i) {
        printf("%u", (word->value >> i) & 1);
    }
    printf("\n");
}
/* Creata a function that can translate a directive */
 
/* Create a function that can translate a label */

/* Create a function that can send the output into the file  */