#ifndef FILE_GENERATOR_H
#define FILE_GENERATOR_H

#include <stdio.h>
#include "transletor.h"


void create_output_files(Shed* shed);
void create_object_file(Shed* shed, WordList* code_list, WordList* data_list);
void create_extern_and_entry_files(Shed* shed);

#endif