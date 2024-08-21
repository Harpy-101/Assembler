#include "file_generator.h"
#include "preprocessor.h"
#include "symbol_table.h"
#include "transletor.h"
#include <stdio.h>

void create_output_files(Shed* shed, WordList* code_list, WordList* data_list) {

}

void create_object_file(Shed* shed, WordList* code_list, WordList* data_list) {
    WordNode* curr = code_list->head;
    char output_text[50];

    char* obj_file_name = add_suffix_to_file_name(shed->curr_file_name, ".obj");
    FILE* obj_file = fopen(obj_file_name, "w");

    while (curr != NULL) {
        // Need to add a function to print words based on their type
        /*snprintf(output_text, sizeof(output_text), "%04d  &o\n", curr->address, curr->word.)*/
    }
}

void create_extern_and_entry_files(Shed* shed) {
    DirectiveNode* curr = shed->directive_list->head;
    char output_text[50];

    char* ent_file_name = add_suffix_to_file_name(shed->curr_file_name, ".ent");
    char* ext_file_name = add_suffix_to_file_name(shed->curr_file_name, ".ext");
    FILE* ent_file = fopen(ent_file_name, "w");
    FILE* ext_file = fopen(ext_file_name, "w");

    while (curr != NULL) {
        if (curr->directive_type == ENTRY_DIRECTTIVE) {
            snprintf(output_text, sizeof(output_text), "%04d  %s\n", *curr->address, curr->labal_name);
            fputs(output_text, ent_file);
        }
        else if (curr->directive_type == EXTERN_DIRECTIVE){
            snprintf(output_text, sizeof(output_text), "%04d  %s\n", *curr->address, curr->labal_name);
            fputs(output_text, ext_file);
        }
        curr = curr->next;
    }

    fclose(ent_file);
    fclose(ext_file);
}