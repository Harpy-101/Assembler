#ifndef TRANSLETOR_H
#define TRANSLETOR_H

#include "ast.h"
#include <stdint.h>
#include <string.h>
#include "panic.h"
#include "symbol_table.h"

typedef struct {
    uint16_t are : 3;
    uint16_t destenation_address : 4;
    uint16_t source_address : 4;
    uint16_t opcode : 4;
} firstWord;

typedef struct {
    uint16_t are : 3;
    uint16_t address : 12;
} directWord;

typedef struct {
    uint16_t are : 3;
    uint16_t value : 12;
} imidiateWord;

typedef struct {
    uint16_t are : 3;
    uint16_t source_regiester_name : 3;
    uint16_t destination_register_name : 3;
    uint16_t padding : 6; 
} registerWord;

typedef struct {
    union{
        firstWord first_word;
        directWord direct_word;
        imidiateWord imidiate_word;
        registerWord register_word;
    } type;
} Word;

typedef struct {
    unresolvedLabelRefList* unresolved_list;
    HashTable* symbol_table;
    char* file_name;
    directWord* direct_word;
    imidiateWord* imidiate_word;
    registerWord* register_word;
} Shed;

typedef enum {
    SOURCE, DESTENATION
} mode;

int id_opcode(char* instruction_name);
int id_addressing_mode(int addressing_mode); 
void translate_indirect_register(registerWord* word, char* register_name, mode mode);
void translate_direct(directWord* word, char* label_name, int address, Shed* shed);
void translate_imidiate(imidiateWord* word, char* value); 
void handle_second_word(ASTNode* node, int source_addressing_mode, int destenation_addressing_mode, Shed* shed); 

#endif