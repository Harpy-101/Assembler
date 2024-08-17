#include "panic.h"
#include "ast.h"

void memory_allocation_failure() {
    printf("\033[31mpanic!\033[0m memory alocation failed.\n");
    exit(EXIT_FAILURE);
}

void too_many_arguments(Token* token, int arg_count) {
    printf("\033[31mpanic!\033[0m at line: %d an instruction of type: %s can only hold %d argumentas\n", token->line, token->val, arg_count);
}

void ASTNode_error(ASTNode* node) {
    printf("\033[31mpanic!\033[0m at line: %d: %s\n", node->line, node->status.error_code);
    exit(EXIT_FAILURE);
}