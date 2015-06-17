/*
   Copyright 2015, Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>

#define ROM_SIZE_KB 64
#define ROM_SIZE_LIMIT (ROM_SIZE_KB * 1024)
#define ROMVLOC 0x100000
#define ROMVEND 0x110000

#define SUCCESS 0

int main(int argc, char *argv[])
{
    uint32_t    romaddr;
    char*       elffile = argv[1];
    char*       romfile = argv[2];
    int         elffd;
    FILE *      romfd;
    void*       elfmmap;
    int         err;
    int         exitCode = SUCCESS;
    struct stat fstats;

    if(argc != 3)
    {
        fprintf(stderr, "argv[1] should be a path to an ELF file\n");
        fprintf(stderr, "argv[2] should be a path to an output ROM file\n");
        exit(-1);
    }

    elffd = open(elffile, O_RDONLY);
    if(elffd < 0)
    {
        perror("open elffile");
        exit(-1);
    }

    romfd = fopen(romfile, "w");
    if(elffd < 0)
    {
        perror("open romfile");
        exitCode = -1;
        goto deferDone;
    }

    err = fstat(elffd, &fstats);
    if(err < 0)
    {
        perror("stat elffd");
        exitCode = -1;
        goto deferDone;
    }

    elfmmap = mmap(NULL, fstats.st_size, PROT_READ, MAP_PRIVATE, elffd, 0);
    if(elfmmap == MAP_FAILED)
    {
        perror("mmap");
        exitCode = -1;
        goto deferDone;
    }

    Elf32_Ehdr* ehdr = elfmmap;
    size_t pnum;
    
    if(fstats.st_size < sizeof(*ehdr))
    {
        fprintf(stderr, "ELF File too small\n");
        exitCode = -1;
        goto deferUnmap;
    }
    
    if(ehdr->e_ident[EI_MAG0] != 0x7f ||
       ehdr->e_ident[EI_MAG1] != 'E' ||
       ehdr->e_ident[EI_MAG2] != 'L' ||
       ehdr->e_ident[EI_MAG3] != 'F' ||
       ehdr->e_ident[EI_CLASS] != ELFCLASS32 ||
       ehdr->e_ident[EI_DATA] != ELFDATA2MSB
        )
    {
        fprintf(stderr, "Invalid ELF\n");
        exitCode = -1;
        goto deferUnmap;
    }

    if(fstats.st_size < sizeof(Elf32_Phdr)*ntohs(ehdr->e_phnum) + ntohl(ehdr->e_phoff))
    {
        fprintf(stderr, "ELF File too small\n");
        exitCode = -1;
        goto deferUnmap;
    }

    fprintf(romfd, "@0000_0000\n");

    for(romaddr = 0; romaddr < ROM_SIZE_LIMIT; romaddr += 4)
    {
        for(pnum=0; pnum<ntohs(ehdr->e_phnum); pnum++)
        {
            Elf32_Phdr* phdr = (Elf32_Phdr *)(elfmmap + (ntohl(ehdr->e_phoff) + sizeof(Elf32_Phdr)*pnum));

            uint32_t paddr  = ntohl(phdr->p_paddr);
            uint32_t vaddr  = ntohl(phdr->p_vaddr);
            uint32_t filesz = ntohl(phdr->p_filesz);
            uint32_t memsz  = ntohl(phdr->p_memsz);
            uint32_t offset = ntohl(phdr->p_offset);

            if((paddr - ROMVLOC) != romaddr)
                continue;

            printf("PHDR paddr@%zu paddr=0x%08x vaddr=0x%08x filesz=%u memsz=%u\n",pnum, paddr, vaddr, filesz, memsz);

            {
                uint8_t *ptr;
                uint32_t i;

                ptr = (uint8_t *) elfmmap;
                ptr = &ptr[offset];
                for(i = 0; i < filesz; i += 4, ptr += 4, romaddr += 4)
                    fprintf(romfd, "%02x%02x_%02x%02x\n", ptr[0], ptr[1], ptr[2], ptr[3]);
            }
        }

        if(pnum >= ntohs(ehdr->e_phnum))
            fprintf(romfd, "%02x%02x_%02x%02x\n", 0, 0, 0, 0);
    }

    if(romaddr > ROM_SIZE_LIMIT)
        fprintf(stderr, "ROM may exceed size limit of %dk\n", ROM_SIZE_KB);

deferUnmap:
    munmap(elfmmap, fstats.st_size);

deferDone:
    close(elffd);
    fclose(romfd);

    if (exitCode != SUCCESS)
        exit(exitCode);

    return 0;
}
