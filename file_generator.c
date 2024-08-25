/**
 * @file file_generator.c
 * @author David Israel
 * @brief 
 * @version Final
 * @date 2024-08-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "file_generator.h"
#include "error_flags.h"
#include "preprocessor.h"
#include "symbol_table.h"
#include "transletor.h"
#include "word_types.h"
#include <stdio.h>

/**
 * @brief This fuction calls the file creatoion functions.
 * 
 * @param shed 
 */
void create_output_files(Shed* shed) {
    create_object_file(shed, shed->code_list, shed->data_list);
    
    if (shed->data_list->tail->address > 4096) {
        printf("\033[31mpanic!\033[0m) Source file requires more memory to run than the host system has to offer. Aborting compilatio\n");
        return;
    }
    if (label_resolution_error == 1) {
        return;
    }
    
    create_extern_and_entry_files(shed);
}

/**
 * @brief Create a object file objectThis function crerates the ".ob" file.
 * 
 * @param shed 
 * @param code_list 
 * @param data_list 
 */
void create_object_file(Shed* shed, WordList* code_list, WordList* data_list) {
    WordNode* curr = code_list->head;
    char output_text[50];
    unsigned int word;
    int code_word_count;
    int data_word_count;

    char* ob_file_name = add_suffix_to_file_name(shed->file_name, ".ob", OUTPUT);
    FILE* ob_file = fopen(ob_file_name, "w");
    free(ob_file_name);

    code_word_count = code_list->tail->address + 1;
    data_word_count = data_list->tail->address + 1;

    stich_both_lists(code_list, data_list);
    resolve_unresolved_list(shed);

    if (label_resolution_error == 1) {
        return;
    }

    if (data_list->tail->address > 4096) {
        printf("\033[31mpanic!\033[0m) Source file requires more memory to run than the host system has to offer. Aborting compilatio\n");
        return;
    }

    sprintf(output_text, "   %d  %d\n", code_word_count, data_word_count);    
    fputs(output_text, ob_file);

    while (curr != NULL) {
        switch (curr->word->word_type) {
            case FIRST_WORD:
                word = (curr->word->type.first_word.are) | (curr->word->type.first_word.destenation_address << 3) | (curr->word->type.first_word.source_address << 7) | (curr->word->type.first_word.opcode << 11);
                sprintf(output_text, "%04d  %05o\n", curr->address, word);
                break;

            case DIRECT_WORD:
                word = (curr->word->type.direct_word.are) | (curr->word->type.direct_word.address << 3);
                sprintf(output_text, "%04d  %05o\n", curr->address, word);
                break;

            case IMIDIATE_WORD:
                word = (curr->word->type.imidiate_word.are) | (curr->word->type.imidiate_word.value << 3);
                sprintf(output_text, "%04d  %05o\n", curr->address, word);
                break;

            case REGISTER_WORD:
                word = (curr->word->type.register_word.are) | (curr->word->type.register_word.destination_register_name << 3) | (curr->word->type.register_word.source_regiester_name << 6) | (curr->word->type.register_word.padding << 9);
                sprintf(output_text, "%04d  %05o\n", curr->address, word);
                break;

            case DIRECTIVE_WORD:
                word = curr->word->type.directive_word.value;
                sprintf(output_text, "%04d  %05o\n", curr->address, word);
                break;

            default:
                printf("\033[31mpanic!\033[0m unknown word type\n");
        } 
        fputs(output_text, ob_file);
        curr = curr->next;
    }

    curr = data_list->head;
   
    while (curr != NULL) {
        switch (curr->word->word_type) {
            case FIRST_WORD:
                word = (curr->word->type.first_word.are) | (curr->word->type.first_word.destenation_address << 3) | (curr->word->type.first_word.source_address << 7) | (curr->word->type.first_word.opcode << 11);
                sprintf(output_text, "%04d  %05o\n", curr->address, word);
                break;

            case DIRECT_WORD:
                word = (curr->word->type.direct_word.are) | (curr->word->type.direct_word.address << 3);
                sprintf(output_text, "%04d  %05o\n", curr->address, word);
                break;

            case IMIDIATE_WORD:
                word = (curr->word->type.imidiate_word.are) | (curr->word->type.imidiate_word.value << 3);
                sprintf(output_text, "%04d  %05o\n", curr->address, word);
                break;

            case REGISTER_WORD:
                word = (curr->word->type.register_word.are) | (curr->word->type.register_word.destination_register_name << 3) | (curr->word->type.register_word.source_regiester_name << 6) | (curr->word->type.register_word.padding << 9);
                sprintf(output_text, "%04d  %05o\n", curr->address, word);
                break;

            case DIRECTIVE_WORD:
                word = curr->word->type.directive_word.value;
                sprintf(output_text, "%04d  %05o\n", curr->address, word);
                break;

            default:
                printf("\033[31mpanic!\033[0m unknown word type\n");
        }
        fputs(output_text, ob_file);
        curr = curr->next;
    }

    fclose(ob_file);
}

/**
 * @brief This function creates the .ent/ext files.
 * 
 * @param shed 
 */
void create_extern_and_entry_files(Shed* shed) {
    DirectiveNode* curr = shed->directive_list->head;
    char output_text[50];

    char* ent_file_name = add_suffix_to_file_name(shed->file_name, ".ent", OUTPUT);
    char* ext_file_name = add_suffix_to_file_name(shed->file_name, ".ext", OUTPUT);
    FILE* ent_file = fopen(ent_file_name, "w");
    FILE* ext_file = fopen(ext_file_name, "w");
    free(ent_file_name);
    free(ext_file_name);

    while (curr != NULL) {
        if (curr->directive_type == ENTRY_DIRECTTIVE) {
            sprintf(output_text, "%04d  %s\n", *curr->address, curr->labal_name);
            fputs(output_text, ent_file);
        }
        else if (curr->directive_type == EXTERN_DIRECTIVE){
            sprintf(output_text, "%04d  %s\n", *curr->address, curr->labal_name);
            fputs(output_text, ext_file);
        }
        curr = curr->next;
    }

    fclose(ent_file);
    fclose(ext_file);
}