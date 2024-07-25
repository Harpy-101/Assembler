#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_HASH_TABLE_SIZE 100
#define LOAD_FACTOR 0.8

typedef struct Symbol {
    char* name;
    int address;
    struct Symbol* next;
} Symbol;

typedef struct {
    Symbol **map;
    int size;
    int count;
} HashTable;

typedef struct unresolvedLabelRef {
    char* name;
    int address;
    struct unresolvedLabelRef* next;
} unresolvedLabelRef;

typedef struct {
    unresolvedLabelRef* head;
} unresolvedLabelRefList;

unsigned int hash(char *s, int hashsize);
HashTable* create_hash_table(int table_size);
void insert_symbol(HashTable* hash_table, char* name, int line_address);
Symbol* lookup_symbol(HashTable* hash_table, char* name);
void clean_symbol_table(HashTable* hash_table); 
void resize_hash_table(HashTable* hash_table);
unresolvedLabelRefList* create_unresolved_label_list();
void add_unresolved_label(char* name, int address, unresolvedLabelRefList* unresolved_list); 
