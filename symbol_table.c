/**
 * @file symbol_table.c
 * @author David Israel
 * @brief This module contains all the hash table implementations in this program. 
 *        The main idea was to create a generic hash table implementation to be used more extensively,
 *        however due to time constraints and complexity I ended up scrapping the idea completely, which resulted in repetitive code. 
 * @version Final
 * @date 2024-08-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "symbol_table.h"
#include "panic.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief This is the hashing function for all the hash tables in this program (taken from the: "Learning the C programming language" (p.144 in the second edition).
 * 
 * @param s the string to be hashed.
 * @param hashsize the table size.
 * @return unsigned int 
 */
unsigned int hash(char *s, int hashsize) {
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    return hashval % hashsize;
}

/**
 * @brief This function creats a symbol hash-table.
 * 
 * @param size original table size.
 * @return SymbolTable* a pointer to the newly created hash table 
 */
SymbolTable* create_hash_table(int size) {
    SymbolTable* hash_table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (hash_table == NULL) {
        memory_allocation_failure();
    }

    hash_table->size = size;
    hash_table->count = 0;
    hash_table->map = (Symbol**)calloc(size, sizeof(Symbol*));
    if (hash_table->map == NULL) {
        memory_allocation_failure();
    }

    return hash_table;
}

/**
 * @brief This function creats a directive hash table for the extern/entry calls.
 * 
 * @param size table size.
 * @return DirectiveTable* a pointer to the newly crated directives table. 
 */
DirectiveTable* create_directive_table(int size) {
    DirectiveTable* directive_table = (DirectiveTable*)malloc(sizeof(DirectiveTable));
    if (directive_table == NULL) {
        memory_allocation_failure();
    }

    directive_table->size = size;
    directive_table->count = 0;
    directive_table->map = (directiveRef**)calloc(size, sizeof(directiveRef*));
    if (directive_table->map == NULL) {
        memory_allocation_failure();
    }
    return directive_table;
}

/**
 * @brief This function iserts a symbol into the symbol table.
 * 
 * @param hash_table the symbol table.
 * @param name the symbols name.
 * @param line_address the stmbols address.
 */
void insert_symbol(SymbolTable* hash_table, char* name, int* line_address) {
    int hash_index;
    Symbol* symbol;

    if ((float)hash_table->count / hash_table->size > LOAD_FACTOR) {
        resize_hash_table(hash_table);
    }

    symbol = (Symbol*)malloc(sizeof(Symbol));
    if (symbol == NULL) {
        memory_allocation_failure();
    }

    symbol->name = strdup(name);
    if (symbol->name == NULL) {
       memory_allocation_failure();
    }

    symbol->address = line_address;
    hash_index = hash(name, hash_table->size);
    symbol->next = hash_table->map[hash_index];
    hash_table->map[hash_index] = symbol;
    hash_table->count++;
}

/**
 * @brief This function iserts a directive call into the directivs table.
 * 
 * @param directive_table the directive hash table.
 * @param name the label name 
 * @param directive_type extern/entry
 */
void insert_directive(DirectiveTable* directive_table, char* name, int directive_type) {
    int hash_index;
    directiveRef* directive;

    if ((float)directive_table->count / directive_table->size > LOAD_FACTOR) {
        resize_directive_table(directive_table);
    }

    directive = (directiveRef*)malloc(sizeof(directiveRef));
    if (directive == NULL) {
        memory_allocation_failure();
    }

    directive->name = strdup(name);
    if (directive->name == NULL) {
        memory_allocation_failure();
    }

    directive->directive_type = directive_type;
    hash_index = hash(name, directive_table->size);
    directive->next = directive_table->map[hash_index];
    directive_table->map[hash_index] = directive;
    directive_table->count++;
}

/**
 * @brief This function looks for the symbol's name in the symbol table. If one is indeed found it  will be returned, if nothing is found the function returns NULL.
 * 
 * @param hash_table the symbol table.
 * @param name the symbol the function looks for.
 * @return Symbol* a pointer to the found symbol (if found) or NULL if the symbol wasn't found.
 */
Symbol* lookup_symbol(SymbolTable* hash_table, char* name) {
    int hash_index = hash(name, hash_table->size);
    Symbol* symbol = hash_table->map[hash_index];
    
    while (symbol != NULL) {
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    return NULL;
}

/**
 * @brief This function looks for the directive label name in the directivs table. If one is indeed found it will be returned, if nothing was found the function returns NULL.
 * 
 * @param directive_table the directiv's table.
 * @param name label name.
 * @return directiveRef* a pointer to the found symbol (if found) or NULL if the symbol wasn't found. 
 */
directiveRef* lookup_directive(DirectiveTable* directive_table, char* name) {
    int hash_index = hash(name, directive_table->size);
    directiveRef* directive = directive_table->map[hash_index];
    
    while (directive != NULL) {
        if (strcmp(directive->name, name) == 0) {
            return directive;
        }
        directive = directive->next;
    }
    return NULL;
}

/**
 * @brief A clear table function fot the symbol table.
 * 
 * @param hash_table the symbol table.
 */
void clean_symbol_table(SymbolTable* hash_table) {
    int i;
    for (i = 0; i < hash_table->size; i++) {
        Symbol* symbol = hash_table->map[i];
        while (symbol != NULL) {
            Symbol* temp = symbol;
            symbol = symbol->next;
            free(temp->name);
            temp->name = NULL;
            free(temp);
        }
    }
    free(hash_table->map);
    free(hash_table);
}

/**
 * @brief This function returns the memory alocated for the direcrivs table.
 * 
 * @param directive_table the firective table.
 */
void clean_directive_table(DirectiveTable* directive_table) {
    int i;
    for (i = 0; i < directive_table->size; i++) {
        directiveRef* directive = directive_table->map[i];
        while (directive != NULL) {
            directiveRef* temp = directive;
            directive = directive->next;
            free(temp->name);
            free(temp);
        }
    }
    free(directive_table->map);
    free(directive_table);
}

/**
 * @brief This function takes the original hash table and expands it if a predefined load factor was reached. 
 * 
 * @param hash_table the original symbol table.
 */
void resize_hash_table(SymbolTable* hash_table) {
    int i;
    int new_size = hash_table->size * 2;
    Symbol** new_map = malloc(new_size * sizeof(SymbolTable));
    if (new_map == NULL) {
        memory_allocation_failure();
    }
    
    for (i = 0; i < new_size; i++) {
        new_map[i] = NULL;
    }

    for (i = 0; i < hash_table->size; i++) {
        Symbol* symbol = hash_table->map[i];
        while (symbol) {
            Symbol* new_symbol = symbol->next;
            unsigned int index = hash(symbol->name, new_size);
            new_symbol->next = new_map[index];
            new_map[index] = new_symbol;
        }
    }
    free(hash_table->map);
    hash_table->map = new_map;
    hash_table->size = new_size;
}

/**
 * @brief This function takes the original hash table and expands it if a predefined load factor was reached. 
 * 
 * @param hash_table the directive table. 
 */
void resize_directive_table(DirectiveTable* hash_table) {
    int i;
    int new_size = hash_table->size * 2;
    directiveRef** new_map = malloc(new_size * sizeof(DirectiveTable));
    if (new_map == NULL) {
        memory_allocation_failure();
    }
    
    for (i = 0; i < new_size; i++) {
        new_map[i] = NULL;
    }

    for (i = 0; i < hash_table->size; i++) {
        directiveRef* directive = hash_table->map[i];
        while (directive) {
            directiveRef* new_directive = directive->next;
            unsigned int index = hash(directive->name, new_size);
            new_directive->next = new_map[index];
            new_map[index] = new_directive;
        }
    }
    free(hash_table->map);
    hash_table->map = new_map;
    hash_table->size = new_size;
}

/**
 * @brief This function creates the unresolved list.
 * 
 * @return unresolvedLabelRefList* a pointer to the list.
 */
unresolvedLabelRefList* create_unresolved_label_list() {
    unresolvedLabelRefList* unresolved_list = malloc(sizeof(unresolvedLabelRefList));
    if (unresolved_list == NULL) {
        memory_allocation_failure();
    }
    unresolved_list->head = NULL;
    return unresolved_list;
}

/**
 * @brief This function adds an unresolved label into the list.
 * 
 * @param name label name.
 * @param address label address.
 * @param word the "Word" structure of this word.
 * @param unresolved_list the list.
 */
void add_unresolved_label(char* name, int* address, Word* word, unresolvedLabelRefList* unresolved_list) {
    unresolvedLabelRef* label = malloc(sizeof(unresolvedLabelRef));
   if (label == NULL) {
        memory_allocation_failure();
    }
    label->name = strdup(name); 
    if (label->name == NULL) {
        memory_allocation_failure();
    }
    label->address = address;
    label->word = word;
    label->next = unresolved_list->head;
    unresolved_list->head = label;
}

/**
 * @brief This function clears the unresolved list.
 * 
 * @param list the list.
 */
void clear_unresolved_list(unresolvedLabelRefList* list) {
    unresolvedLabelRef* node = list->head;
    unresolvedLabelRef* temp;
    while (node) {
        temp = node;
        node = node->next;
        free(temp->name);
        free(temp);
    }
    list->head = NULL;
}

/**
 * @brief This function duplicates a string and returns a dynamically allocated version of it.
 *        This function is my interpretation of the original "strdup" 
 * 
 * @param str the source string 
 * @return char* a pointer to the newly allocated sstring.
 */
char* strdup(const char* str) {
    char* dup = (char*)malloc(strlen(str) + 1); 
    char* ptr;
    if (dup == NULL) {
        return NULL; 
    }

    ptr = dup;
    while (*str) {
        *ptr++ = *str++;
    }
    *ptr = '\0'; 

    return dup;
}

/**
 * @brief A printing function for testing purposes. 
 * 
 * @param symbol_table the symbol table.
 */
void print_symbol_table(SymbolTable* symbol_table) {
    int i;
    for (i = 0; i < symbol_table->size; i++) {
        Symbol* curr_symbol = symbol_table->map[i];
        if (curr_symbol) {
            printf("Index: %d\n", i);
            while (curr_symbol) {
                printf("Symbol: %s, found in line: %d\n", curr_symbol->name, *curr_symbol->address);
                curr_symbol = curr_symbol->next;
            }
        }
    }
}

/**
 * @brief A printing function for testing purposes. 
 * 
 * @param list 
 */
void print_unresolved_list(unresolvedLabelRefList* list) {
    unresolvedLabelRef* node = list->head;
    while (node != NULL) {
        printf("Name: %s, at line: %d\n", node->name, *node->address);
        node = node->next;
    }
}

