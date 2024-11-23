#include <vector>
#include <string.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "program.hpp"

void Program::Initialize(char* programPath, PagerType type) {
    this->pagerType = type;
    this->file = std::ifstream(programPath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open ELF file.");
    }
}

void Program::MapSectionsFromElf()
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
            this->sections.emplace_back(SectionType::TEXT, this->pagerType, file, &ph);
            std::cout << "TEXT: ";
       } else if (ph.p_flags & PF_W) {
            this->sections.emplace_back(SectionType::DATA_RW, this->pagerType, file, &ph);
            std::cout << "DATA_RW: ";
        } else if (ph.p_flags & PF_R) {
            this->sections.emplace_back(SectionType::DATA_RO, this->pagerType, file, &ph);
            std::cout << "DATA_RO: ";
        }
        std::cout << (void*)ph.p_vaddr << " " << (void*)ph.p_vaddr + ph.p_memsz << std::endl;
    }   
}

void Program::PrepStack(char **argv, char **envp, Elf64_auxv_t *auxv)
{
    this->sections.emplace_back(argv, envp, auxv);
}

bool Program::FindSectionAndAllocNewPage(uint64_t faultAddr)
{
    for (auto& section: this->sections) {
        const auto ph = section.ph;
        if (section.sectionType == SectionType::STACK) {
            continue;
        }
        if (faultAddr < ph->p_vaddr || (uint64_t) ph->p_vaddr + ph->p_memsz <= faultAddr) {
             continue;
        }

        section.MapAdditionalPage(this->file, faultAddr);

        if (pagerType == PagerType::HPAGER) {
            if (faultAddr + 4096 < (uint64_t) ph->p_vaddr + ph->p_memsz) {
                section.MapAdditionalPage(this->file, faultAddr + 4096);
            }
        }

        return true;
    }
    return false;
}

const Section& Program::getSection(SectionType type)
{
    for (const auto& section: this->sections) {
        if (section.sectionType == type) {
            return section;
        }
    }
    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! get section failed !!!!!!!!!!!!!!!!1\n";
    return this->sections[0]; // dangerous
}
