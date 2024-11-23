#ifndef AOS_LAB3_SECTION
#define AOS_LAB3_SECTION

#include <cstddef>
#include <fstream>
#include <unordered_set>
#include <sys/mman.h>
#include <elf.h>
#include <sys/auxv.h>

enum class PagerType {
    APAGER,
    DPAGER,
    HPAGER,
};

enum class SectionType {
    TEXT,
    DATA_RW, // contains .data and .bss
    DATA_RO,
    STACK,
};

class Section {
public:
    void* addr;
    uint64_t len;
    void* base_addr;

    Elf64_Phdr *ph; // a pointer to a Program->programHeaders[i]
    PagerType pagerType; // given by Program

    SectionType sectionType;

    // void MapFromElfPhdr(std::ifstream &file, Elf64_Phdr *ph);
    // void BecomeStack(char **argv, char **envp, Elf64_auxv_t *auxv);

    void MapAdditionalPage(std::ifstream &file, uint64_t faultAddr);

    /* used for prediction */
    void* page_addr_first;
    void* page_addr_second;
    std::unordered_set<uint64_t> allocPageAddr;

    Section(SectionType sectionType, PagerType pagerType, std::ifstream &file, Elf64_Phdr *ph);
    Section(char **argv, char **envp, Elf64_auxv_t *auxv);

    ~Section() {
        std::cout << "destructor of section\n";
    }
};

#endif /* AOS_LAB3_SECTION */
