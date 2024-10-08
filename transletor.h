#ifndef TRANSLETOR_H
#define TRANSLETOR_H

#include "lexer.h"
#include "symbol_table.h"
#include <stdint.h>
#include <string.h>
#include "panic.h"

#define A 4
#define R 1
#define E 0

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

typedef struct DirectiveNode {
    diretiveType directive_type;
    int* address;
    char* labal_name;
    struct DirectiveNode* next;
} DirectiveNode;

typedef struct {
    DirectiveNode* head;
} DirectiveList;

typedef struct {
    unresolvedLabelRefList* unresolved_list;
    SymbolTable* symbol_table;
    DirectiveTable* directive_table;
    char* file_name;
    directWord* direct_word;
    WordList* code_list;
    WordList* data_list;
    imidiateWord* imidiate_word;
    registerWord* register_word;
    ASTNodeList* node_list;
    DirectiveList* directive_list;
    char* curr_file_name;
    int* ic;
    int* dc;
} Shed;

typedef enum {
    SOURCE, DESTENATION
} mode;

int id_opcode(char* instruction_name);
int id_addressing_mode(int addressing_mode); 
void translate_indirect_register(Word* word, char* register_name, mode mode, WordList* word_list);
void translate_direct(Word* word, char* label_name, WordList* word_list, Shed* shed);
void translate_imidiate(Word* word, char* value, WordList* word_list); 
void handle_second_word(ASTNode* node, Word* word, int source_addressing_mode, int destenation_addressing_mode, WordList* word_list, Shed* shed); 
void delegate_node(ASTNode* node, WordList* code_list, WordList* data_list, Shed* shed);
void print_first_word(firstWord* word);
void print_direct_word(directWord* word);
void print_imidiate_word(imidiateWord* word); 
void print_register_word(registerWord* word); 
WordNode* translate_instruction_first(Word* first_word ,ASTNode* node, WordList* word_list, Shed* shed);
void translate(Shed* shed, WordList* code_list);
void print_directive_word(directiveWord* word); 
WordNode* translate_string_directive (ASTNode* node, WordList* word_list, Shed* shed);
WordNode* translate_data_directive(ASTNode* node, WordList* word_list, Shed* shed); 
void translate_labels(ASTNode* node, WordList* code_list, WordList* data_list, Shed* shed);
WordList* create_word_list();
void add_word_node(WordList* list, Word* word);
void clear_word_list(WordList* list);
void print_word_list(WordList* list);
void free_word_list(WordList* list);
void print_first_word_to_file(FILE *file, firstWord *word);
void print_word(Word* word);
void check_if_label_is_extern_or_entry(DirectiveTable* directive_table, char* label_name, DirectiveList* directive_list, SymbolTable* symbol_table, int* address); 
DirectiveList* create_directive_list();
void add_directive_node(DirectiveList* directive_list, char* label_name, int* address, diretiveType directive_type);
void clear_directive_list(DirectiveList* list);
void print_directive_list(DirectiveList* list);
void stich_both_lists(WordList* code_list, WordList* data_list);
void resolve_unresolved_list(Shed* shed); 


#endif