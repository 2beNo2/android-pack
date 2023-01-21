//
// Created by huangchen on 2023/1/21.
//

#ifndef ANDROIDPACK2_DLFCN_COMPAT_H
#define ANDROIDPACK2_DLFCN_COMPAT_H

#include <stdint.h>
#include <elf.h>
#include <link.h>
#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    ElfW(Addr)  base_addr;
    ElfW(Addr)  bias_addr;
    ElfW(Ehdr)  *ehdr;
    ElfW(Phdr)  *phdr;
    ElfW(Dyn)   *dynamic;
    ElfW(Word)  dynamic_sz;

    const char *dynstr_tab; //.dynstr (string-table)
    ElfW(Sym)  *dynsym_tab; //.dynsym (symbol-index to string-table's offset)
    size_t      dynsym_ent;
    size_t      dynsym_count;

    int         is_use_rela;
    ElfW(Addr)  relplt; //.rel.plt or .rela.plt
    ElfW(Word)  relplt_sz;
    ElfW(Addr)  reldyn; //.rel.dyn or .rela.dyn
    ElfW(Word)  reldyn_sz;
    ElfW(Addr)  relandroid; //android compressed rel or rela
    ElfW(Word)  relandroid_sz;

    //ELF hash
    ElfW(Addr)  hash;
    uint32_t    hash_bucket_cnt;
    uint32_t    hash_chain_cnt;
    uint32_t    *hash_bucket;
    uint32_t    *hash_chain;

    //Elf GNU hash
    ElfW(Addr)  gnu_hash;
    uint32_t    gnu_bucket_cnt;
    uint32_t    symoffset;
    uint32_t    bloom_sz;
    uint32_t    bloom_shift;
    ElfW(Addr)  *bloom;
    uint32_t    *gnu_bucket;
    uint32_t    *gnu_chain;

    ElfW(Addr)  re_addr; //save symbol vaddr
} ch_elf_t;

JNIHIDE void *dlopen_compat(const char *lib_name, int flags);
JNIHIDE void *dlsym_compat(void *handle, const char *symbol_name);
JNIHIDE int dlclose_compat(void *handle);
JNIHIDE const char *dlerror_compat();

#ifdef __cplusplus
}
#endif

#endif //ANDROIDPACK2_DLFCN_COMPAT_H
