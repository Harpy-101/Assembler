#ifndef PANIC_H
#define PANIC_H

#include <stdio.h>
#include "lexer.h"
#include "ast.h"

void memory_allocation_failure();
void too_many_arguments(Token* token, int arg_count);
void ASTNode_error(ASTNode* node);

#endif