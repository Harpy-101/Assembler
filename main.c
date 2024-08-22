#include "file_generator.h"
#include "lexer.h"
#include "preprocessor.h"
#include "symbol_table.h"
#include "ast.h"
#include "transletor.h"
#include <stdio.h>

/*
int main(int argc, char *argv[]) {
 
    char line_content[MAX_LINE_LENGTH];
    Token* tokens;
    int line_number = 0;

    if (argc < 2) {
        printf("No input file detected. Stoping the program.\n");
    }

    else {
        int i;
        for (i = 1; i < argc; i++) {
            char* file_name  = add_suffix_to_file_name(argv[i], ".as", INPUT);
            FILE* input_file = fopen(file_name, "r");
            FILE* output_file; 
            int macro_error_detected = 0;
            MacroList* macro_list;
            char*output_file_name;

            if (!input_file) {
                printf("panic! unable to open %s\n", file_name);
                return EXIT_FAILURE;
            }

            output_file_name = add_suffix_to_file_name(file_name, ".am", OUTPUT);
            output_file = fopen(output_file_name, "w");
            if (output_file == NULL) {
                printf("panic! unable to create \".am\" file\n");
                return EXIT_FAILURE;
            }

            macro_list = create_macro_list();

            macro_error_detected = process_file(input_file, output_file, macro_list);
            if (macro_error_detected) {
                clear_macro_list(macro_list);
                continue;
            }
            else {
                fopen(output_file_name, "r");
                Shed* shed = malloc(sizeof(Shed));
                int token_creation_error = 0, ast_creation_error = 0, transletion_error = 0;
                shed->symbol_table = create_hash_table(INITIAL_HASH_TABLE_SIZE);
                shed->directive_table = create_directive_table(INITIAL_HASH_TABLE_SIZE);
                shed->node_list = create_node_list();
                shed->code_list = create_word_list();
                shed->data_list = create_word_list();
                shed->directive_list = create_directive_list();
                shed->unresolved_list = create_unresolved_label_list();
                shed->file_name = file_name;
                line_number++;
                while (fgets(line_content, sizeof(line_content), output_file)) {
                    tokens = tokenize(line_content, line_number, macro_list, &token_creation_error);
                    create_AST_node(tokens, shed->node_list, shed->directive_table);
                }
                translate(shed, shed->code_list, &transletion_error);
                    
                if (token_creation_error == 0 && ast_creation_error == 0 && transletion_error == 0) {
                    create_output_files(shed);
                }
                    
            }
        }
    }


}
*/

int main(int argc, char *argv[]) {
    char line_content[MAX_LINE_LENGTH];
    int line_number = 0;
    int i;

    if (argc < 2) {
        printf("No input file detected. Stopping the program.\n");
        return EXIT_FAILURE;
    }

    for ( i = 1; i < argc; i++) {
        char* file_name = add_suffix_to_file_name(argv[i], ".as", INPUT);
        FILE* input_file = fopen(file_name, "r");
        char* output_file_name;
        FILE* output_file;
        MacroList* macro_list;
        int macro_error_detected;
        Shed* shed = malloc(sizeof(Shed));
        int token_creation_error = 0, ast_creation_error = 0, translation_error = 0;
        if (!input_file) {
            printf("panic! unable to open %s\n", file_name);
            free(file_name);
            continue;
        }

        output_file_name = add_suffix_to_file_name(file_name, ".am", OUTPUT);
        output_file = fopen(output_file_name, "w");
        if (output_file == NULL) {
            printf("panic! unable to create \".am\" file\n");
            fclose(input_file);
            free(file_name);
            free(output_file_name);
            continue;
        }

        macro_list = create_macro_list();
        macro_error_detected = process_file(input_file, output_file, macro_list);
        fclose(input_file);
        fclose(output_file);

        if (macro_error_detected) {
            clear_macro_list(macro_list);
            free(file_name);
            free(output_file_name);
            continue;
        }

        output_file = fopen(output_file_name, "r");
        if (output_file == NULL) {
            printf("panic! unable to open %s for reading\n", output_file_name);
            clear_macro_list(macro_list);
            free(file_name);
            free(output_file_name);
            continue;
        }

        shed->symbol_table = create_hash_table(INITIAL_HASH_TABLE_SIZE);
        shed->directive_table = create_directive_table(INITIAL_HASH_TABLE_SIZE);
        shed->node_list = create_node_list();
        shed->code_list = create_word_list();
        shed->data_list = create_word_list();
        shed->directive_list = create_directive_list();
        shed->unresolved_list = create_unresolved_label_list();
        shed->file_name = file_name;

        line_number = 1;

        while (fgets(line_content, sizeof(line_content), output_file)) {
            Token* tokens = tokenize(line_content, line_number, macro_list, &token_creation_error);
            create_AST_node(tokens, shed->node_list, shed->directive_table);
            line_number++;
        }

        fclose(output_file);

        if (token_creation_error == 0 && ast_creation_error == 0 && translation_error == 0) {
            translate(shed, shed->code_list, &translation_error);
            if (translation_error == 0) {
                create_output_files(shed);
            }
        }

        clear_macro_list(macro_list);
        free(shed);
        free(file_name);
        free(output_file_name);
    }

    return EXIT_SUCCESS;



}

/*
#include <stdio.h>
#include "lexer.h"
#include "symbol_table.h"


int main() {
   HashTable* map = create_hash_table(5);
   insert_symbol(map, "ayo", 5);
   Symbol* found = lookup_symbol(map, "ayo");

   unresolvedLabelRefList* unresolved_list = create_unresolved_label_list();
   add_unresolved_label("ayo", 4, unresolved_list);
   add_unresolved_label("What", 10, unresolved_list);
}

*/