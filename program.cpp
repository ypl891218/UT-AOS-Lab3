#include <vector>
#include <string.h>
#include <iostream>
#include "program.hpp"

void Program::MapSectionsFromElf(std::ifstream &file)
{
    this->programHeaders = (Elf64_Phdr *)malloc(elfHeader.e_phnum * sizeof(Elf64_Phdr));
    file.seekg(elfHeader.e_phoff, std::ios::beg);
    for (int i = 0; i < elfHeader.e_phnum; ++i) {
        file.read(reinterpret_cast<char *>(&programHeaders[i]), sizeof(Elf64_Phdr));
    }

    for (int i = 0; i < elfHeader.e_phnum; ++i) {
        // Don't make it const auto &ph, because we need to modify its p_vaddr
        auto& ph = this->programHeaders[i];

        if (ph.p_type != PT_LOAD) { // Only process loadable segments 
            continue;
        }
        if (ph.p_flags & PF_X) {
            this->text.MapFromElfPhdr(file, &ph);
            this->elfHeader.e_entry = (uint64_t)text.addr + (this->elfHeader.e_entry - ph.p_vaddr);
        } else if (ph.p_flags & PF_W) {
            if (ph.p_filesz < ph.p_memsz) {
                this->bss.MapFromElfPhdr(file, &ph);
            } else {
                this->data_rw.MapFromElfPhdr(file, &ph);
            }
        } else if (ph.p_flags & PF_R) {
             this->data_ro.MapFromElfPhdr(file, &ph);
        }
    }   
}

void Program::PrepStack(char **argv, char **envp, Elf64_auxv_t *auxv) {
    this->stack.BecomeStack(argv, envp, auxv);
}
