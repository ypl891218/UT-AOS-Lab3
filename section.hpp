#ifndef AOS_LAB3_SECTION
#define AOS_LAB3_SECTION

#include <cstddef>
#include <fstream>

#include <sys/mman.h>
#include <elf.h>
#include <sys/auxv.h>

class Section {
public:
    void* addr;
    size_t len;
    void* base_addr;
    
    void MapFromElfPhdr(std::ifstream &file, const Elf64_Phdr &ph);

    void BecomeStack(char **argv, char **envp, Elf64_auxv_t *auxv);

    ~Section() {
        if (this->addr != nullptr && this->addr != MAP_FAILED) {
            munmap(this->addr, this->len);
        }
    }
};

#endif /* AOS_LAB3_SECTION */