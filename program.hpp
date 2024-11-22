#ifndef AOS_LAB3_PROGRAM
#define AOS_LAB3_PROGRAM
#include <elf.h>
#include <vector>
#include "section.hpp"

class Program {
public:
//    Section stack;
//    Section bss;
//    Section data_rw;
//    Section data_ro;
//    Section text;

    std::vector<Section> sections;
    Elf64_Ehdr elfHeader;
    Elf64_Phdr *programHeaders;

    PagerType pagerType;

    std::ifstream file;

    void MapSectionsFromElf();
    void PrepStack(char **argv, char **envp, Elf64_auxv_t *auxv);
    void Initialize(char *programPath, PagerType);
    bool IsWithinSections(uint64_t faultAddr);
    bool FindSectionAndAllocNewPage(uint64_t faultAddr);

    const Section& getSection(SectionType type);

    ~Program() {
        free(programHeaders);
    }
};

#endif /* AOS_LAB3_PROGRAM */
