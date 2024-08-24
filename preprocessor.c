/**
 * @file preprocessor.c
 * @author David Israel (you@domain.com)
 * @brief This module's goal is to turn an "as" file into an "am" file.
 * @version Final.
 * @date 2024-08-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "preprocessor.h"
#include "lexer.h"
#include "panic.h"
#include <string.h>

/**
 * @brief This function takes a file and processes it to create an output file after all macros have been inserted into their positions.
 * 
 * @param input input file.
 * @param output output file.
 * @param macro_list macro_list to store all of the macros that were defibned in the file.
 * @return int 1 if an error was found, 0 if all is well.
 */
int process_file(FILE* input, FILE* output, MacroList* macro_list) {
    char line[MAX_LINE_LEN];
    char macro_name[MAX_MACRO_NAME_LEN];
    char macro_defenition[MAX_MACRO_DEFENITION_LEN];
    int state = NORMAL_LINE;
    int line_number = 1;
    int error_detected = 0;
    
    while (fgets(line, sizeof(line), input)) {
        char* curr_line = line;
        while (isspace(*curr_line)) curr_line++;

        if (curr_line[0] == ';') {
            line_number++;
            continue;
        }

        if (curr_line[0] == '\n' || curr_line[0] == '\0') {
            line_number++;
            continue;
        }

        if (sscanf(curr_line, "macr %s", macro_name)) {
            state = DEFINING_MACRO;
            macro_defenition[0] = '\0';
            line_number++;
        }

        else if (state == DEFINING_MACRO && strstr(curr_line, "endmacr")) {
            if (is_opcocde(macro_name) || is_register(macro_name)) {
                printf("\033[31mpanic!\033[0m at line %d: \"%s\" is a reserved word, and so cannot be used as a macro name\n", line_number, macro_name);
                line_number++;
                error_detected = 1;
                continue;
            }
            add_macro(macro_list, macro_name, macro_defenition);
            state = NORMAL_LINE;
            line_number++;
        }
        else {
            replace_macro(macro_list, curr_line, output);
            line_number++;
        }
    }
    return error_detected;
}

/**
 * @brief This function creates a macro list.
 * 
 * @return MacroList* the macro list.
 */
MacroList* create_macro_list() {
    MacroList* list = malloc(sizeof(MacroList));
    list->head = NULL;

    return list;
}

/**
 * @brief This functiob adds macros into the macro list.
 * 
 * @param list the macro list.
 * @param macro_name new macro's name.
 * @param macro_defenition new macro's defenition.
 */
void add_macro(MacroList* list, char* macro_name, char* macro_defenition) {
    Macro* new_macro = malloc(sizeof(Macro));
    if (new_macro == NULL) {
        memory_allocation_failure();
    }

    new_macro->name = strdup(macro_name);
    new_macro->definition = strdup(macro_defenition);

    if (new_macro->name == NULL || new_macro->definition == NULL) {
        free(new_macro);
        memory_allocation_failure();
    }

    new_macro->next = list->head;
    list->head = new_macro;
}

/**
 * @brief This function clears the macro list.
 * 
 * @param list the list to be cleard 
 */
void clear_macro_list(MacroList* list) {
    Macro* curr = list->head;
    Macro* next;

    while (curr != NULL) {
        next = curr->next;
        free(curr->name);
        free(curr->definition);
        curr = next;
    }

    list->head = NULL;
    list = NULL;
}

/**
 * @brief This function replaces a macro call with it's defenition.
 * 
 * @param macro_list the macro list.
 * @param curr_line current line.
 * @param output_file the output file.
 */
void replace_macro(MacroList* macro_list, char* curr_line, FILE* output_file) {
    Macro* curr_macro = macro_list->head;
    int found = 0;
    while (curr_macro != NULL) {
        if (strstr(curr_line, curr_macro->name)) {
            found = 1;
            fputs(curr_macro->definition, output_file);
            break;
        }
        curr_macro = curr_macro->next;
    }
    if (found == 0) {
        fputs(curr_line, output_file);
    }
}

/**
 * @brief This function is used in the tokenizetion process to detrmin if a label token has the same name as a pre-defined macro.
 * 
 * @param macro_list macro list.
 * @param name label name.
 * @return int return 1 if they share the same name, and 0 if they don't.
 */
int is_macro(MacroList* macro_list, char* name) {
    Macro* curr = macro_list->head;
    while (curr != NULL) {
        if (strcmp(name, curr->name) == 0) {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

/**
 * @brief This function takes a file's name and adds/changes its suffix.
 * 
 * @param curr_file_name 
 * @param new_suffix 
 * @param use_case to signal if dealing with the original input file or using the function to create output files.
 * @return char* the new file name.
 */
char* add_suffix_to_file_name(char* curr_file_name, char* new_suffix, useCase use_case) {
    char* suffix_start;
    char* new_file_name;

    suffix_start = strrchr(curr_file_name, '.');

    if (use_case == OUTPUT) {
    
        if (suffix_start == NULL) {
            printf("\033[31mpanic!\033[0m %s can't be processed as it lacks the correct suffix: \".as\"\n", curr_file_name);
            EXIT_FAILURE;
        }
        else {
            new_file_name = malloc((suffix_start - curr_file_name) + strlen(new_suffix) + 1);
            if (new_file_name == NULL) {
                memory_allocation_failure();
            }
            strncpy(new_file_name, curr_file_name, suffix_start - curr_file_name);
            new_file_name[suffix_start - curr_file_name] = '\0';
        }
    }
    if (use_case == INPUT) {
        new_file_name = malloc(strlen(curr_file_name) + strlen(new_suffix) + 1);
        if (new_file_name == NULL) {
            memory_allocation_failure();
        }

        strcpy(new_file_name, curr_file_name);
    }
    strcat(new_file_name, new_suffix);

    return new_file_name;
}
