//
// Created by huangchen on 2023/1/14.
//

#ifndef DEXVMP_VM_CORE_H
#define DEXVMP_VM_CORE_H

#include <jni.h>
#include "log.h"

struct vm_Context;
typedef unsigned char u1;
typedef int8_t int4_t;
typedef uint8_t uint4_t;

struct vm_Context{
    u1* pc;
    u1* pc_end;
    uint32_t *v_regs;
    jvalue ret_value;
    bool is_exit;
};

struct CodeItem{
    uint16_t registers_size_;
    uint16_t ins_size_;
    uint16_t outs_size_;
    uint16_t tries_size_;
    uint32_t debug_info_off_;
    uint32_t insns_size_in_code_units_;
    uint16_t insns_[1];
};

JNIHIDE void vm_Loop_impl(vm_Context* ctx);

#endif //DEXVMP_VM_CORE_H
