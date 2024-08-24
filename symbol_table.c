#include "symbol_table.h"
#include "panic.h"
/*#include "transletor.h"*/
#include <stdio.h>
#include <string.h>

unsigned int hash(char *s, int hashsize) {
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    return hashval % hashsize;
}

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
    /*symbol->name = name;*/
    hash_index = hash(name, hash_table->size);
    symbol->next = hash_table->map[hash_index];
    hash_table->map[hash_index] = symbol;
    hash_table->count++;
    /*printf("Inserted symbol: %s at address: 0x%x in hash index: %d\n", symbol->name, *symbol->address, hash_index);*/
}

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
    /*printf("Inserted directive: %s in hash index: %d\n", directive->name, hash_index);*/
}

Symbol* lookup_symbol(SymbolTable* hash_table, char* name) {
    int hash_index = hash(name, hash_table->size);
    Symbol* symbol = hash_table->map[hash_index];
    
    while (symbol != NULL) {
        /*printf("Comparing '%s' with '%s'\n", symbol->name, name);*/
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    return NULL;
}

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

/* Still need to check this function. 
   It's possible that something in the whiile loop is wrong, but I'm not sure
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

unresolvedLabelRefList* create_unresolved_label_list() {
    unresolvedLabelRefList* unresolved_list = malloc(sizeof(unresolvedLabelRefList));
    if (unresolved_list == NULL) {
        memory_allocation_failure();
    }
    unresolved_list->head = NULL;
    return unresolved_list;
}

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

char* strdup(const char* src) {
   /*Allocate memory for the duplicate string*/
    char* dup = (char*)malloc(strlen(src) + 1); /*. +1 for the null terminator*/
    char* ptr;
    if (dup == NULL) {
        return NULL; /*Allocation failed*/
    }

    /*Copy the string */
    ptr = dup;
    while (*src) {
        *ptr++ = *src++;
    }
    *ptr = '\0'; /* Null-terminate the duplicated string*/

    return dup;
}

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

void print_unresolved_list(unresolvedLabelRefList* list) {
    unresolvedLabelRef* node = list->head;
    while (node != NULL) {
        printf("Name: %s, at line: %d\n", node->name, *node->address);
        node = node->next;
    }
}

/**
 * @brief 
 *
 * @param symbol_table 
 * @param list 
 * @return :
 * todo: Add a proper resolution algorithem for both directives and labels. 
 */
/*void resolve_unresolved_list(SymbolTable* symbol_table, unresolvedLabelRefList* list) {
    unresolvedLabelRef* prev = NULL;
    unresolvedLabelRef* node = list->head;
    Symbol* found;

    if (node == NULL) {
        printf("Symbol table is empty\n");
        return;
    }

    if (lookup_symbol(symbol_table, node->name) != NULL) {
        node= node->next;
    }

    while(node != NULL) {
        found = lookup_symbol(symbol_table, node->name);
        if (found == NULL) {
            prev = node;
            node = node->next;
        }
        else {
            prev->next = node->next;
            free(node);
        }
    }
}*/
/*
HashTable* create_hash_table(
                            int size,
                            void* (lookup_function)(HashTable* table, char* value),
                            int (*add_function)(HashTable* table, char* value, int address),
                            void (*clear_function)(void* data)) {
    HashTable* hash_table = (HashTable*)malloc(sizeof(HashTable));
    if (hash_table == NULL) {
        memory_allocation_failure();
    }

    hash_table->size = size;
    hash_table->count = 0;
    hash_table->map = (void**)calloc(size, sizeof(void*));
    if (hash_table->map == NULL) {
        free(hash_table);
        memory_allocation_failure();
    }

    hash_table->lookup_function = lookup_function;
    hash_table->add_function = add_function;
    hash_table->clear_function = clear_function;
    
    return hash_table;
}
*/