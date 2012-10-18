/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002 - 2012   Klaus Rudolph & other
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 *
 *  $Id$
 */

#ifndef _MSC_VER
    // only, if BFD lib is used
#   include "config.h"
#   include "bfd.h"
#endif

#include <string>
#include <map>

#include "avrdevice_impl.h"

#include "avrreadelf.h"

#ifdef _MSC_VER
// Stolen from http://developers.sun.com/solaris/articles/elf.html

#define EI_NIDENT     16
// Segment types
#define PT_NULL             0
#define PT_LOAD             1
#define PT_NOTE             4
#define PT_SHLIB            5
#define PT_PHDR             6
// Segment flags
#define PF_X                 1
#define PF_W                 2
#define PF_R                 4

typedef uint32_t  Elf32_Addr;
typedef uint16_t  Elf32_Half;
typedef uint32_t  Elf32_Off;
typedef int32_t   Elf32_Sword;
typedef uint32_t  Elf32_Word;

typedef struct {
    unsigned char e_ident[EI_NIDENT];    /* ident bytes */
    Elf32_Half e_type;                   /* file type */
    Elf32_Half e_machine;                /* target machine */
    Elf32_Word e_version;                /* file version */
    Elf32_Addr e_entry;                  /* start address */
    Elf32_Off e_phoff;                   /* phdr file offset */
    Elf32_Off e_shoff;                   /* shdr file offset */
    Elf32_Word e_flags;                  /* file flags */
    Elf32_Half e_ehsize;                 /* sizeof ehdr */
    Elf32_Half e_phentsize;              /* sizeof phdr */
    Elf32_Half e_phnum;                  /* number phdrs */
    Elf32_Half e_shentsize;              /* sizeof shdr */
    Elf32_Half e_shnum;                  /* number shdrs */
    Elf32_Half e_shstrndx;               /* shdr string index */
} Elf32_Ehdr;
// Segment header
typedef struct {
    Elf32_Word p_type;   /* entry type */
    Elf32_Off p_offset;  /* file offset */
    Elf32_Addr p_vaddr;  /* virtual address */
    Elf32_Addr p_paddr;  /* physical address */
    Elf32_Word p_filesz; /* file size */
    Elf32_Word p_memsz;  /* memory size */
    Elf32_Word p_flags;  /* entry flags */
    Elf32_Word p_align;  /* memory/file alignment */
} Elf32_Phdr;

void ELFLoad(const AvrDevice * core) {
    FILE * f = fopen(core->actualFilename.c_str(), "rb");
    if(f == NULL)
        avr_error("Could not open file: %s", core->actualFilename.c_str());

    Elf32_Ehdr header;
    fread(&header, sizeof(header), 1, f);
    if(header.e_ident[0] != 0x7F || header.e_ident[1] != 'E'
        || header.e_ident[2] != 'L' || header.e_ident[3] != 'F')
        avr_error("File '%s' is not an ELF file", core->actualFilename.c_str());
    // TODO: fix endianity in header
    if(header.e_machine != 83)
        avr_error("ELF file '%s' is not for Atmel AVR architecture (%d)", core->actualFilename.c_str(), header.e_machine);

    for(int i = 0; i < header.e_phnum; i++) {
        fseek(f, header.e_phoff + i * header.e_phentsize, SEEK_SET);
        Elf32_Phdr progHeader;
        fread(&progHeader, sizeof(progHeader), 1, f);
        // TODO: fix endianity
        if(progHeader.p_type != PT_LOAD)
            continue;
        if((progHeader.p_flags & PF_X ) == 0 || (progHeader.p_flags & PF_R) == 0)
            continue;  // must be readable and writable
        if(progHeader.p_vaddr >= 0x80ffff)
            continue;  // not into a Flash
        if(progHeader.p_filesz != progHeader.p_memsz) {
            avr_error("Segment sizes 0x%x and 0x%x in ELF file '%s' must be the same",
                progHeader.p_filesz, progHeader.p_memsz, core->actualFilename.c_str());
        }
        unsigned char * tmp = new unsigned char[progHeader.p_filesz];
        fseek(f, progHeader.p_offset, SEEK_SET);
        fread(tmp, progHeader.p_filesz, 1, f);

        core->Flash->WriteMem(tmp, progHeader.p_vaddr, progHeader.p_filesz);
        delete [] tmp;
    }

    fclose(f);
}

unsigned int ELFGetSignature(const char *filename) {
    return -1;
}
#endif

#ifndef _MSC_VER
void ELFLoad(const AvrDevice * core) {
    bfd *abfd;
    asection *sec;

    // open file
    bfd_init();
    abfd = bfd_openr(core->actualFilename.c_str(), NULL);

    if(abfd == NULL)
        avr_error("Could not open file: %s", core->actualFilename.c_str());

    // check format
    if(bfd_check_format(abfd, bfd_object) == FALSE)
        avr_error("File '%s' isn't a elf object", core->actualFilename.c_str());

    // reading out the symbols
    long storage_needed;
    static asymbol **symbol_table;
    long number_of_symbols;
    long i;

    storage_needed = bfd_get_symtab_upper_bound(abfd);
    if(storage_needed < 0)
        avr_error("internal error: storage_needed < 0");
    if(storage_needed > 0) {

        symbol_table = (asymbol **)malloc(storage_needed);

        number_of_symbols = bfd_canonicalize_symtab(abfd, symbol_table);
        if(number_of_symbols < 0)
            avr_error("internal error: number_of_symbols < 0");

        // over all symbols ...
        for(i = 0; i < number_of_symbols; i++) {
            // if no section data, skip
            if(!symbol_table[i]->section)
                continue;

            unsigned int lma = symbol_table[i]->section->lma;
            unsigned int vma = symbol_table[i]->section->vma;

            if(vma < 0x800000) { //range of flash space
                std::pair<unsigned int, std::string> p((symbol_table[i]->value+lma) >> 1, symbol_table[i]->name);
                core->Flash->AddSymbol(p);
            } else if(vma < 0x810000) { //range of ram
                unsigned int offset = vma - 0x800000;
                std::pair<unsigned int, std::string> p(symbol_table[i]->value + offset, symbol_table[i]->name);
                core->data->AddSymbol(p);  // not a real data container, only holding symbols!

                std::pair<unsigned int, std::string> pp(symbol_table[i]->value + lma, symbol_table[i]->name);
                core->Flash->AddSymbol(pp);
            } else if(vma < 0x820000) { // range of eeprom
                unsigned int offset = vma - 0x810000;
                std::pair<unsigned int, std::string> p(symbol_table[i]->value + offset, symbol_table[i]->name);
                core->eeprom->AddSymbol(p);
            } else if(vma < 0x820400) {
                /* fuses space starting from 0x820000, do nothing */;
            } else if(vma >= 0x830000 && vma < 0x830400) {
                /* lock bits starting from 0x830000, do nothing */;
            } else if(vma >= 0x840000 && vma < 0x840400) {
                /* signature space starting from 0x840000, do nothing */;
            } else
                avr_warning("Unknown symbol address range found! (symbol='%s', address=0x%x)", symbol_table[i]->name, vma);
        }
        free(symbol_table);
    } else
        avr_warning("Elf file '%s' has no symbol table!", core->actualFilename.c_str());

    // load program, data and - if available - eeprom, fuses and signature
    sec = abfd->sections;
    while(sec != 0) {
        // only loadable sections
        if(sec->flags & SEC_LOAD) {
            int size;
            size = sec->size;

            unsigned char *tmp = (unsigned char *)malloc(size);

            bfd_get_section_contents(abfd, sec, tmp, 0, size);

            if(sec->vma < 0x810000) {
                // read program, space below 0x810000
                core->Flash->WriteMem(tmp, sec->lma, size);
            } else if(sec->vma >= 0x810000 && sec->vma < 0x820000) {
                // read eeprom content, if available, space from 0x810000 to 0x820000
                unsigned int offset = sec->vma - 0x810000;
                core->eeprom->WriteMem(tmp, offset, size);
            } else if(sec->vma >= 0x820000 && sec->vma < 0x820400) {
                // read fuses, if available, space from 0x820000 to 0x820400
                if(!core->fuses->LoadFuses(tmp, size)) {
                    free(tmp); // free memory before abort program
                    avr_error("wrong byte size of fuses");
                }
            } else if(sec->vma >= 0x830000 && sec->vma < 0x830400) {
                // read lock bits, if available, space from 0x830000 to 0x830400
                if(!core->lockbits->LoadLockBits(tmp, size)) {
                    free(tmp); // free memory before abort program
                    avr_error("wrong byte size of lock bits");
                }
            } else if(sec->vma >= 0x840000 && sec->vma < 0x840400) {
                // read and check signature, if available, space from 0x840000 to 0x840400
                if(size != 3) {
                    free(tmp); // free memory before abort program
                    avr_error("wrong device signature size in elf file, expected=3, given=%d", size);
                } else {
                    unsigned int sig = (((tmp[2] << 8) + tmp[1]) << 8) + tmp[0];
                    if(core->devSignature != -1 && sig != core->devSignature) {
                        free(tmp); // free memory before abort program
                        avr_error("wrong device signature, expected=0x%x, given=0x%x", core->devSignature, sig);
                    }
                }
            }

            // free allocated space
            free(tmp);
        }

        sec = sec->next;
    }

    bfd_close(abfd);
}

unsigned int ELFGetSignature(const char *filename) {
    bfd *abfd;
    asection *sec;
    unsigned int signature = -1;

    bfd_init();
    abfd = bfd_openr(filename, NULL);

    if((abfd != NULL) && (bfd_check_format(abfd, bfd_object) == TRUE)) {
        sec = abfd->sections;
        while(sec != 0) {
            if(sec->flags & SEC_LOAD) {
                int size = sec->size;
                unsigned char *tmp = (unsigned char *)malloc(size);

                bfd_get_section_contents(abfd, sec, tmp, 0, size);

                if(sec->vma >= 0x840000 && sec->vma < 0x840400) {
                    // read and check signature, if available, space from 0x840000 to 0x840400
                    if(size != 3) {
                        free(tmp); // free memory before abort program
                        avr_error("wrong device signature size in elf file, expected=3, given=%d", size);
                    } else
                        signature = (((tmp[2] << 8) + tmp[1]) << 8) + tmp[0];
                }

                free(tmp);
            }

            sec = sec->next;
        }

        bfd_close(abfd);
    }

    return signature;
}
#endif

// EOF
