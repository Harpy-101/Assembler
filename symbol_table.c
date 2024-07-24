#include "symbol_table.h"
#include <stdio.h>

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
    hash_table->map = (Symbol**)calloc(size, sizeof(Symbol*));
    if (hash_table->map == NULL) {
        fprintf(stderr, "Memory allocation for hash table map failed\n");
        free(hash_table);
        exit(EXIT_FAILURE);
    }

    return hash_table;
}

void insert_symbol(HashTable* hash_table, char* name, int line_address) {
    int hash_index;
    Symbol* symbol;
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
