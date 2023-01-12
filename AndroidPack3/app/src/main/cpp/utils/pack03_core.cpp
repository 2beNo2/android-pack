//
// Created by huangchen on 2023/1/11.
//
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "../Dobby/include/dobby.h"
#include "pack03_core.h"
#include "aosp_header.h"
#include "aosp_header.h"


typedef int(*EXECVE)(const char*, char* const [], char *const []);
typedef art::ArtMethod*(*LOADMETHOD_21)(void*, void*, art::DexFile*, void*, void*);

static int SDK_INT = -1;
static void* old_execve = nullptr;
static void* old_LoadMethod_21 = nullptr;

//------------------------------------------------------------
//-----------       Created with 010 Editor        -----------
//------         www.sweetscape.com/010editor/          ------
//
// File    : /Users/hc/AndroidStudioProjects/AndroidPack3/app/src/main/jnilibs/arm64-v8a/libdex.so
// Address : 2541160 (0x26C668)
// Size    : 32 (0x20)
//------------------------------------------------------------
unsigned char g_CodeItem[] = {
        0x02, 0x00,  //uint16_t registers_size_;
        0x02, 0x00,  //uint16_t ins_size_;
        0x02, 0x00,  //uint16_t outs_size_;
        0x00, 0x00,  //uint16_t tries_size_;
        0xA6, 0xA0, 0x29, 0x00,  //uint32_t debug_info_off_;
        0x08, 0x00, 0x00, 0x00, //uint32_t insns_size_in_code_units_;
        0x6F, 0x20, 0xE0, 0x00,
        0x10, 0x00, 0x1A, 0x01,
        0xBE, 0x3C, 0x5B, 0x01,
        0x1F, 0x5F, 0x0E, 0x00
};


int new_execve(const char* path, char* const argv[], char *const envp[])
{
    LOGD("[%s] path:%s", __FUNCTION__, path);
    if (strcmp(path, "/system/bin/dex2oat") == 0)
        return 0;
    return ((EXECVE)old_execve)(path, argv, envp);
}

static int core_hook_dex2oat()
{
    void* execveAddr = nullptr;
    int ret = -1;

    execveAddr = DobbySymbolResolver("libc.so", "execve");
    if(nullptr == execveAddr){
        LOGE("[%s] DobbySymbolResolver->execve Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] DobbySymbolResolver->execve Ok, execveAddr:%p", __FUNCTION__, execveAddr);

    ret = DobbyHook(execveAddr, (dobby_dummy_func_t)&new_execve, (dobby_dummy_func_t*)&old_execve);
    if(RT_FAILED == ret){
        LOGE("[%s] DobbyHook->execve Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] DobbyHook->execve Ok, result:%d", __FUNCTION__, ret);

    return 0;
}

static int core_get_sdk_level()
{
    if (SDK_INT > 0) {
        return SDK_INT;
    }
    char sdk[260] = {0};
    __system_property_get("ro.build.version.sdk", sdk);
    SDK_INT = atoi(sdk);
    return SDK_INT;
}


art::ArtMethod* new_LoadMethod_21(void* thiz, void* self, art::DexFile* dex_file, void* it, void* klass)
{
    art::DexFile::Header* header = (art::DexFile::Header*)dex_file->begin_;
    art::ArtMethod* artMethod = ((LOADMETHOD_21)old_LoadMethod_21)(thiz, self, dex_file, it, klass);
    if(header->checksum_ != 0x904e8e9b || artMethod->dex_method_index_ != 0x8008 || artMethod->access_flags_ != 4){
        return artMethod;
    }
    LOGD("[%s] artMethod->access_flags_:%p, artMethod->dex_method_index_:%p, artMethod->dex_code_item_offset_:%p", __FUNCTION__,
         artMethod->access_flags_, artMethod->dex_method_index_, artMethod->dex_code_item_offset_);

    art::DexFile::CodeItem* codeItem = (art::DexFile::CodeItem*)((char*)dex_file->begin_ + artMethod->dex_code_item_offset_);

    LOGD("[%s] codeItem->registers_size_:%02x", __FUNCTION__, codeItem->registers_size_);
    LOGD("[%s] codeItem->ins_size_:%02x", __FUNCTION__, codeItem->ins_size_);
    LOGD("[%s] codeItem->outs_size_:%02x", __FUNCTION__, codeItem->outs_size_);
    LOGD("[%s] codeItem->debug_info_off_:%p", __FUNCTION__, codeItem->debug_info_off_);
    LOGD("[%s] codeItem->insns_size_in_code_units_:%d", __FUNCTION__, codeItem->insns_size_in_code_units_);

    LOGD("[%s] old code:%04x %04x %04x %04x",
         __FUNCTION__, codeItem->insns_[0], codeItem->insns_[1], codeItem->insns_[2], codeItem->insns_[3]);

    mprotect((void*)((size_t)codeItem & ~0xfff), 0x1000, PROT_READ | PROT_WRITE);

    for(int i = 0; i < codeItem->insns_size_in_code_units_; i++){
        codeItem->insns_[i] = ((art::DexFile::CodeItem*)g_CodeItem)->insns_[i];
    }

    LOGD("[%s] new code:%04x %04x %04x %04x",
         __FUNCTION__, codeItem->insns_[0], codeItem->insns_[1],codeItem->insns_[2],codeItem->insns_[3]);

    return artMethod;
}

static int core_hook_LoadMethod_21()
{
    void* LoadMethodAddr = nullptr;
    int ret = -1;

    LoadMethodAddr = DobbySymbolResolver("libart.so", "_ZN3art11ClassLinker10LoadMethodEPNS_6ThreadERKNS_7DexFileERKNS_21ClassDataItemIteratorENS_6HandleINS_6mirror5ClassEEE");
    if(nullptr == LoadMethodAddr){
        LOGE("[%s] DobbySymbolResolver->LoadMethod Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] DobbySymbolResolver->LoadMethod Ok, execveAddr:%p", __FUNCTION__, LoadMethodAddr);

    ret = DobbyHook(LoadMethodAddr, (dobby_dummy_func_t)&new_LoadMethod_21, (dobby_dummy_func_t*)&old_LoadMethod_21);
    if(RT_FAILED == ret){
        LOGE("[%s] DobbyHook->LoadMethod Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] DobbyHook->LoadMethod Ok, result:%d", __FUNCTION__, ret);

    return 0;
}



int pack03_core()
{
    int ret = -1;

    core_get_sdk_level();
    LOGD("[%s] SDK_INT:%d", __FUNCTION__, SDK_INT);

    //函数抽取壳需要禁用掉`dex2oat`
    if(core_hook_dex2oat() < 0){
        return -1;
    }

    //对LoadMethod 进行hook 监控
    switch (SDK_INT) {
        case 21:
            ret = core_hook_LoadMethod_21();
            break;
        default:
            break;
    }

    if(ret < 0){
        return -1;
    }
    return 0;
}



