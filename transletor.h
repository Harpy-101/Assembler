#ifndef TRANSLETOR_H
#define TRANSLETOR_H

#include "ast.h"
#include <stdint.h>
#include <string.h>
#include "panic.h"
#include "symbol_table.h"

typedef enum {
    FIRST_WORD, DIRECT_WORD, IMIDIATE_WORD, REGISTER_WORD, DIRECTIVE_WORD
} wordType;

typedef struct {
    unsigned int are : 3;
    unsigned int destenation_address : 4;
    unsigned int source_address : 4;
    unsigned int opcode : 4;
} firstWord;

typedef struct {
    unsigned int are : 3;
    unsigned int address : 12;
} directWord;

typedef struct {
    unsigned int are : 3;
    unsigned int value : 12;
} imidiateWord;

typedef struct {
    unsigned int are : 3;
    unsigned int source_regiester_name : 3;
    unsigned int destination_register_name : 3;
    unsigned int padding : 6; 
} registerWord;

typedef struct {
    unsigned int value : 15;
} directiveWord;

typedef struct {
    union{
        firstWord first_word;
        directWord direct_word;
        imidiateWord imidiate_word;
        registerWord register_word;
        directWord directive_word;
    } type;
    wordType word_type;
} Word;

typedef struct {
    unresolvedLabelRefList* unresolved_list;
    HashTable* symbol_table;
    char* file_name;
    directWord* direct_word;
    imidiateWord* imidiate_word;
    registerWord* register_word;
    ASTNodeList* node_list;
    int* ic;
    int* dc;
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
void delegate_node(ASTNode* node, char* file_name, Shed* shed);
void print_first_word(firstWord* word);
void print_direct_word(directWord* word);
void print_imidiate_word(imidiateWord* word); 
void print_register_word(registerWord* word); 
firstWord* translate_instruction_first(firstWord* first_word ,ASTNode* node, Shed* shed);
void translate(Shed* shed);
void print_directive_word(directiveWord* word); 
void translate_string_directive (ASTNode* node, Shed* shed);
void translate_data_directive(ASTNode* node, Shed* shed); 


#endif