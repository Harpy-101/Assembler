#include "symbol_table.h"
#include <stdio.h>
#include <string.h>

unsigned int hash(char *s, int hashsize) {
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    return hashval % hashsize;
}

HashTable* create_hash_table(int size) {
    HashTable* hash_table = (HashTable*)malloc(sizeof(HashTable));
    if (hash_table == NULL) {
        printf("Memory allocation for hash table failed\n");
        exit(EXIT_FAILURE);
    }

    hash_table->size = size;
    hash_table->count = 0;
    hash_table->map = (Symbol**)calloc(size, sizeof(Symbol*));
    if (hash_table->map == NULL) {
        printf( "Memory allocation for hash table map failed\n");
        free(hash_table);
        exit(EXIT_FAILURE);
    }

    return hash_table;
}

void insert_symbol(HashTable* hash_table, char* name, int line_address) {
    int hash_index;
    Symbol* symbol;

    if ((float)hash_table->count / hash_table->size > LOAD_FACTOR) {
        resize_hash_table(hash_table);
    }

    symbol = (Symbol*)malloc(sizeof(Symbol));
    if (symbol == NULL) {
        printf("Memory alocation failed\n");
        exit(EXIT_FAILURE);
    }

    symbol->name = strdup(name);
    if (symbol->name == NULL) {
        printf("Memory alocation for name failed\n");
        free(symbol);
        exit(EXIT_FAILURE);
    }

    symbol->address = line_address;
    symbol->name = name;
    hash_index = hash(name, hash_table->size);
    symbol->next = hash_table->map[hash_index];
    hash_table->map[hash_index] = symbol;
    hash_table->count++;
    printf("Inserted symbol: %s at address: 0x%x in hash index: %d\n", symbol->name, symbol->address, hash_index);
}

Symbol* lookup_symbol(HashTable* hash_table, char* name) {
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

void clean_symbol_table(HashTable* hash_table) {
    int i;
    for (i = 0; i < hash_table->size; i++) {
        Symbol* symbol = hash_table->map[i];
        while (symbol != NULL) {
            Symbol* temp = symbol;
            symbol = symbol->next;
            free(temp->name);
            free(temp);
        }
    }
    free(hash_table->map);
    free(hash_table);
}

/* Still need to check this function. 
   It's possible that something in the whiile loop is wrong, but I'm not sure
*/
void resize_hash_table(HashTable* hash_table) {
    int i;
    int new_size = hash_table->size * 2;
    Symbol** new_map = malloc(new_size * sizeof(HashTable));
    if (new_map == NULL) {
        printf("Memory alocation for resizing of the hashtable falied\n");
        exit(EXIT_FAILURE);
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

unresolvedLabelRefList* create_unresolved_label_list() {
    unresolvedLabelRefList* unresolved_list = malloc(sizeof(unresolvedLabelRefList));
    if (unresolved_list == NULL) {
        printf("Memory alocation for \"unresolved list\" falied\n");
        exit(EXIT_FAILURE);
    }
    unresolved_list->head = NULL;
    return unresolved_list;
}

void add_unresolved_label(char* name, int address, unresolvedLabelRefList* unresolved_list, char* file_name) {
    unresolvedLabelRef* label = malloc(sizeof(unresolvedLabelRef));
   if (label == NULL) {
        printf("Memory alocation for \"unresolved label\" falied\n");
        exit(EXIT_FAILURE);
    }
    label->name = strdup(name); 
    /* Test that the alocation succeeded */
    label->origin = strdup(file_name);
    label->address = address;
    label->next = unresolved_list->head;
    unresolved_list->head = label;
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

void print_symbol_table(HashTable* symbol_table) {
    int i;
    for (i = 0; i < symbol_table->size; i++) {
        Symbol* curr_symbol = symbol_table->map[i];
        if (curr_symbol) {
            printf("Index: %d\n", i);
            while (curr_symbol) {
                printf("Symbol: %s, found in line: %d\n", curr_symbol->name, curr_symbol->address);
                curr_symbol = curr_symbol->next;
            }
        }
    }
}

void print_unresolved_list(unresolvedLabelRefList* list) {
    unresolvedLabelRef* node = list->head;
    while (node != NULL) {
        printf("Name: %s, at line: %d\n", node->name, node->address);
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
void resolve_unresolved_list(HashTable* symbol_table, unresolvedLabelRefList* list) {
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
}
