#include "preprocessor.h"
#include "lexer.h"
#include "panic.h"
#include "symbol_table.h"
#include <stdio.h>
#include <string.h>


int process_file(FILE* input, FILE* output, MacroList* macro_list) {
    char line[256];
    char macro_name[30];
    char macro_defenition[512];
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
                printf("panic! at line %d: \"%s\" is a reserved word, and so cannot be used as a macro name\n", line_number, macro_name);
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

MacroList* create_macro_list() {
    MacroList* list = malloc(sizeof(MacroList));
    list->head = NULL;

    return list;
}

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

char* add_suffix_to_file_name(char* curr_file_name, char* new_suffix, useCase use_case) {
    char* suffix_start;
    char* new_file_name;

    suffix_start = strrchr(curr_file_name, '.');

    if (use_case == OUTPUT) {
    
        if (suffix_start == NULL) {
            printf("panic! %s can't be processed as it lacks the correct suffix: \".as\"\n", curr_file_name);
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

/*
int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }

    FILE* input_file = fopen(argv[1], "r");
    FILE* output_file = fopen(argv[2], "w");

    if (!input_file) {
        fprintf(stderr, "Error: Could not open input file %s\n", argv[1]);
        return 1;
    }

    if (!output_file) {
        fprintf(stderr, "Error: Could not open output file %s\n", argv[2]);
        fclose(input_file);
        return 1;
    }

    process_macros(input_file, output_file);

    fclose(input_file);
    fclose(output_file);

    printf("Pre-processing complete. Output written to %s\n", argv[2]);

    return 0;
}
*/
