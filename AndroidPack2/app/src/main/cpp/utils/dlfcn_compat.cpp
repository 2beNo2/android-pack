//
// Created by huangchen on 2023/1/21.
//

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <errno.h>

#include "dlfcn_compat.h"

#define MAX_LENGTH 260
static int SDK_INT = -1;


static int get_sdk_level()
{
    char sdk[MAX_LENGTH] = {0};
    if (SDK_INT > 0) {
        return SDK_INT;
    }
    __system_property_get("ro.build.version.sdk", sdk);
    SDK_INT = atoi(sdk);
    return SDK_INT;
}

static ElfW(Phdr) *ch_elf_get_segment_by_type(ch_elf_t *self, ElfW(Word) type)
{
    ElfW(Phdr) *phdr = nullptr;
    for(phdr = self->phdr; phdr < self->phdr + self->ehdr->e_phnum; phdr++){
        if(phdr->p_type == type){
            return phdr;
        }
    }
    return nullptr;
}

static ElfW(Phdr) *ch_elf_get_segment_by_type_and_offset(ch_elf_t *self, ElfW(Word) type, ElfW(Off) offset)
{
    ElfW(Phdr) *phdr = nullptr;
    for(phdr = self->phdr; phdr < self->phdr + self->ehdr->e_phnum; phdr++){
        if(phdr->p_type == type && phdr->p_offset == offset){
            return phdr;
        }
    }
    return nullptr;
}

static int ch_elf_init(ch_elf_t *self, uintptr_t base_addr)
{
    ElfW(Phdr) *phdr0         = nullptr;
    ElfW(Phdr) *dynamic_Phdr  = nullptr;
    ElfW(Dyn)  *dyn           = nullptr;
    ElfW(Dyn)  *dyn_end       = nullptr;
    uint32_t   *hash          = nullptr;

    if(0 == base_addr) return -1;

    memset(self, 0, sizeof(ch_elf_t));
    self->base_addr   = (ElfW(Addr))base_addr;
    self->ehdr        = (ElfW(Ehdr)*)base_addr;
    self->phdr        = (ElfW(Phdr)*)(base_addr + self->ehdr->e_phoff);

    //find the first load-segment with offset 0
    phdr0 = ch_elf_get_segment_by_type_and_offset(self, PT_LOAD, 0);
    if(nullptr == phdr0) return -1;

    //save load bias addr
    if(self->base_addr < phdr0->p_vaddr) return -1;
    self->bias_addr = self->base_addr - phdr0->p_vaddr;

    //find dynamic segment
    dynamic_Phdr = ch_elf_get_segment_by_type(self, PT_DYNAMIC);
    if(nullptr == dynamic_Phdr) return -1;

    //save dynamic phdr
    self->dynamic     = (ElfW(Dyn)*)(self->bias_addr + dynamic_Phdr->p_vaddr);
    self->dynamic_sz  = dynamic_Phdr->p_memsz;
    dyn     = self->dynamic;
    dyn_end = self->dynamic + (self->dynamic_sz / sizeof(ElfW(Dyn)));
    LOGD("[+] (DYNAMIC) :%p", (void *)dynamic_Phdr->p_vaddr);

    size_t dynstr_va = 0;
    size_t dynsym_va = 0;

    for(; dyn < dyn_end; dyn++){
        switch(dyn->d_tag)
        {
            case DT_NULL:
            {
                dyn = dyn_end;
                break;
            }
            case DT_STRTAB:
            {
                self->dynstr_tab = (const char *)(self->bias_addr + dyn->d_un.d_ptr);
                dynstr_va = dyn->d_un.d_ptr;
                LOGD("[+] (STRTAB) :%p", (void *)dyn->d_un.d_ptr);
                break;
            }
            case DT_SYMTAB:
            {
                self->dynsym_tab = (ElfW(Sym)*)(self->bias_addr + dyn->d_un.d_ptr);
                dynsym_va = dyn->d_un.d_ptr;
                LOGD("[+] (SYMTAB) :%p", (void *)dyn->d_un.d_ptr);
                break;
            }
            case DT_SYMENT:
            {
                self->dynsym_ent = dyn->d_un.d_val;
                break;
            }
            case DT_PLTREL:
            {
                //use rel or rela?
                self->is_use_rela = (dyn->d_un.d_val == DT_RELA ? 1 : 0);
                break;
            }
            case DT_JMPREL:
            {
                self->relplt = self->bias_addr + dyn->d_un.d_ptr;
                LOGD("[+] (JMPREL) :%p", (void *)dyn->d_un.d_ptr);
                break;
            }
            case DT_PLTRELSZ:
            {
                self->relplt_sz = dyn->d_un.d_val;
                LOGD("[+] (PLTRELSZ) :%p", (void *)dyn->d_un.d_val);
                break;
            }
            case DT_REL:
            case DT_RELA:
            {
                self->reldyn = self->bias_addr + dyn->d_un.d_ptr;
                LOGD("[+] (REL/RELA) :%p", (void *)dyn->d_un.d_ptr);
                break;
            }
            case DT_RELSZ:
            case DT_RELASZ:
            {
                self->reldyn_sz = dyn->d_un.d_val;
                LOGD("[+] (RELSZ/RELASZ) :%p", (void *)dyn->d_un.d_val);
                break;
            }
            case DT_HASH:
            {
                self->hash = self->bias_addr + dyn->d_un.d_ptr;
                hash = (uint32_t*)self->hash;
                self->hash_bucket_cnt  = hash[0];
                self->hash_chain_cnt   = hash[1];
                self->hash_bucket      = &hash[2];
                self->hash_chain       = &(self->hash_bucket[self->hash_bucket_cnt]);
                LOGD("[+] (HASH) :%p", (void *)dyn->d_un.d_ptr);
                break;
            }
            case DT_GNU_HASH:
            {
                self->gnu_hash = self->bias_addr + dyn->d_un.d_ptr;
                hash = (uint32_t *)(self->gnu_hash);
                self->gnu_bucket_cnt  = hash[0];
                self->symoffset       = hash[1];
                self->bloom_sz        = hash[2];
                self->bloom_shift     = hash[3];
                self->bloom           = (ElfW(Addr) *)(&hash[4]);
                self->gnu_bucket      = (uint32_t *)(&(self->bloom[self->bloom_sz]));
                self->gnu_chain       = (uint32_t *)(&(self->gnu_bucket[self->gnu_bucket_cnt]));
                LOGD("[+] (GNU_HASH) :%p", (void *)dyn->d_un.d_ptr);
                break;
            }
            default:
                break;
        }
    }

    self->dynsym_count = (dynstr_va - dynsym_va) / self->dynsym_ent;
    return 0;
}

static void *fake_dlopen(const char *lib_name, int flags)
{
    FILE* fp = nullptr;
    char buff[MAX_LENGTH] = {0};
    void* base_addr = nullptr;
    ch_elf_t* self = nullptr;

    fp = fopen("/proc/self/maps", "r");
    if(NULL == fp)
        return self;

    //遍历maps
    while(fgets(buff, sizeof(buff), fp)){
        sscanf(buff, "%p", &base_addr);
        if (strstr(buff, "r-xp") && strstr(buff, lib_name)){
            self = (ch_elf_t*)malloc(sizeof(ch_elf_t)); //注意free
            if(ch_elf_init(self, (uintptr_t)base_addr) < 0){
                LOGE("[%s] ch_elf_init Error", __FUNCTION__);
            }
            break;
        }
    }

    if(NULL != fp)
        fclose(fp);

    return self;
}

static void *fake_dlsym(void *handle, const char *symbol_name)
{
    void* symbol_addr = nullptr;
    ch_elf_t* self = nullptr;
    char *dynstr_tab = nullptr;
    ElfW(Sym) *dynsym_tab = nullptr;

    self = (ch_elf_t*)handle;
    dynstr_tab = (char*)self->dynstr_tab;
    dynsym_tab = self->dynsym_tab;

    for (int k = 0; k < self->dynsym_count; k++, dynsym_tab++) {
        //LOGD("[%s] symbol:%s", __FUNCTION__, dynstr_tab + dynsym_tab->st_name);
        if (strcmp(dynstr_tab + dynsym_tab->st_name, symbol_name) == 0) {
            symbol_addr = (char *)self->bias_addr + dynsym_tab->st_value;
            LOGD("[%s] symbol_addr:%p", __FUNCTION__, symbol_addr);
            break;
        }
    }

    return symbol_addr;
}


void *dlopen_compat(const char *lib_name, int flags)
{
    if (get_sdk_level() >= 24) {
        return fake_dlopen(lib_name, flags);
    } else {
        return dlopen(lib_name, flags);
    }
}


void *dlsym_compat(void *handle, const char *symbol_name)
{
    if (get_sdk_level() >= 24) {
        return fake_dlsym(handle, symbol_name);
    } else {
        return dlsym(handle, symbol_name);
    }
}


const char *dlerror_compat()
{
    if (get_sdk_level() >= 24) {
        //return fake_dlerror();
        return nullptr;
    } else {
        return dlerror();
    }
}


int dlclose_compat(void *handle)
{
    if (get_sdk_level() >= 24) {
        //return fake_dlclose(handle);
        return -1;
    } else {
        return dlclose(handle);
    }
}

