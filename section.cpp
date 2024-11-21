#include <iostream>
#include <cstring>

#include "section.hpp"

#define STACK_SIZE (8 * 1024 * 1024) // 8MB

static int argv_count(char **argv) {
    int count = 0;
    while (argv && argv[count]) {
        count++;
    }
    return count;
}

static int envp_count(char **envp) {
    int count = 0;
    while (envp && envp[count]) {
        count++;
    }
    return count;
}

static int auxv_count(Elf64_auxv_t *auxv) {
    int count = 0;
    while (auxv && auxv[count].a_type != AT_NULL) {
        count++;
    }
    return count;
}

void Section::MapFromElfPhdr(std::ifstream &file, const Elf64_Phdr &ph) {
    this->addr = mmap(reinterpret_cast<void *>(ph.p_vaddr), ph.p_memsz,
                            PROT_READ | PROT_WRITE | PROT_EXEC,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    this->len = ph.p_memsz;
    file.seekg(ph.p_offset, std::ios::beg);
    file.read(reinterpret_cast<char *>(addr), ph.p_filesz);

    // Zero out the remaining part
    if (ph.p_memsz > ph.p_filesz) {
        std::memset(reinterpret_cast<char *>(addr) + ph.p_filesz, 0, ph.p_memsz - ph.p_filesz);
    }
    std::cout << "Address stated in program headers: " << reinterpret_cast<void *>(ph.p_vaddr) << ", reality: " << this->addr << std::endl; 
    std::cout << "Mapped with size: " << this->len << std::endl;
    std::cout << "-------------------------------------------\n";
}

/*
 +------------------+
 | NULL             | <- (Auxiliary vector terminator)
 | Auxiliary vectors|
 | NULL             | <- (Environment variables terminator)
 | Environment vars |
 | NULL             | <- (argv terminator)
 | argv[n-1]        |
 | ...              |
 | argv[0]          |
 | argc             |
 +------------------+
 */
void Section::BecomeStack(char **argv, char **envp, Elf64_auxv_t *auxv) {
    // initialize class variables
    this->len = STACK_SIZE;
    this->addr = mmap(NULL, this->len, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    this->base_addr = (char*)this->addr + this->len;

    // prepare to store auxv, env, argv
    char *stack = (char *)this->base_addr;
    char **argv_ptrs = NULL;
    char **envp_ptrs = NULL;
    Elf64_auxv_t *auxv_ptr = NULL;

    // 1. Write auxiliary vectors (auxv) at the top
    stack -= sizeof(Elf64_auxv_t) * (auxv_count(auxv) + 1); // Include space for AT_NULL
    auxv_ptr = (Elf64_auxv_t *)stack;
    memcpy(auxv_ptr, auxv, sizeof(Elf64_auxv_t) * (auxv_count(auxv) + 1));

    std::cout << "Finished copying auxv\n";

    // 2. Write environment variables (envp)
    stack -= sizeof(char *) * (envp_count(envp) + 1); // Include NULL terminator
    std::cout << "envp_count: " << envp_count(envp) << std::endl;
    envp_ptrs = (char **)stack;
    memcpy(envp_ptrs, envp, sizeof(char **) * (envp_count(envp) + 1));
    std::cout << "Finished copying envp\n";

    // 3. Write argument vectors (argv)
    stack -= sizeof(char *) * (argv_count(argv) + 1); // Include NULL terminator
    argv_ptrs = (char **)stack;
    memcpy(argv_ptrs, argv, sizeof(char **) * (argv_count(argv) + 1));
    std::cout << "Finished copying argv\n";

    // 4. Write argc
    stack -= sizeof(uint64_t);
    *((uint64_t *)stack) = argv_count(argv);

    std::cout << "argc = " << argv_count(argv) << std::endl;
    std::cout << "Finished writing argc\n";

    // 5. update base_addr
    // std::cout << "The lowest addr of stack: " << this->addr << std::endl;
    // std::cout << "The highest addr of stack: " << this->base_addr << std::endl;
    this->base_addr = (void*)stack;
    // std::cout << "The current base addr of stack: " << this->base_addr << std::endl;
}
