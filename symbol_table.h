#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Symbol {
    char* name;
    int address;
    struct Symbol* next;
} Symbol;

typedef struct {
    Symbol **map;
    int size;
} HashTable;

unsigned int hash(char *s, int hashsize);
HashTable* create_hash_table(int table_size);
void insert_symbol(HashTable* hash_table, char* name, int line_address);
Symbol* lookup_symbol(HashTable* hash_table, char* name);
