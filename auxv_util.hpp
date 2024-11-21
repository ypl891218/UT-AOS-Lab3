#ifndef AOS_LAB3_AUXV_UTIL
#define AOS_LAB3_AUXV_UTIL

#include <sys/auxv.h>
#include "program.hpp"

Elf64_auxv_t* GetAuxvCopy(void* top_of_stack, int argc, char **argv);
void ModifyAuxvForChild(Program &program, Elf64_auxv_t* auxv, char *childFileName);

void StackSanityCheck(void* top_of_stack, uint64_t argc, char** argv);
#endif /* AOS_LAB3_AUXV_UTIL */