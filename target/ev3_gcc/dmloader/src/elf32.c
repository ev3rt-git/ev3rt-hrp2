/*
 * elf32.c
 *
 *  Created on: Oct 1, 2014
 *      Author: liyixiao
 */

#include <kernel.h>
#include <t_syslog.h>
#include <string.h>
#include "elf32.h"

/**
 * Structures about ELF format taken from the System V Application Binary Interface.
 */

typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;

#define EI_NIDENT 16

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off  sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct {
    Elf32_Word      st_name;
    Elf32_Addr      st_value;
    Elf32_Word      st_size;
    unsigned char   st_info;
    unsigned char   st_other;
    Elf32_Half      st_shndx;
} Elf32_Sym;

typedef struct {
    Elf32_Word  p_type;
    Elf32_Off   p_offset;
    Elf32_Addr  p_vaddr;
    Elf32_Addr  p_paddr;
    Elf32_Word  p_filesz;
    Elf32_Word  p_memsz;
    Elf32_Word  p_flags;
    Elf32_Word  p_align;
} Elf32_Phdr;

typedef struct {
    Elf32_Addr  r_offset;
    Elf32_Word  r_info;
} Elf32_Rel;

typedef struct {
    Elf32_Addr  r_offset;
    Elf32_Word  r_info;
    Elf32_Sword r_addend;
} Elf32_Rela;

typedef unsigned char Elf32_R_Type;

enum SpecialSection {
    SECTION_BSS,
    SECTION_DATA,
    SECTION_GOT,
    SECTION_RODATA,
    SECTION_TEXT,
    SECTION_UNKNOWN
};

enum SectionType {
    SHT_NULL     = 0,
    SHT_PROGBITS = 1,
    SHT_SYMTAB   = 2,
    SHT_STRTAB   = 3,
    SHT_RELA     = 4,
    SHT_HASH     = 5,
    SHT_DYNAMIC  = 6,
    SHT_NOTE     = 7,
    SHT_NOBITS   = 8,
    SHT_REL      = 9,
};

enum SegmentType {
    PT_NULL = 0,
    PT_LOAD = 1
};

enum SegmentPermission {
    PF_X = 0x1,
    PF_W = 0x2,
    PF_R = 0x4
};

enum SymbolType {
	STT_OBJECT = 1,
    STT_FUNC   = 2,
};

#define ELF32_R_SYM(i)  ((i)>>8)
#define ELF32_R_TYPE(i)   ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

#define ELF32_ST_BIND(i)   ((i)>>4)
#define ELF32_ST_TYPE(i)   ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf)

/**
 * Utility functions
 */

static inline
const Elf32_Shdr* ELF32_SHDR(const Elf32_Ehdr *ehdr, uint32_t idx) {
    assert(idx < ehdr->e_shnum);
    return (const Elf32_Shdr*)((const uint8_t *)ehdr + ehdr->e_shoff + ehdr->e_shentsize * idx);
}

static inline
const Elf32_Phdr* ELF32_PHDR(const Elf32_Ehdr *ehdr, uint32_t idx) {
    assert(idx < ehdr->e_phnum);
    return (const Elf32_Phdr*)((const uint8_t *)ehdr + ehdr->e_phoff + ehdr->e_phentsize * idx);
}

static inline
const Elf32_Rel* ELF32_REL(const Elf32_Ehdr *ehdr, const Elf32_Shdr *shdr, uint32_t idx) {
    assert(idx < shdr->sh_size / shdr->sh_entsize);
    return (const Elf32_Rel*)((const uint8_t *)ehdr + shdr->sh_offset + shdr->sh_entsize * idx);
}

static inline
const Elf32_Rela* ELF32_RELA(const Elf32_Ehdr *ehdr, const Elf32_Shdr *shdr, uint32_t idx) {
    assert(idx < shdr->sh_size / shdr->sh_entsize);
    return (const Elf32_Rela*)((const uint8_t *)ehdr + shdr->sh_offset + shdr->sh_entsize * idx);
}

static inline
const Elf32_Sym* ELF32_SYM(const Elf32_Ehdr *ehdr, const Elf32_Shdr *shdr, uint32_t idx) {
    assert(idx < shdr->sh_size / shdr->sh_entsize);
    return (const Elf32_Sym*)((const uint8_t *)ehdr + shdr->sh_offset + shdr->sh_entsize * idx);
}

static inline
const char* ELF32_SH_NAME(const Elf32_Ehdr *ehdr, Elf32_Word sh_name) {
    const Elf32_Shdr* shstrtab = ELF32_SHDR(ehdr, ehdr->e_shstrndx);
    assert(sh_name < shstrtab->sh_size);
    return (const char*)((const uint8_t *)ehdr + shstrtab->sh_offset + sh_name);
}

static inline
const char* ELF32_STR_NAME(const Elf32_Ehdr *ehdr, const Elf32_Shdr* strtab, Elf32_Word name) {
    assert(name < strtab->sh_size);
    return (const char*)((const uint8_t *)ehdr + strtab->sh_offset + name);
}

static inline
void *elf32_vaddr_to_paddr(elf32_ldctx_t *ctx, Elf32_Addr vaddr) {
    if(vaddr >= ctx->text_seg_vaddr && vaddr < ctx->text_seg_vaddr + ctx->text_seg_sz)
        return ctx->text_buf + (vaddr - ctx->text_seg_vaddr);
    if(vaddr >= ctx->data_seg_vaddr && vaddr < ctx->data_seg_vaddr + ctx->data_seg_sz)
        return ctx->data_buf + (vaddr - ctx->data_seg_vaddr);
//    if(vaddr >= elt->data_vaddr && vaddr < elt->data_vaddr + elt->data_size)
//        return elt->data_mem + (vaddr - elt->data_vaddr);
    syslog(LOG_ERROR, "%s(): Invalid virtual address: 0x%08x", __FUNCTION__, vaddr); // Invalid virtual address
    return NULL;
}

/**
 * Target dependent definitions for ARM architecture
 */

/**
 * ELF for the ARM® Architecture
 * ARM IHI 0044E, current through ABI release 2.09
 */

enum RelocationCode {
    R_ARM_ABS32     = 2,
    R_ARM_GLOB_DAT  = 21,
    R_ARM_JUMP_SLOT = 22,
    R_ARM_RELATIVE  = 23,
};

/**
 * dest: position to apply relocation
 * symbol: physical address of the symbol in ELF32_R_SYM
 * type: ELF32_R_TYPE
 * addend
 */
static
bool_t apply_elf32_rela(elf32_ldctx_t *ctx, const Elf32_Rela *rela, const Elf32_Sym *sym) {
    Elf32_Addr *dest = elf32_vaddr_to_paddr(ctx, rela->r_offset);

    if(dest == NULL) {
        syslog(LOG_ERROR, "%s(): Invalid virtual address for relocation.", __FUNCTION__);
        return false;
    }

    /**
     * By now, only R_ARM_RELATIVE and R_ARM_ABS32 relocations are supported.
     */
    switch(ELF32_R_TYPE(rela->r_info)) {
    case R_ARM_RELATIVE:
        // TODO: Thumb instruction not supported yet.
        *dest = (Elf32_Addr)elf32_vaddr_to_paddr(ctx, rela->r_addend);
        syslog(LOG_DEBUG, "R_ARM_RELATIVE relocation: 0x%x(p:0x%08x) => 0x%08x.", rela->r_offset, dest, *dest);
        return true;
        break;
    case R_ARM_JUMP_SLOT: // TODO: This is generated in C++ applications, check this
    case R_ARM_ABS32:
        assert(sym != NULL);
    	*dest = (Elf32_Addr)elf32_vaddr_to_paddr(ctx, sym->st_value + rela->r_addend);
        syslog(LOG_DEBUG, "R_ARM_ABS32 relocation: 0x%x(p:0x%08x) => 0x%08x.", rela->r_offset, dest, *dest);
    	return true;
    	break;
    }

    syslog(LOG_ERROR, "%s(): Unsupported relocation info %x.", rela->r_info, __FUNCTION__);
    return false;
}

/**
 * Interface implementation
 */

ER elf32_load(const void *elf32_data, uint32_t elf32_data_sz, elf32_ldctx_t *ctx) {
    const Elf32_Ehdr *ehdr = elf32_data;

    /**
     * Check magic number
     */
    if (elf32_data_sz < 4 || *(uint8_t*)elf32_data != 0x7F || memcmp(elf32_data + 1, "ELF", 3)) {
        syslog(LOG_ERROR, "%s(): ELF data doesn't have a valid magic number.", __FUNCTION__);
        return E_PAR;
    }

    /**
     * Parse program headers to locate text segment and data segment
     */
    if(ehdr->e_phoff == 0 || ehdr->e_phnum == 0) {
        syslog(LOG_ERROR, "%s(): ELF data doesn't have any program header.", __FUNCTION__);
        return E_PAR;
    }

    const Elf32_Phdr *seg_text = NULL;
    const Elf32_Phdr *seg_data = NULL;

    for(uint32_t i = 0; i < ehdr->e_phnum; ++i) {
        const Elf32_Phdr *phdr = ELF32_PHDR(ehdr, i);

        // Skip if not PT_LOAD segment
        if(phdr->p_type == PT_NULL) continue;
        if(phdr->p_type != PT_LOAD) {
            syslog(LOG_WARNING, "%s(): Unsupported segment type %d.", __FUNCTION__, phdr->p_type);
            continue;
        }

        if(phdr->p_flags & PF_X) { // Text segment
            if(seg_text != NULL) {
                syslog(LOG_ERROR, "%s(): Only one text segment is supported.", __FUNCTION__);
                return E_PAR;
            }
            seg_text = phdr;
        } else {                   // Data segment
            if(seg_data != NULL) {
                syslog(LOG_ERROR, "%s(): Only one data segment is supported.", __FUNCTION__);
                return E_PAR;
            }
            seg_data = phdr;
        }
    }

    if(seg_text == NULL || seg_data == NULL) {
        syslog(LOG_ERROR, "%s(): %s segment doesn't exist.", __FUNCTION__, seg_text == NULL ? "Text" : "Data");
        return E_PAR;
    }

    /**
     * Update context & copy segments into buffers.
     */
    if (ctx->text_bufsz < seg_text->p_memsz) {
    	 syslog(LOG_ERROR, "%s(): Text buffer size (%d bytes) is too small. %d bytes is needed.", __FUNCTION__, ctx->text_bufsz, seg_text->p_memsz);
    	 return E_NOMEM;
    }
    if (ctx->data_bufsz < seg_data->p_memsz) {
    	 syslog(LOG_ERROR, "%s(): Data buffer size (%d bytes) is too small. %d bytes is needed.", __FUNCTION__, ctx->data_bufsz, seg_data->p_memsz);
    	 return E_NOMEM;
    }
    ctx->text_seg_sz = seg_text->p_memsz;
    ctx->text_seg_vaddr = seg_text->p_vaddr;
    ctx->data_seg_sz = seg_data->p_memsz;
    ctx->data_seg_vaddr = seg_data->p_vaddr;
    if (seg_text->p_offset + seg_text->p_filesz > elf32_data_sz || seg_data->p_offset + seg_data->p_filesz > elf32_data_sz) {
    	syslog(LOG_ERROR, "%s(): ELF data may be corrupted.", __FUNCTION__);
    	return E_PAR;
    }
    memcpy(ctx->text_buf, elf32_data + seg_text->p_offset, seg_text->p_filesz);
    memset(ctx->text_buf + seg_text->p_filesz, '\0', seg_text->p_memsz - seg_text->p_filesz);
    memcpy(ctx->data_buf, elf32_data + seg_data->p_offset, seg_data->p_filesz);
    memset(ctx->data_buf + seg_data->p_filesz, '\0', seg_data->p_memsz - seg_data->p_filesz);

    /**
     * Perform relocations and locate '.symtab' and '.strtab'
     */
    const Elf32_Shdr* symtab = NULL;
    const Elf32_Shdr* strtab = NULL;
    for(uint32_t i = 0; i < ehdr->e_shnum; ++i) {
        const Elf32_Shdr* shdr = ELF32_SHDR(ehdr, i);

        // Find '.symtab'
        if(shdr->sh_type == SHT_SYMTAB && strcmp(ELF32_SH_NAME(ehdr, shdr->sh_name), ".symtab") == 0) {
            symtab = shdr;
            if(shdr->sh_link != 0) strtab = ELF32_SHDR(ehdr, shdr->sh_link);
            continue;
        }

        // TODO: SHT_RELA not supported yet.
        assert(shdr->sh_type != SHT_RELA);

        if(shdr->sh_type != SHT_REL && shdr->sh_type != SHT_RELA)
            continue;

        const Elf32_Shdr* dynsym = ELF32_SHDR(ehdr, shdr->sh_link);

        for(size_t i = 0; i < shdr->sh_size / shdr->sh_entsize; ++i) {
            Elf32_Rela rela;

            if(shdr->sh_type == SHT_REL) {
                const Elf32_Rel* rel = ELF32_REL(ehdr, shdr, i);
                rela.r_info = rel->r_info;
                rela.r_offset = rel->r_offset;
                Elf32_Sword addend = *(Elf32_Sword*)elf32_vaddr_to_paddr(ctx, rela.r_offset);
                rela.r_addend = addend;
            } else
                rela = *ELF32_RELA(ehdr, shdr, i);

            const Elf32_Sym* sym = ELF32_SYM(ehdr, dynsym, ELF32_R_SYM(rela.r_info));

            if(!apply_elf32_rela(ctx, &rela, sym)) {
                syslog(LOG_ERROR, "%s(): apply_elf32_rela() failed.", __FUNCTION__);
                return E_PAR;
            }

        }
    }
    if(symtab == NULL || strtab == NULL) {
        syslog(LOG_ERROR, "%s(): '.symtab' section doesn't exist or is corrupted.", __FUNCTION__);
        return E_PAR;
    }

    /**
     * Locate symbols
     */
    ctx->sym__module_cfg_entry_num = NULL;
    ctx->sym__module_cfg_tab = NULL;
    ctx->sym__module_pil_version = NULL;
    for(size_t i = 0; i < symtab->sh_size / symtab->sh_entsize; ++i) {
        const Elf32_Sym* sym = ELF32_SYM(ehdr, symtab, i);

        if(ELF32_ST_TYPE(sym->st_info) == STT_OBJECT) {
            if(strcmp(ELF32_STR_NAME(ehdr, strtab, sym->st_name), "_module_cfg_tab") == 0)
            	ctx->sym__module_cfg_tab = elf32_vaddr_to_paddr(ctx, sym->st_value);
            else if(strcmp(ELF32_STR_NAME(ehdr, strtab, sym->st_name), "_module_cfg_entry_num") == 0)
                ctx->sym__module_cfg_entry_num = elf32_vaddr_to_paddr(ctx, sym->st_value);
            else if(strcmp(ELF32_STR_NAME(ehdr, strtab, sym->st_name), "_module_pil_version") == 0)
            	ctx->sym__module_pil_version = elf32_vaddr_to_paddr(ctx, sym->st_value);
        }
    }
    if(ctx->sym__module_cfg_entry_num == NULL || ctx->sym__module_cfg_tab == NULL || ctx->sym__module_pil_version == NULL) {
        syslog(LOG_ERROR, "%s(): Module configurations are corrupted.", __FUNCTION__);
        return E_PAR;
    }

    return E_OK;
}
