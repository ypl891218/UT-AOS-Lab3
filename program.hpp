#ifndef AOS_LAB3_PROGRAM
#define AOS_LAB3_PROGRAM
#include <elf.h>
#include "section.hpp"

class Program {
public:
    Section stack;
    Section bss;
    Section data_rw;
    Section data_ro;
    Section text;
    Elf64_Ehdr elfHeader;
    Elf64_Phdr *programHeaders;

    PagerType pagerType;

    void MapSectionsFromElf(std::ifstream &file);
    void PrepStack(char **argv, char **envp, Elf64_auxv_t *auxv);

    Program(PagerType type): pagerType(type) {
        stack.pagerType = type;
        bss.pagerType = type;
        data_rw.pagerType = type;
        data_ro.pagerType = type;
        text.pagerType = type;
    }

    ~Program() {
        free(programHeaders);
    }
};

#endif /* AOS_LAB3_PROGRAM */
