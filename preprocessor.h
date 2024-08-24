#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "symbol_table.h"
#include <stdio.h>

typedef enum {
    NORMAL_LINE, DEFINING_MACRO 
} defenition_state;

typedef enum {
    INPUT, OUTPUT
} useCase;

typedef struct Macro {
    char* name;
    char* definition;
    struct Macro* next;
} Macro;

typedef struct {
    Macro* head;
} MacroList;

MacroList* create_macro_list();
void add_macro(MacroList* list, char* macro_name, char* macro_defenition);
void clear_macro_list(MacroList* list);
void replace_macro(MacroList* macro_list, char* curr_line, FILE* output_file);
int is_macro(MacroList* macro_list, char* name); 
char* add_suffix_to_file_name(char* curr_file_name, char* new_suffix, useCase use_case);
int process_file(FILE* input, FILE* output, MacroList* macro_list); 

#endif 