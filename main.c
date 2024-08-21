#include "lexer.h"
#include "preprocessor.h"
#include "symbol_table.h"
#include "ast.h"
#include "transletor.h"

/*typedef struct {
   struct unresolvedLabelRefList* unresolved_list;
    //unresolvedLabelRefList* unresolved_list;
    struct SymbolTable* symbol_table;
    char* file_name;
    struct ASTNodeList* node_list;
    int* ic;
    int* dc;
    FILE *file;
} Shed;
*/
int main(int argc, char *argv[]) {
    char* filename;
    char line_content[MAX_LINE_LENGTH];
    Token* tokens;
    Token* current;
    FILE *file;
    int line_number = 0;
    SymbolTable* symbol_table = create_hash_table(INITIAL_HASH_TABLE_SIZE);
    DirectiveTable* directive_table = create_directive_table(INITIAL_HASH_TABLE_SIZE);
    DirectiveList* directive_list = create_directive_list();
    unresolvedLabelRefList* unresolved_list = create_unresolved_label_list();
    MacroList* macro_list = create_macro_list();
    ASTNodeList* node_list = create_node_list();
    WordList* code_list = create_word_list();
    WordList* data_list = create_word_list();
    Shed* shed = malloc(sizeof(Shed));
    int ic = 0;
    int dc = 0;
    shed->symbol_table = symbol_table;
    shed->unresolved_list = unresolved_list;
    shed->node_list = node_list;
    shed->dc = &dc;
    shed->ic = &ic;
    shed->directive_table = directive_table;
    shed->directive_list = directive_list;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    filename = argv[1];
    file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    shed->file_name = filename;

    while (fgets(line_content, sizeof(line_content), file)) {
        line_number++;
        tokens = tokenize(line_content, line_number,filename, macro_list);

        current = tokens;
        printf("=============== Line number: %d ===============\n", line_number);
        while (current && current->type != TOKEN_EOF) {
            printf("Token Type: %d, Value: \"%s\", Line: %d, Mode: %d\n",
                   current->type, current->val, current->line, current->mode);
            current++;
        }
        /*
        print_symbol_table(symbol_table);
        print_unresolved_list(unresolved_list);
        */
        /*free(tokens); */
        printf("^^^^^^^^^^^^^^^ Testion ASTNodes: ^^^^^^^^^^^^^^\n");
        create_AST_node(tokens, node_list, directive_table);
    }
        /*printf("$$$$$$$$$$$$$$$ Symbol table values $$$$$$$$$$$$$$$\n");
        print_symbol_table(symbol_table);
        printf("$$$$$$$$$$$$$$$ Unresolved list values values BEFORE $$$$$$$$$$$$$$$\n");
        print_unresolved_list(unresolved_list);

        resolve_unresolved_list(symbol_table, unresolved_list);
        printf("$$$$$$$$$$$$$$$ Unresolved list values values AFTER $$$$$$$$$$$$$$$\n");
        print_unresolved_list(unresolved_list);
    */
        printf("^^^^^^^^^^^^^^^ Printing ASTNode list: ^^^^^^^^^^^^^^\n"); 
        print_node_list(node_list);
        printf("\n ==================== Words ===================\n");
        translate(shed, code_list, data_list);
        stich_both_lists(code_list, data_list);

        printf("====================== After address patching ===================\n");
        resolve_unresolved_list(shed); 
        print_word_list(code_list);
        print_word_list(data_list);

        printf("====================== Symbol table values ======================\n");
        print_symbol_table(shed->symbol_table);

        printf("==================== Directive_list ========================\n");
        print_directive_list(directive_list);

    fclose(file);
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