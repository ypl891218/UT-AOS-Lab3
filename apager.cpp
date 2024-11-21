#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <elf.h>    // Use system-provided ELF header definitions
#include <unistd.h>
#include <sys/auxv.h>
#include <assert.h>

#include "section.hpp"
#include "program.hpp"
#include "auxv_util.hpp"

static Elf64_Ehdr read_elf_header(std::ifstream &file)
{
    Elf64_Ehdr header;
    file.read(reinterpret_cast<char *>(&header), sizeof(header));
    return header;
}

static void clearRegisterAndJump(void* entry_point, void* new_stack) {
    asm volatile (
        // Set the stack pointer (rsp) to the new address
        "mov %0, %%rsp\n\t"

        // Push the entry point address onto the new stack
        "push %1\n\t"

        // Clear all general-purpose registers
        "xor %%rax, %%rax\n\t"
        "xor %%rbx, %%rbx\n\t"
        "xor %%rcx, %%rcx\n\t"
        "xor %%rdx, %%rdx\n\t"
        "xor %%rsi, %%rsi\n\t"
        "xor %%rdi, %%rdi\n\t"
        "xor %%r8, %%r8\n\t"
        "xor %%r9, %%r9\n\t"
        "xor %%r10, %%r10\n\t"
        "xor %%r11, %%r11\n\t"
        "xor %%r12, %%r12\n\t"
        "xor %%r13, %%r13\n\t"
        "xor %%r14, %%r14\n\t"
        "xor %%r15, %%r15\n\t"

        // Transfer control to the entry point via the stack
        "ret\n\t"
        :
        : "r"(new_stack), "r"(entry_point) // Inputs: new_stack in a register, entry_point in a register
        : "memory" // Clobbers memory to indicate potential side effects
    );
}


extern char **environ;

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <ELF file>\n";
        return 1;
    }

    Elf64_auxv_t* auxv;

    try {
        void* top_of_stack = (void*)(argv-1);

        std::ifstream file(argv[1], std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open ELF file.");
        }

        Program program(PagerType::APAGER);

        // Parse ELF header
        program.elfHeader = read_elf_header(file);

        // Validate ELF magic number
        if (memcmp(program.elfHeader.e_ident, ELFMAG, SELFMAG) != 0) {
            throw std::runtime_error("Not a valid ELF file.");
        }

        program.MapSectionsFromElf(file);
        std::cout << "ELF file successfully loaded into memory.\n";

        auxv = GetAuxvCopy(top_of_stack, argc, argv);
	    std::cout << "Got the auxv copy\n";
	    ModifyAuxvForChild(program, auxv, argv[1]); // suppose argv[1] is the child's program name
        std::cout << "Aux vector is now prepared\n";

        program.PrepStack(&argv[1], environ, auxv);
        std::cout << "Stack is now prepared\n";

        StackSanityCheck(program.stack.base_addr, argc-1, &argv[1]);
        assert((uint64_t)program.text.addr <= (uint64_t)program.elfHeader.e_entry);
        assert((uint64_t)program.elfHeader.e_entry < (uint64_t)program.text.addr + program.text.len);
        std::cout << "e_entry is indeed in TEXT section\n";

        clearRegisterAndJump((void*)program.elfHeader.e_entry, program.stack.base_addr);

        free(auxv);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        free(auxv);
        return 1;
    } 

    return 0;
}