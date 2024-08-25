#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "word_types.h"


#define INITIAL_HASH_TABLE_SIZE 100
#define LOAD_FACTOR 0.8


struct ASTNode;

typedef enum {
    EXTERN, ENTRY, UNDEFINED
} symbol_scope;

typedef enum {
    EXTERN_DIRECTIVE, ENTRY_DIRECTTIVE
} diretiveType;

typedef enum {
    IC, DC
} address_type;

typedef struct Symbol {
    char* name;
    int* address;
    char* definition_origin;    
    struct Symbol* next;
    struct ASTNode* dfinition_node; /* This was originally supposed to be used for the linker*/
} Symbol;

typedef struct {
    Symbol **map;
    int size;
    int count;
} SymbolTable;

typedef struct {
    unsigned int are : 3;
    unsigned int address : 12; 
} labelWord;

typedef struct unresolvedLabelRef {
    char* name;
    int* address;
    address_type ad_type;
    symbol_scope scope;
    char* origin;
    Word* word;
    struct unresolvedLabelRef* next;
} unresolvedLabelRef;

typedef struct {
    unresolvedLabelRef* head;
} unresolvedLabelRefList;

typedef struct directiveRef{
    char* name;
    diretiveType directive_type; 
    struct directiveRef* next;
} directiveRef;

typedef struct {
    directiveRef **map;
    int size;
    int count;
} DirectiveTable;


unsigned int hash(char *s, int hashsize);
SymbolTable* create_hash_table(int table_size);
void insert_symbol(SymbolTable* hash_table, char* name, int* line_address);
Symbol* lookup_symbol(SymbolTable* hash_table, char* name);
void clean_symbol_table(SymbolTable* hash_table); 
void resize_hash_table(SymbolTable* hash_table);
unresolvedLabelRefList* create_unresolved_label_list();
void add_unresolved_label(char* name, int* address, Word* word, unresolvedLabelRefList* unresolved_list);
char* strdup(const char* src); 
void print_symbol_table(SymbolTable* symbol_table); 
void print_unresolved_list(unresolvedLabelRefList* list);
/*void resolve_unresolved_list(HashTable* symbol_table, unresolvedLabelRefList* list); */
DirectiveTable* create_directive_table(int size);
void resize_directive_table(DirectiveTable* hash_table);
void clean_directive_table(DirectiveTable* directive_table);
directiveRef* lookup_directive(DirectiveTable* directive_table, char* name);
void insert_directive(DirectiveTable* directive_table, char* name, int directive_type);
void clear_unresolved_list(unresolvedLabelRefList* list);



#endif