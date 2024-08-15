#include "transletor.h"
#include "ast.h"
#include "lexer.h"
#include "panic.h"
#include "symbol_table.h"
#include <cstdio>

// Creata a function that can send the node divide a into helper functions that can handke it's conetents
void delegate_node(ASTNode* node, char* file_name) {
    while (node->next != NULL) {
        if (node->type == AST_INSTRUCTION) {
            /* Make sure all alocations were checked! */
            firstWord* first_word = malloc(sizeof(firstWord));
            node = node->next;
        }
        else if (node->type == AST_DIRECTIVE) {
            
        }
        else if (node->type == AST_LABEL) {
        
        }
    }
}

// Creata a function that can translate an instruction
firstWord* translate_instruction_first(firstWord* first_word ,ASTNode* node, Shed* shed) {
    int instruction_opcode;
    int source_addressing_mode = IGNORE;
    int destenation_addressing_mode = IGNORE;

    first_word = 0;
    instruction_opcode = id_opcode(node->data.instruction.name);
    if (node->data.instruction.arg1 != NULL) {
        source_addressing_mode = id_addressing_mode(node->data.instruction.arg1_addressing_mode);
        first_word->source_address = source_addressing_mode;
    }
    if (node->data.instruction.arg2 != NULL) {
        destenation_addressing_mode = id_addressing_mode(node->data.instruction.arg2_addressing_mode);
        first_word->destenation_address = destenation_addressing_mode;
    }

    first_word->are = 1;
    first_word->opcode = instruction_opcode;

    /* an instruction that has at least one operand */
    if (source_addressing_mode != IGNORE) {
        /* an instruction that has atleast two operands */
        /* First operand is a register */
        if (source_addressing_mode == INDIRECT_REGISTER || source_addressing_mode == DIRECT_REGISTER) {
               registerWord* register_word = malloc(sizeof(registerWord));
               if (register_word == NULL) {
                memory_allocation_failure();
            }
            translate_indirect_register(register_word, node->data.instruction.arg1, SOURCE);
               /* Handle the second word */
               if (destenation_addressing_mode != IGNORE) {
                handle_second_word(node, source_addressing_mode, destenation_addressing_mode, shed);
            }
         }
        /* First operand is a label */
        else if (source_addressing_mode == DIRECT) {
            Word* word = malloc(sizeof(Word));
            if (word == NULL) {
                memory_allocation_failure();
           }
            translate_direct(word, node->data.label.name, address, shed);
            if (destenation_addressing_mode != IGNORE) {
                handle_second_word(node, source_addressing_mode, destenation_addressing_mode, shed);
            }
        }
        /* First operand is a number */
        else if (source_addressing_mode == IMIDIATE) {
            Word* word = malloc(sizeof(Word));
            if (word == NULL) {
                memory_allocation_failure();
            }
            translate_imidiate(word, node->data.imidiate_value.value);
            if (destenation_addressing_mode != IGNORE) {
                handle_second_word(node, source_addressing_mode, destenation_addressing_mode, shed);
            }
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
        translate_direct(direct_word, node->data.label.name, address, shed->file_name, shed->unresolved_list);
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
    found  = lookup_symbol(symbol_table, label_name);
    if (found == NULL) {
        add_unresolved_label(label_name, address, shed->unresolved_list, shed->file_name);
    }
    
    word = 0;
    
    /* create an if statment that checks if a label was defined in the current file 
       based on that sewt the "are" portiion of the word*/
    word->are = 1;
    word->address = found->address;

}

void translate_indirect_register(registerWord* word, char* register_name, mode mode) {
    if (mode == SOURCE) {
        char* ptr = register_name;
        word->are = 1;
        word->source_regiester_name = atoi((ptr+1));
    }
    else if (mode == DESTENATION) {
        char* ptr = register_name;
        word->are = 1;
        word->destination_register_name = atoi((ptr+1));
    }
    return;
}

void translate_imidiate(imidiateWord* word, char* value) {
    word->are = 1;
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

        translate_instruction_first(first_word, node, shed);
    }
}

void translate_directive (ASTNode* node)


// Creata a function that can translate a directive

// Create a function that can translate a label

// Create a function that can send the output into the file 