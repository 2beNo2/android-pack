//
// Created by huangchen on 2023/1/13.
//
#include <string>

#include "vm_utils.h"


struct vm_Context;

typedef unsigned char u1;
typedef int(*HANDLER)(vm_Context *);

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

JNIHIDE unsigned char add_codeItem[22] = {
        0x04, 0x00, //registers_size_
        0x03, 0x00, //ins_size_
        0x00, 0x00, //outs_size_
        0x00, 0x00, //tries_size_
        0xFA, 0x04, 0x00, 0x00,   //debug_info_off_
        0x03, 0x00, 0x00, 0x00, //insns_size_in_code_units_
        0x00, 0x00, 0x02, 0x03, //add-int v0, p1, p2
        0x01, 0x00                        //return v0

        // 0x90 -> 0x00
        // 0x0F -> 0x01
};



int vm_add(vm_Context *ctx)
{
    /*
     * 90..af 23x	binop vAA, vBB, vCC
     *        23x	op vAA, vBB, vCC
     *              AA|op CC|BB
     * 0x90, 0x00, 0x02, 0x03, //add-int v0, p1, p2
     * */
    u1 vAA = *ctx->pc++;
    u1 vBB = *ctx->pc++;
    u1 vCC = *ctx->pc++;

    ctx->v_regs[vAA] = ctx->v_regs[vBB] + ctx->v_regs[vCC];
    LOGD("[%s] %d+%d=%d", __FUNCTION__ , ctx->v_regs[vBB], ctx->v_regs[vCC], ctx->v_regs[vAA]);

    return 0;
}


int vm_ret_int(vm_Context *ctx)
{
    /*
     * 0f 11x	return vAA
     *    11x	op vAA
     *          AA|op
     * 0x0F, 0x00 //return v0
     * */
    u1 vAA = *ctx->pc++;

    ctx->ret_value.i = ctx->v_regs[vAA];
    ctx->is_exit = true;
    LOGD("[%s] result:%d", __FUNCTION__ , ctx->ret_value.i);

    return 0;
}


JNIHIDE HANDLER vm_handlers[] = {
        &vm_add,
        &vm_ret_int,
};

static int vm_Init(JNIEnv *env, jclass clazz, jobjectArray args, vm_Context* ctx)
{
    jvalue result = {0};
    int ret = -1;
    int args_count = 0;
    jobject obj_MethodIndex = nullptr;
    int method_index = -1;
    jobject obj_thiz = nullptr;
    char class_name[260] = {0};

    /*
     * ProxyApp.interface1(new Object[] {1000, this, n1, n2});
     * Object[]：method_id，this，args
     * */
    args_count = env->GetArrayLength(args);
    if (args_count < 2) {
        LOGE("[%s] args_count < 2", __FUNCTION__);
        return -1;
    }

    //获取被传入的参数信息
    obj_MethodIndex = env->GetObjectArrayElement(args, 0);
    if(env->ExceptionCheck()){
        LOGE("[%s] GetObjectArrayElement Erros", __FUNCTION__);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    ret = jni_call_method(&result, env, obj_MethodIndex, "java/lang/Integer", "intValue", "()I");
    if(ret < 0){
        LOGE("[%s] jni_call_method->intValue Failed", __FUNCTION__);
        return -1;
    }
    method_index = result.i;
    method_index = method_index - 1000;
    args_count = args_count - 1;

    obj_thiz = env->GetObjectArrayElement(args, 1);
    if(env->ExceptionCheck()){
        LOGE("[%s] GetObjectArrayElement Erros", __FUNCTION__);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    LOGD("[%s] method_index:%d obj_thiz:%p args_count:%d ", __FUNCTION__, method_index, obj_thiz, args_count);

    //准备参数环境和寄存器环境
    CodeItem* codeItem = nullptr;
    if(method_index == 0){
        codeItem = (CodeItem* )add_codeItem;
    } else{
        codeItem = (CodeItem* )add_codeItem;
    }

    ctx->pc = reinterpret_cast<u1 *>(codeItem->insns_);
    ctx->pc_end = reinterpret_cast<u1 *>(codeItem->insns_ + codeItem->insns_size_in_code_units_);
    ctx->v_regs = new uint32_t[codeItem->registers_size_];
    memset(ctx->v_regs, 0, sizeof(uint32_t) * codeItem->registers_size_);
    ctx->ret_value = {0};
    ctx->is_exit = false;

    int param_index = codeItem->registers_size_ - args_count;
    for(int i = 1; i <= args_count; i++){
        jobject obj = env->GetObjectArrayElement(args, i);
        if(env->ExceptionCheck()){
            LOGE("[%s] GetObjectArrayElement Erros", __FUNCTION__);
            env->ExceptionDescribe();
            env->ExceptionClear();
            return -1;
        }
        if (obj == nullptr){
            continue;
        }

        ret = jni_call_method(&result, env, obj, "java/lang/Object", "getClass", "()Ljava/lang/Class;");
        if(ret < 0){
            LOGE("[%s] jni_call_method->getClass Failed", __FUNCTION__);
            return -1;
        }
        jobject obj_Class = result.l;
        ret = jni_call_method(&result, env, obj_Class, "java/lang/Class", "getName", "()Ljava/lang/String;");
        if(ret < 0){
            LOGE("[%s] jni_call_method->getName Failed", __FUNCTION__);
            return -1;
        }
        jobject obj_ClassName = result.l;
        jni_jstring_2_cstring(class_name, env, obj_ClassName);

        if (strcmp(class_name, "java.lang.Integer") == 0) {
            ret = jni_call_method(&result, env, obj, "java/lang/Integer", "intValue", "()I");
            if(ret < 0){
                LOGE("[%s] jni_call_method->intValue Failed", __FUNCTION__);
                return -1;
            }
            ctx->v_regs[param_index++] = result.i;
        }
        else {
            ctx->v_regs[param_index++] = (uint32_t)(size_t)obj;
        }
    }

    return 0;
}


static void vm_Loop(vm_Context* ctx)
{
    //取指令、译码、执行
    while (ctx->pc < ctx->pc_end){
        unsigned char op = *ctx->pc++;

        (*vm_handlers[op])(ctx);

        if(ctx->is_exit)
            break;
    }
}


static void vm_Exit(vm_Context* ctx)
{
    //退出虚拟机，并释放资源
    delete[] ctx->v_regs;
}


jvalue vm_Interpreter(JNIEnv *env, jclass clazz, jobjectArray args)
{
    jvalue ret = {0};
    vm_Context ctx;

    if(vm_Init(env, clazz, args, &ctx) < 0){
        LOGE("[%s] vm_Init Failed", __FUNCTION__);
        return ret;
    }

    vm_Loop(&ctx);

    vm_Exit(&ctx);

    ret = ctx.ret_value;
    return ret;
}


