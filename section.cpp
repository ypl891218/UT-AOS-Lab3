#include <iostream>
#include <cstring>
#include <unistd.h>
#include <assert.h>
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

Section::Section(SectionType sectionType, PagerType pagerType, std::ifstream &file, Elf64_Phdr *ph):
        sectionType(sectionType), pagerType(pagerType), ph(ph) {
    size_t page_size = sysconf(_SC_PAGESIZE);
    uint64_t aligned_start = ph->p_vaddr & ~(page_size - 1);
    uint64_t mapping_size = ph->p_memsz + (ph->p_vaddr - aligned_start);

    if (this->pagerType == PagerType::APAGER) {
        this->addr = mmap(reinterpret_cast<void *>(aligned_start), mapping_size,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        this->len = mapping_size;
        file.seekg(ph->p_offset, std::ios::beg);
        file.read(reinterpret_cast<char *>(ph->p_vaddr), ph->p_filesz);

        if (ph->p_filesz < ph->p_memsz) {
            std::cout << "Memset for the part without file backed\n";
            std::memset(reinterpret_cast<char *>(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
        }

        std::cout << "Address in phdr: " << reinterpret_cast<void *>(ph->p_vaddr) << ", reality: " << this->addr << std::endl; 
        std::cout << "Mapped size in phdr: " << ph->p_memsz << ", reality: " << this->len << std::endl;
    } /* else if (this->pagerType == PagerType::DPAGER) {
        if (this->sectionType != SectionType::TEXT) {
            return;
        }       
        this->addr = mmap(reinterpret_cast<void *>(aligned_start), page_size,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        this->len = mapping_size;
        file.seekg(ph->p_offset, std::ios::beg);
        file.read(reinterpret_cast<char *>(ph->p_vaddr), page_size);
    } */ 
    else {
        // only allocates and map for size = ph->p_filesz
        uint64_t logical_length = ph->p_filesz + (ph->p_vaddr - aligned_start);
        uint64_t padded_length = (logical_length + page_size - 1) - (logical_length + page_size - 1) % page_size;        

        this->addr = mmap(reinterpret_cast<void *>(aligned_start), padded_length,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        this->len = padded_length;
        file.seekg(ph->p_offset, std::ios::beg);
        file.read(reinterpret_cast<char *>(ph->p_vaddr), ph->p_filesz);
    }
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
Section::Section(char **argv, char **envp, Elf64_auxv_t *auxv): sectionType(SectionType::STACK) {
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
    std::cout << "The lowest addr of stack: " << this->addr << std::endl;
    std::cout << "The highest addr of stack: " << this->base_addr << std::endl;
    this->base_addr = (void*)stack;
    std::cout << "The current base addr of stack: " << this->base_addr << std::endl;
}

void Section::MapAdditionalPage(std::ifstream &file, uint64_t faultAddr)
{
    if (this->pagerType == PagerType::APAGER) {
        std::cout << "apager should not call Expand()\n";
        return;
    }

    // Only read the addr range: [ph->p_vaddr , ph->p_vaddr  + ph->p_filesz)
    //               file range: [ph->p_offset, ph->p_offset + ph->p_filesz)
    size_t page_size = sysconf(_SC_PAGESIZE);
    uint64_t aligned_start = faultAddr & ~(page_size - 1);

    // My memory range is [aligned_start, aligned_start + mmap_len)
    uint64_t mmap_len = std::min(page_size, this->ph->p_vaddr + this->ph->p_memsz - aligned_start);
    uint64_t ret = (uint64_t)mmap(reinterpret_cast<void *>(aligned_start),
            mmap_len, PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ret != aligned_start) {
        munmap((void*)ret, mmap_len);
        return;
    }
    //assert(ret == aligned_start);
    memset((void*)aligned_start, 0, mmap_len);

/*
    } else {
        if (ph->p_vaddr <= aligned_start && aligned_start + mmap_len < ph->p_vaddr + ph->p_filesz) {
            file.seekg(ph->p_offset + (aligned_start - ph->p_vaddr), std::ios::beg);
            file.read(reinterpret_cast<char *>(aligned_start), mmap_len);
        } else if (aligned_start <= ph->p_vaddr && ph->p_vaddr + ph->p_filesz < aligned_start + mmap_len) {
            file.seekg(ph->p_offset, std::ios::beg);
            file.read(reinterpret_cast<char *>(ph->p_vaddr), ph->p_filesz);
        } else if (ph->p_vaddr <= aligned_start && aligned_start < ph->p_vaddr + ph->p_filesz) {
            file.seekg(ph->p_offset + (aligned_start - ph->p_vaddr), std::ios::beg);
            file.read(reinterpret_cast<char *>(aligned_start), ph->p_vaddr + ph->p_filesz - aligned_start);
        } else if (ph->p_vaddr <= aligned_start + mmap_len && aligned_start + mmap_len < ph->p_vaddr + ph->p_filesz) {
            file.seekg(ph->p_offset, std::ios::beg);
            file.read(reinterpret_cast<char *>(ph->p_vaddr), aligned_start + mmap_len - ph->p_vaddr);
        }
    }
*/
}
