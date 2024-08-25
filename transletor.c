/**
 * @file transletor.c
 * @author David Israel
 * @brief 
 * @version Final
 * @date 2024-08-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "transletor.h"
#include "ast.h"
#include "error_flags.h"
#include "lexer.h"
#include "panic.h"
#include "symbol_table.h"
#include "word_types.h"
#include <string.h>


/**
 * @brief This function takes an ASTNode and sends it to the correct node processing function based on its node type.
 * 
 * @param node the node.
 * @param code_list the the "IC" list.
 * @param data_list the "DC" list
 * @param shed a general struct to make the function signiture a bit less cluttered. 
 */
void delegate_node(ASTNode* node, WordList* code_list, WordList* data_list,  Shed* shed) {
    while (node->next != NULL) {
        if (node->type == AST_INSTRUCTION) { 
            Word* first_word = malloc(sizeof(Word));
            if (first_word == NULL) {
                memory_allocation_failure();
            }
            memset(first_word, 0, sizeof(Word));
            first_word->word_type = FIRST_WORD;
            translate_instruction_first(first_word, node, code_list, shed);
            node = node->next;
        }
        else if (node->type == AST_DIRECTIVE) {
            if (strcmp(node->data.directive.directive, ".entry") == 0) {
                node = node->next;
            }
            else if (strcmp(node->data.directive.directive, ".extern") == 0) {
                node = node->next;
            }
            else if (strcmp(node->data.directive.directive, ".data") == 0) {
                translate_data_directive(node, data_list, shed);
                node = node->next;

            }
            else if (strcmp(node->data.directive.directive, ".string") == 0) {
                translate_string_directive(node, data_list, shed);
                node = node->next;
            }
        }
        else if (node->type == AST_LABEL) {
            translate_labels(node, code_list, data_list, shed);
            node = node->next;
        }
    }
}

/**
 * @brief This function takes an instruction node and translates it and its arguments to binary code through the help of other functions.
 * 
 * @param word to store the first word.
 * @param node the node
 * @param word_list to keep track of the address.
 * @param shed a general struct to make the function signiture a bit less cluttered. 
 * @return WordNode* depending on the context, this can be ignored or taken into account based on who called the function.
 */
WordNode* translate_instruction_first(Word* word ,ASTNode* node, WordList* word_list, Shed* shed) {
    int instruction_opcode;
    int source_addressing_mode = IGNORE;
    int destenation_addressing_mode = IGNORE;
    int source_value;
    int destenation_value;
    WordNode* first_word_node;

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

    add_word_node(word_list, word);
    first_word_node = word_list->tail;

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
         
            /* Handle the second word */
            if (destenation_addressing_mode != IGNORE) {
                if (destenation_addressing_mode == DIRECT_REGISTER || destenation_addressing_mode == INDIRECT_REGISTER) {
                    handle_second_word(node, register_word, source_addressing_mode, destenation_addressing_mode, word_list, shed);
                }
                else {
                    add_word_node(word_list, register_word);
                    handle_second_word(node, register_word, source_addressing_mode, destenation_addressing_mode, word_list, shed); 
                }
                return first_word_node;
            }
            add_word_node(word_list, register_word);
            return first_word_node;
         }

        /* First operand is a label */
        else if (source_addressing_mode == DIRECT) {
            Word* word = malloc(sizeof(Word));
            if (word == NULL) {
                memory_allocation_failure();
           }
            memset(word, 0, sizeof(Word));
            word->word_type = DIRECT_WORD;
            translate_direct(word, node->data.instruction.arg1, word_list, shed);
            
            if (destenation_addressing_mode != IGNORE) {
                if (destenation_addressing_mode == INDIRECT_REGISTER || destenation_addressing_mode == DIRECT_REGISTER) {
                    Word* register_word = malloc(sizeof(Word));
                    memset(register_word, 0, sizeof(Word));
                    handle_second_word(node, register_word, source_addressing_mode, destenation_addressing_mode, word_list, shed);
                }
                else {
                    handle_second_word(node, word, source_addressing_mode, destenation_addressing_mode, word_list, shed);
                }
            }
            return first_word_node; 
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
            return first_word_node;
        }
    }    

    return first_word_node;
}

/**
 * @brief This function handles the creation of a second word (if one is needed) based on it's type.
 * 
 * @param node the original node.
 * @param word the word to to be used for storage.
 * @param source_addressing_mode 
 * @param destenation_addressing_mode 
 * @param word_list 
 * @param shed a general struct to make the function signiture a bit less cluttered. 
 */
void handle_second_word(ASTNode* node, Word* word, int source_addressing_mode, int destenation_addressing_mode, WordList* word_list, Shed* shed) {
    /* Second operand is a register */
    if (destenation_addressing_mode == INDIRECT_REGISTER || destenation_addressing_mode == DIRECT_REGISTER) {
        word->word_type = REGISTER_WORD;
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
        translate_direct(direct_word, node->data.instruction.arg2, word_list, shed);
    }
    /* Second operand is a number */
    else if (destenation_addressing_mode == IMIDIATE) {
        Word* imidiate_word = malloc(sizeof(Word));
        if (imidiate_word == NULL) {
            memory_allocation_failure();
        }
        memset(imidiate_word, 0, sizeof(Word));
        imidiate_word->word_type = IMIDIATE_WORD;
        translate_imidiate(imidiate_word, node->data.instruction.arg2, word_list);
    }
} 

/**
 * @brief This function is used to identiy the opcode.
 * 
 * @param instruction_name 
 * @return int 
 */
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

/**
 * @brief This function is used to identify the addressing mode.
 * 
 * @param addressing_mode 
 * @return int 
 */
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

/**
 * @brief This function addsa label call to the list of unresolved labels to be resolved later.
 * 
 * @param word 
 * @param label_name 
 * @param word_list 
 * @param shed 
 */
void translate_direct(Word* word, char* label_name, WordList* word_list, Shed* shed) {
    memset(word, 0, sizeof(Word));
    add_word_node(word_list, word);
    add_unresolved_label(label_name, &word_list->tail->address, word, shed->unresolved_list);
    check_if_label_is_extern_or_entry(shed->directive_table, label_name, shed->directive_list,shed->symbol_table, &word_list->tail->address);
   
}

/**
 * @brief This function translate all register words, unlike what the name suggests.
 * 
 * @param word to store the binary code.
 * @param register_name 
 * @param mode to know where to flip the bits.
 * @param word_list 
 */
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
    return;
}

/**
 * @brief This function translates a number.
 * 
 * @param word to store the binary,
 * @param value the number
 * @param word_list 
 */
void translate_imidiate(Word* word, char* value, WordList* word_list) {
    word->type.imidiate_word.are = 4;
    word->type.imidiate_word.value = atoi(value);
    add_word_node(word_list, word);
    return;
}

/**
 * @brief This function translates the string directive values.
 * 
 * @param node the node
 * @param word_list data list
 * @param shed a general struct to make the function signiture a bit less cluttered. 
 * @return WordNode* used for when the directive is a part of a label's definition, so an address is needed to be passed back for storage.
 */
WordNode* translate_string_directive (ASTNode* node, WordList* word_list, Shed* shed) { 
    char* ptr = node->data.directive.value;
    Word* directive_word;
    int first_word = 0;
    WordNode* label_word;

    for (; *(ptr) != '\0'; ptr++) {
        directive_word = malloc(sizeof(Word));
        if (directive_word == NULL) {
            memory_allocation_failure();
        }
        memset(directive_word, 0, sizeof(Word));
        directive_word->word_type = DIRECTIVE_WORD;
        directive_word->type.directive_word.value = *ptr;
        add_word_node(word_list, directive_word);
        if (first_word == 0) {
            first_word = 1;
            label_word = word_list->tail;
        }
    }

    directive_word = malloc(sizeof(Word));
    if (directive_word == NULL) {
        memory_allocation_failure();
    }
    directive_word->word_type = DIRECTIVE_WORD;
    directive_word->type.directive_word.value = 0;
    add_word_node(word_list, directive_word);

    return label_word;
}

/**
 * @brief This function translates the data directive values.
 * 
 * @param node the node. 
 * @param word_list data list
 * @param shed a general struct to make the function signiture a bit less cluttered. 
 * @return WordNode* used for when the directive is a part of a label's definition, so an address is needed to be passed back for storage.
 */
WordNode* translate_data_directive(ASTNode* node, WordList* word_list, Shed* shed) {
    char* ptr = node->data.directive.value;
    int first_word = 0;
    WordNode* label_word;

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


        add_word_node(word_list, directive_word);
        if (first_word == 0) {
            first_word = 1;
            label_word = word_list->tail;
        }
       
    }
    return label_word;
}

/**
 * @brief This function delegate the printing action of different types of words based on their types.
 * 
 * @param word 
 */
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
        default:
            printf("\033[31mpanic!\033[0m Uknowed word type");
    }
}

/**
 * @brief This function is used in order to make the action of calling the translation module from "main" feel less cluttered.
 * 
 * @param shed 
 * @param code_list 
 */
void translate(Shed* shed, WordList* code_list) {
    if (shed->node_list->head != NULL) {
        delegate_node(shed->node_list->head, code_list, shed->data_list, shed);
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
 * @brief This function translates label definition. 
 * 
 * @param node the node.
 * @param code_list 
 * @param data_list 
 * @param shed used to make the signiture shorter.
 */
void translate_labels(ASTNode* node, WordList* code_list, WordList* data_list, Shed* shed) {
    WordNode* label_word;
    if (lookup_symbol(shed->symbol_table, node->data.label.name) != NULL) {
        printf("\033[31mpanic!\033[0m at line %d: %s is already defined\n", node->line, node->data.label.name);
        translation_error = 1;
    }
    else if (node->data.label.definition_node->type == AST_INSTRUCTION) {
        Word* word = malloc(sizeof(Word));
        if (word == NULL) {
            memory_allocation_failure();
        }
        word->word_type = FIRST_WORD;
     
        label_word = translate_instruction_first(word, node->data.label.definition_node, code_list, shed);
        insert_symbol(shed->symbol_table, node->data.label.name, &label_word->address);
        check_if_label_is_extern_or_entry(shed->directive_table, node->data.label.name, shed->directive_list, shed->symbol_table, &label_word->address);
    }
    else if (node->data.label.definition_node->type == AST_DIRECTIVE) {
        if (strcmp(node->data.label.definition_node->data.directive.directive, ".data") == 0) {
            label_word = translate_data_directive(node->data.label.definition_node, data_list, shed);
            insert_symbol(shed->symbol_table, node->data.label.name, &label_word->address);
            check_if_label_is_extern_or_entry(shed->directive_table, node->data.label.name, shed->directive_list, shed->symbol_table, &label_word->address);
        }
        else if (strcmp(node->data.label.definition_node->data.directive.directive, ".string") == 0) {
            label_word = translate_string_directive(node->data.label.definition_node, data_list, shed);
            insert_symbol(shed->symbol_table, node->data.label.name, &label_word->address);
            check_if_label_is_extern_or_entry(shed->directive_table, node->data.label.name, shed->directive_list, shed->symbol_table, &label_word->address); 
        }
    }
}

/**
 * @brief This function iterates through the unresolved label references and looks for the label definitions in order to resolve them. 
 * 
 * @param shed 
 */
void resolve_unresolved_list(Shed* shed) {
    unresolvedLabelRef* node = shed->unresolved_list->head;
    unresolvedLabelRef* prev = NULL;
    unresolvedLabelRef* temp;
    Symbol* found = NULL;
    directiveRef* found_directive = NULL;

    while (node != NULL) {
        found = lookup_symbol(shed->symbol_table, node->name);

        if (found != NULL) {
            node->word->type.direct_word.address = *found->address;
            node->word->type.direct_word.are = 2;  /* Set ARE to 2 if resolved*/

            if (prev == NULL) {
                shed->unresolved_list->head = node->next;
            } else {
                prev->next = node->next;
            }

            temp = node;
            node = node->next;
            free(temp->name);
            free(temp);
        } else {
            found_directive = lookup_directive(shed->directive_table, node->name);
            if (found_directive != NULL && found_directive->directive_type == EXTERN_DIRECTIVE) {
                node->word->type.direct_word.are = 1;  
                prev = node;
                node = node->next;
            }
            else if (found_directive != NULL && found_directive->directive_type == ENTRY_DIRECTTIVE) {
                printf("panic!: %s label was previously mentioned with the entry tag but no defenition was ever provided\n", node->name);
                label_resolution_error = 1;
                node = node->next;
            }
            else if (found_directive == NULL) {
                printf("panic!: the label %s was mentioned in the file but it was never defeined or declared with an extern tag\n", node->name);
                label_resolution_error = 1;
                node = node->next;
            }
        }
    }

}

/**
 * @brief This function creates a word list.
 * 
 * @return WordList* a pointer to the newly created word list.
 */
WordList* create_word_list() {
    WordList* list = (WordList*)malloc(sizeof(WordList));
    if (list == NULL) {
        memory_allocation_failure();
    }
    list->head = NULL;
    list->tail = NULL;
    return list;
}

/**
 * @brief This function adds a word to the releven list.
 * 
 * @param list the list.
 * @param word the word.
 */
void add_word_node(WordList* list, Word* word) {
    WordNode* new_node = (WordNode*)malloc(sizeof(WordNode));
    if (new_node == NULL) {
        memory_allocation_failure();
    }

    new_node->word = word;
    new_node->next = NULL;

    if (list->head == NULL) {
        list->head = new_node;
        list->tail = new_node;
        new_node->address = 0;
    } else {
        new_node->address = list->tail->address + 1;
        list->tail->next = new_node;
        list->tail = new_node;
    }
}

/**
 * @brief This function clears the word list.
 * 
 * @param list the list.
 */
void clear_word_list(WordList* list) {
    WordNode* curr = list->head;
    WordNode* next;

    while (curr != NULL) {
        next = curr->next;
        free(curr->word);
        free(curr);
        curr = next;
    }

    list->head = NULL;
    list->tail = NULL;
}

/**
 * @brief This function prints the word list.
 * 
 * @param list the list.
 */
void print_word_list(WordList* list) {
    WordNode* current = list->head;
    while (current != NULL) {
        printf("%d - ", current->address);
        print_word(current->word);
        current = current->next;
    }
}

/**
 * @brief This function clears the actual list.
 * 
 * @param list the list.
 */
void free_word_list(WordList* list) {
    clear_word_list(list);
    free(list);
}

/**
 * @brief This function checks if a label was defined as an extern/entry in the file. this function is relevn for the ent/ext file creation in the later stages.
 * 
 * @param directive_table 
 * @param label_name 
 * @param directive_list 
 * @param symbol_table 
 * @param address 
 */
void check_if_label_is_extern_or_entry(DirectiveTable* directive_table, char* label_name, DirectiveList* directive_list, SymbolTable* symbol_table, int* address) {
    directiveRef* found = lookup_directive(directive_table, label_name);
    Symbol* defined_label = lookup_symbol(symbol_table, label_name);
    if (found != NULL) {
        if (found->directive_type == EXTERN_DIRECTIVE && defined_label == NULL) {
            add_directive_node(directive_list, label_name, address, EXTERN_DIRECTIVE);
        }
        else if (found->directive_type == ENTRY_DIRECTTIVE && defined_label != NULL) {
            add_directive_node(directive_list, label_name, address, ENTRY_DIRECTTIVE);
        }
    }
}

/**
 * @brief This function creates a directive list.
 * 
 * @return DirectiveList* a pointer to the newly created list.
 */
DirectiveList* create_directive_list() {
    DirectiveList* list = (DirectiveList*)malloc(sizeof(DirectiveList));
    if (list == NULL) {
        memory_allocation_failure();
    }
    list->head = NULL;
    return list;
} 

/**
 * @brief This function adds a node into the directive list.
 * 
 * @param directive_list 
 * @param label_name 
 * @param address 
 * @param directive_type 
 */
void add_directive_node(DirectiveList* directive_list, char* label_name, int* address, diretiveType directive_type) {
    DirectiveNode* new_node = malloc(sizeof(DirectiveNode));
    if (new_node == NULL) {
        memory_allocation_failure();
    }

    new_node->address = address;
    new_node->directive_type = directive_type;
    new_node->labal_name = strdup(label_name);
    if (new_node->labal_name == NULL) {
        free(new_node);
        memory_allocation_failure();
    }

    new_node->next = directive_list->head;
    directive_list->head = new_node;
}

/**
 * @brief This function clears the directive list.
 * 
 * @param list the list.
 */
void clear_directive_list(DirectiveList* list) {
    DirectiveNode* curr = list->head;
    DirectiveNode* next;

    while (curr != NULL) {
        next = curr->next;
        free(curr->labal_name);
        free(curr);
        curr = next;
    }

    list->head = NULL;
}

/**
 * @brief This function prints the directive list for testing.
 * 
 * @param list the list.
 */
void print_directive_list(DirectiveList* list) {
    DirectiveNode* curr = list->head;
    while (curr != NULL) {
        if (curr->directive_type == EXTERN_DIRECTIVE) {
            printf("EXTERN %s at address %d\n", curr->labal_name, *curr->address);
            curr = curr->next;
        }
        else {
            printf("ENTRY %s at address %d\n", curr->labal_name, *curr->address);
            curr = curr->next;
        }
    }
}

/**
 * @brief This function patches the address of both word lists (code and data) so they could be used in the file creation module.
 * 
 * @param code_list 
 * @param data_list 
 */
void stich_both_lists(WordList* code_list, WordList* data_list) {
    WordNode* code_list_index = code_list->head;
    WordNode* data_list_index = data_list->head;
    int val;
    while (code_list_index != NULL) {
        code_list_index->address = (code_list_index->address+100);
        code_list_index = code_list_index->next;
    }

    val = code_list->tail->address + 1;

    while (data_list_index != NULL) {
        data_list_index->address = (data_list_index->address + val);
        data_list_index = data_list_index->next;  
    }

}

