#include "transletor.h"
#include "ast.h"
#include "lexer.h"
#include "panic.h"
#include "symbol_table.h"
#include "word_types.h"
#include <string.h>

int line_counter = 2;
/* Creata a function that can send the node divide a into helper functions that can handke it's conetents */
void delegate_node(ASTNode* node, WordList* code_list, WordList* data_list,  Shed* shed) {
    while (node != NULL) {
        if (node->type == AST_INSTRUCTION) {
            /* Make sure all alocations were checked! */
            /* old line: */
            /*firstWord* first_word = malloc(sizeof(firstWord));*/
            Word* first_word = malloc(sizeof(Word));
            if (first_word == NULL) {
                memory_allocation_failure();
            }
            memset(first_word, 0, sizeof(Word));
            first_word->word_type = FIRST_WORD;
            translate_instruction_first(first_word, node, code_list, shed);
            node = node->next;
            printf("--------------- Line number: %d ---------------\n", line_counter++);
        }
        else if (node->type == AST_DIRECTIVE) {
            if (strcmp(node->data.directive.directive, ".entry") == 0) {
                /* Add a list to store the extern and entry calls*/
                /* Add a testing method and a (potentially) a symbol table or linked list for them to be used when createing the files  */
                node = node->next;
            }
            else if (strcmp(node->data.directive.directive, ".extern") == 0) {
                /* Add a list to store the extern and entry calls*/
                /* Add a testing method and a (potentially) a symbol table or linked list for them to be used when createing the files  */
                node = node->next;
            }
            else if (strcmp(node->data.directive.directive, ".data") == 0) {
                translate_data_directive(node, data_list, shed);
                printf("--------------- Line number: %d ---------------\n", line_counter++);

            }
            else if (strcmp(node->data.directive.directive, ".string") == 0) {
                translate_string_directive(node, data_list, shed);
                node = node->next;
                printf("--------------- Line number: %d ---------------\n", line_counter++);
            }
        }
        else if (node->type == AST_LABEL) {
            translate_labels(node, code_list, data_list, shed);
            node = node->next;
            printf("--------------- Line number: %d ---------------\n", line_counter++);
        }
    }
}

/* Creata a function that can translate an instruction */
void translate_instruction_first(Word* word ,ASTNode* node, WordList* word_list, Shed* shed) {
    int instruction_opcode;
    int source_addressing_mode = IGNORE;
    int destenation_addressing_mode = IGNORE;
    int source_value;
    int destenation_value;

    memset(word, 0, sizeof(firstWord));
    instruction_opcode = id_opcode(node->data.instruction.name);

    if (node->data.instruction.arg2 != NULL) {
        source_addressing_mode = node->data.instruction.arg1_addressing_mode;
        source_value = id_addressing_mode(node->data.instruction.arg1_addressing_mode);
        word->type.first_word.source_address = source_value;
        destenation_addressing_mode = node->data.instruction.arg2_addressing_mode;
        destenation_value = id_addressing_mode(node->data.instruction.arg2_addressing_mode);
        word->type.first_word.destenation_address = destenation_value;
    }
    else if (node->data.instruction.arg1 != NULL) {
        source_addressing_mode = node->data.instruction.arg1_addressing_mode;
        source_value = id_addressing_mode(node->data.instruction.arg1_addressing_mode);
        word->type.first_word.destenation_address = source_value;
    }

    word->type.first_word.are = 4;
    word->type.first_word.opcode = instruction_opcode;
    print_word(word);
    add_word_node(word_list, word);
    /*print_first_word(word); */
    /*rint_first_word_to_file(shed->file_name, first_word); */

    /* an instruction that has at least one operand */
    if (source_addressing_mode != IGNORE) {
        /* an instruction that has atleast two operands */
        /* First operand is a register */
        if (source_addressing_mode == INDIRECT_REGISTER || source_addressing_mode == DIRECT_REGISTER) {
               Word* register_word = malloc(sizeof(Word));
               if (register_word == NULL) {
                memory_allocation_failure();
            }
            memset(register_word, 0, sizeof(Word));
            register_word->word_type = REGISTER_WORD;
            if (destenation_addressing_mode == IGNORE) {
                translate_indirect_register(register_word, node->data.instruction.arg1, DESTENATION, word_list);
            }
            else {
                translate_indirect_register(register_word, node->data.instruction.arg1, SOURCE, word_list);
            }
            add_word_node(word_list, register_word);
            print_word(register_word);
               /* Handle the second word */
               if (destenation_addressing_mode != IGNORE) {
                /*shed->register_word = register_word;*/
                handle_second_word(node, word, source_addressing_mode, destenation_addressing_mode, word_list, shed);
            }
            return;
         }
        /* First operand is a label */


        else if (source_addressing_mode == DIRECT) {
            Word* word = malloc(sizeof(Word));
            if (word == NULL) {
                memory_allocation_failure();
           }
            memset(word, 0, sizeof(Word));
            word->word_type = DIRECT_WORD;
            translate_direct(word, node->data.label.name, *shed->ic, word_list, shed);
            /* Checking if the label was defined as an "extern" or "entry" */
            check_if_label_is_extern_or_entry(shed->directive_table, node->data.label.name); 
            if (destenation_addressing_mode != IGNORE) {
                handle_second_word(node, word, source_addressing_mode, destenation_addressing_mode, word_list, shed);
            }
            return; 
        }
        /* First operand is a number */
        else if (source_addressing_mode == IMIDIATE) {
            Word* word = malloc(sizeof(Word));
            if (word == NULL) {
                memory_allocation_failure();
            }
            memset(word, 0, sizeof(Word));
            word->word_type = IMIDIATE_WORD;
            translate_imidiate(word, node->data.directive.value, word_list);
            if (destenation_addressing_mode != IGNORE) {
                handle_second_word(node, word, source_addressing_mode, destenation_addressing_mode, word_list, shed);
            }
            return;
        }
    }    

    return;
}

void handle_second_word(ASTNode* node, Word* word, int source_addressing_mode, int destenation_addressing_mode, WordList* word_list, Shed* shed) {
    /* Second operand is a register */
    if (destenation_addressing_mode == INDIRECT_REGISTER || destenation_addressing_mode == DIRECT_REGISTER) {
        translate_indirect_register(word, node->data.instruction.arg2, DESTENATION, word_list);
        add_word_node(word_list, word);
    }
    /* IF the second operand is a label */
    else if (destenation_addressing_mode == DIRECT) {
        Word* direct_word = malloc(sizeof(Word));
        if (direct_word == NULL) {
            memory_allocation_failure();
        }
        memset(direct_word, 0, sizeof(Word));
        direct_word->word_type = DIRECT_WORD;
        translate_direct(direct_word, node->data.label.name, *shed->ic, word_list, shed);
    }
    /* Second operand is a directive */
    else if (destenation_addressing_mode == IMIDIATE) {
        Word* imidiate_word = malloc(sizeof(Word));
        if (imidiate_word == NULL) {
            memory_allocation_failure();
        }
        memset(imidiate_word, 0, sizeof(Word));
        imidiate_word->word_type = IMIDIATE_WORD;
        translate_imidiate(imidiate_word, node->data.directive.value, word_list);
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

void translate_direct(Word* word, char* label_name, int address, WordList* word_list, Shed* shed) {
    /*
    Symbol* found;
    found  = lookup_symbol(shed->symbol_table, label_name);
    if (found == NULL) {
        add_word_node(word_list, word);
        //`dd_unresolved_label(label_name, address, shed->unresolved_list, (word));
        return;
         Add a placeholder word for so tracking back is possible 
    }
    */
    memset(word, 0, sizeof(Word));
    add_word_node(word_list, word);
    add_unresolved_label(label_name, &address, word, shed->unresolved_list);
    print_word(word);
    /*word->type.direct_word.are = 4;
    word->type.direct_word.address = found->address;
*/
}

void translate_indirect_register(Word* word, char* register_name, mode mode, WordList* word_list) {
    word->word_type = REGISTER_WORD;
    if (mode == SOURCE) {
        char* ptr = register_name;
        word->type.register_word.are = 4;
        word->type.register_word.source_regiester_name = atoi((ptr+1));
    }
    else if (mode == DESTENATION) {
        char* ptr = register_name;
        word->type.register_word.are = 4;
        word->type.register_word.destination_register_name = atoi((ptr+1));
    }
    /*add_word_node(word_list, word);*/
    return;
}

void translate_imidiate(Word* word, char* value, WordList* word_list) {
    word->type.imidiate_word.are = 4;
    word->type.imidiate_word.value = atoi(value);
    add_word_node(word_list, word);
    print_word(word);
    return;
}
/* Check if this function is actually beeing used */
/*void translate_label_defenition(ASTNode* node, Shed* shed) {
    node = node->data.label.definition_node;
    if (node->type == AST_INSTRUCTION) {
        Word* first_word = malloc(sizeof(Word));
        if (first_word == NULL) {
            memory_allocation_failure();
        }

        translate_instruction_first(&first_word->type.first_word, node, shed);
    }
}
*/

void translate_string_directive (ASTNode* node, WordList* word_list, Shed* shed) { 
    char* ptr = node->data.directive.value;
    Word* directive_word;
    for (; *(ptr) != '\0'; ptr++) {
        directive_word = malloc(sizeof(Word));
        if (directive_word == NULL) {
            memory_allocation_failure();
        }
        memset(directive_word, 0, sizeof(Word));
        directive_word->word_type = DIRECTIVE_WORD;
        directive_word->type.directive_word.value = *ptr;
        (*shed->dc)++;
        add_word_node(word_list, directive_word);
        print_word(directive_word);
    }
    (*shed->dc)++;
    directive_word = malloc(sizeof(Word));
    if (directive_word == NULL) {
        memory_allocation_failure();
    }
    directive_word->word_type = DIRECTIVE_WORD;
    directive_word->type.directive_word.value = 0;
    add_word_node(word_list, directive_word);
    print_word(directive_word);
}

void translate_data_directive(ASTNode* node, WordList* word_list, Shed* shed) {
    char* ptr = node->data.directive.value;
    
    while (*ptr != '\0') {
        Word* directive_word;
        char* start;
        char value[16];  

        if (*ptr == ',') {
            ptr++;
            continue;
        }

        directive_word = malloc(sizeof(Word));
        if (directive_word == NULL) {
            memory_allocation_failure();
            return;  
        }

        memset(directive_word, 0, sizeof(Word));
        directive_word->word_type = DIRECTIVE_WORD;

        start = ptr;

        while (*ptr != ',' && *ptr != '\0') {
            ptr++;
        }

        strncpy(value, start, ptr - start);
        value[ptr - start] = '\0';  
        directive_word->type.directive_word.value = atoi(value);

        (*shed->dc)++;

        add_word_node(word_list, directive_word);
        print_word(directive_word);

        /*free(directive_word); */
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
            print_directive_word(&word->type.directive_word);
        break;
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
void translate(Shed* shed, WordList* code_list, WordList* data_list) {
    if (shed->node_list->head != NULL) {
        delegate_node(shed->node_list->head, code_list, data_list, shed);
        /*shed->node_list->head = shed->node_list->head->next; */
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
/**
 * @brief 
 * 
 * @param node 
 * @param word_list 
 * @param shed 
 *
 * TODO: Add a function that can the line count/address so it could be for the symbol table
 */
void translate_labels(ASTNode* node, WordList* code_list, WordList* data_list, Shed* shed) {
    if (lookup_symbol(shed->symbol_table, node->data.label.name) != NULL) {
        printf("panic! at line %d: %s is already defined\n", node->line, node->data.label.name);
    }
    else if (node->data.label.definition_node->type == AST_INSTRUCTION) {
        Word* word = malloc(sizeof(Word));
        if (word == NULL) {
            memory_allocation_failure();
        }
        word->word_type = FIRST_WORD;
        /* Add the address type */
        if (code_list->head == NULL) {
            insert_symbol(shed->symbol_table, node->data.label.name, 0);
        }
        else {
            insert_symbol(shed->symbol_table, node->data.label.name, code_list->tail->address);
        }
        translate_instruction_first(word, node->data.label.definition_node, code_list, shed);
    }
    else if (node->data.label.definition_node->type == AST_DIRECTIVE) {
        /* add the address type */
        if (data_list->head == NULL) {
            insert_symbol(shed->symbol_table, node->data.label.name, 0);
        }
        else {
            insert_symbol(shed->symbol_table, node->data.label.name, data_list->tail->address);
        }
        /*delegate_node(node->data.label.definition_node, shed->file_name, shed); */
        if (strcmp(node->data.label.definition_node->data.directive.directive, ".data") == 0) {
            translate_data_directive(node->data.label.definition_node, data_list, shed);
        }
        else if (strcmp(node->data.label.definition_node->data.directive.directive, ".string") == 0) {
            translate_string_directive(node->data.label.definition_node, data_list, shed);
        }
    }
}

void resolve_unresolved_list(Shed* shed) {
    unresolvedLabelRef* node = shed->unresolved_list->head;
    unresolvedLabelRef* prev = NULL;
    Symbol* found = NULL;

    while (node != NULL) {
        /* Look up the symbol in the symbol table */
        found = lookup_symbol(shed->symbol_table, node->name);

        if (found != NULL) {
            /* Resolve the placeholder word with the actual address from the symbol */
            node->word->type.direct_word.address = *found->address;
            node->word->type.direct_word.are = 2;
            /*node->word->type.direct_word.address = found->address;*/
            /*if (node->word->type.direct_word.address = found->address) {
                node->word->address = found->address;
            } else {
            
            }*/

            /* Remove the resolved node from the unresolved list*/
            if (prev == NULL) {
                shed->unresolved_list->head = node->next;
            } else {
                prev->next = node->next;
            }

            /* Dont forget to free the resolved node!! */
            unresolvedLabelRef* temp = node;
            node = node->next;
            free(temp);
        } else {
            /* Move to the next node if the symbol was not found*/
            printf("%s is an external label not defined in this file\n", node->name);
            node->word->type.direct_word.are = 2;
            prev = node;
            node = node->next;
        }
    }

    if (shed->unresolved_list->head == NULL) {
        printf("All labels resolved successfully.\n");
    } else {
        printf("Some labels could not be resolved.\n");
    }
}

WordList* create_word_list() {
    WordList* list = (WordList*)malloc(sizeof(WordList));
    if (list == NULL) {
        perror("Failed to create list");
        exit(EXIT_FAILURE);
    }
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void print_first_word_to_file(FILE *file, firstWord *word) {
    fprintf(file, "%03o %01o %01o %01o\n", word->opcode, word->source_address, word->destenation_address, word->are);
}


void add_word_node(WordList* list, Word* word) {
    WordNode* new_node = (WordNode*)malloc(sizeof(WordNode));
    char* value;
    if (new_node == NULL) {
        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    /* check if passing the word value is necissary 
    value = strdup(word_value);
    if (value == NULL) {
        memory_allocation_failure();
    }
    */
    new_node->word = word;
    /*new_node->word_value = value;*/
    new_node->next = NULL;

    if (list->head == NULL) {
        list->head = new_node;
        list->tail = new_node;
        new_node->address = 1;
    } else {
        new_node->address = list->tail->address + 1;
        list->tail->next = new_node;
        list->tail = new_node;
    }
}

void clear_word_list(WordList* list) {
    WordNode* current = list->head;
    WordNode* next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    list->head = NULL;
    list->tail = NULL;
}

void print_word_list(WordList* list) {
    WordNode* current = list->head;
    while (current != NULL) {
        print_word(current->word);
        current = current->next;
    }
}

void free_word_list(WordList* list) {
    clear_word_list(list);
    free(list);
}

void check_if_label_is_extern_or_entry(DirectiveTable* directive_table, char* label_name) {
    directiveRef* found = lookup_directive(directive_table, label_name);
    if (found != NULL) {
        if (found->directive_type == EXTERN_DIRECTIVE) {
            printf("Label: %s is defined in the file as an extern veriable. printing...\n", label_name);
        }
        else if (found->directive_type == ENTRY_DIRECTTIVE) {
            printf("Label: %s is defined in the file as an entry veriable. printing...\n", label_name);
            
        }
    }
}
