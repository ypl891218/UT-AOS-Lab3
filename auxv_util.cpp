#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/auxv.h>

#include "auxv_util.hpp"

Elf64_auxv_t* GetAuxvCopy(void* top_of_stack, int argc, char **argv)
{
    uint64_t* stack = (uint64_t*)top_of_stack;

	printf("----- stack check -----\n");

	assert(((uint64_t)stack) % 8 == 0);
	printf("top of stack is 8-byte aligned\n");

	uint64_t actual_argc = *(uint64_t*)(stack++);
    std::cout << actual_argc << " " << argc << std::endl;
    assert(actual_argc == argc);

	for (int i = 0; i < argc; i++) {
		char* argp = (char*)*(char**)(stack++);
		assert(strcmp(argp, argv[i]) == 0);
		printf("arg %d: %s\n", i, argp);
	}
	assert(*(char**)(stack++) == 0);

    int envp_count = 0;
	while (*(char**)(stack++) != 0)
		envp_count++;

	printf("env count: %d\n", envp_count);

	Elf64_auxv_t* auxv_start = (Elf64_auxv_t*)stack;
	Elf64_auxv_t* auxv_null = auxv_start;
	while (auxv_null->a_type != AT_NULL) {
		auxv_null++;
	}
	printf("aux count: %lu\n", auxv_null - auxv_start);

    // Create a copy
    Elf64_auxv_t* mycopy = (Elf64_auxv_t*)malloc(sizeof(Elf64_auxv_t)*(auxv_null - auxv_start + 1));
    memcpy(mycopy, auxv_start, (auxv_null - auxv_start + 1) * sizeof(Elf64_auxv_t));
    return mycopy;
}

void ModifyAuxvForChild(Program& program, Elf64_auxv_t* auxv, char *childFileName)
{
    while (auxv->a_type != AT_NULL) {
        switch (auxv->a_type) {
            case AT_PHDR:
                auxv->a_un.a_val = (uint64_t)program.programHeaders;
                break;
            case AT_PHNUM:
                auxv->a_un.a_val = program.elfHeader.e_phnum;
                break;
            case AT_BASE:
                auxv->a_un.a_val = 0;
                break;
            case AT_ENTRY:
                auxv->a_un.a_val = program.elfHeader.e_entry;
                break;
            case AT_EXECFN:
                // here, auxv->a_un.a_val is a pointer
                strcpy((char*)(auxv->a_un.a_val), childFileName);
                break;
        }
        
        auxv++;
    }
}

void StackSanityCheck(void* top_of_stack, uint64_t argc, char** argv)
{
	printf("----- stack check -----\n");

	assert(((uint64_t)top_of_stack) % 8 == 0);
	printf("top of stack is 8-byte aligned\n");

	uint64_t* stack = (uint64_t*)top_of_stack;
	uint64_t actual_argc = *(stack++);
	printf("argc: %lu\n", actual_argc);
	assert(actual_argc == argc);

	for (int i = 0; i < argc; i++) {
		char* argp = (char*)*(stack++);
		assert(strcmp(argp, argv[i]) == 0);
		printf("arg %d: %s\n", i, argp);
	}
	// Argument list ends with null pointer
	assert(*(stack++) == 0);

	int envp_count = 0;
	while (*(stack++) != 0)
		envp_count++;

	printf("env count: %d\n", envp_count);

	Elf64_auxv_t* auxv_start = (Elf64_auxv_t*)stack;
	Elf64_auxv_t* auxv_null = auxv_start;
	while (auxv_null->a_type != AT_NULL) {
		auxv_null++;
	}
	printf("aux count: %lu\n", auxv_null - auxv_start);
	printf("----- end stack check -----\n");
}