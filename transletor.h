#ifndef TRANSLETOR_H
#define TRANSLETOR_H

#include "symbol_table.h"
#include <stdint.h>
#include <string.h>
#include "panic.h"

/*typedef enum {
    FIRST_WORD, DIRECT_WORD, IMIDIATE_WORD, REGISTER_WORD, DIRECTIVE_WORD
} wordType;


typedef struct {
    unsigned int are : 3;
    unsigned int destenation_address : 4;
    unsigned int source_address : 4;
    unsigned int opcode : 4;
} firstWord;

typedef struct directWord{
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

typedef struct  {
    unsigned int value : 15;
} directiveWord;

typedef struct {
    union{
        firstWord first_word;
        directWord direct_word;
        imidiateWord imidiate_word;
        registerWord register_word;
        directiveWord directive_word;
    } type;
    wordType word_type;
} Word;
*/

typedef struct WordNode {
    Word* word;
    struct WordNode* next;
    int address;
    char* word_value;
} WordNode;

typedef struct {
    WordNode* head;
    WordNode* tail;
} WordList;

typedef struct {
    unresolvedLabelRefList* unresolved_list;
    SymbolTable* symbol_table;
    DirectiveTable* directive_table;
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
void translate_indirect_register(Word* word, char* register_name, mode mode, WordList* word_list);
void translate_direct(Word* word, char* label_name, int address, WordList* word_list, Shed* shed);
void translate_imidiate(Word* word, char* value, WordList* word_list); 
void handle_second_word(ASTNode* node, Word* word, int source_addressing_mode, int destenation_addressing_mode, WordList* word_list, Shed* shed); 
void delegate_node(ASTNode* node, WordList* code_list, WordList* data_list, Shed* shed);
void print_first_word(firstWord* word);
void print_direct_word(directWord* word);
void print_imidiate_word(imidiateWord* word); 
void print_register_word(registerWord* word); 
void translate_instruction_first(Word* first_word ,ASTNode* node, WordList* word_list, Shed* shed);
void translate(Shed* shed, WordList* code_list, WordList* data_list);
void print_directive_word(directiveWord* word); 
void translate_string_directive (ASTNode* node, WordList* word_list, Shed* shed);
void translate_data_directive(ASTNode* node, WordList* word_list, Shed* shed); 
void translate_labels(ASTNode* node, WordList* code_list, WordList* data_list, Shed* shed);
WordList* create_word_list();
void add_word_node(WordList* list, Word* word);
void clear_word_list(WordList* list);
void print_word_list(WordList* list);
void free_word_list(WordList* list);
void print_first_word_to_file(FILE *file, firstWord *word);
void print_word(Word* word);
void check_if_label_is_extern_or_entry(DirectiveTable* directive_table, char* label_name); 


#endif