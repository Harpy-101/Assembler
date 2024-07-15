#include "panic.h"


void memory_allocation_failure() {
    printf("\033[31mpanic!\033[0m failure to allocate memory.\n");
}
