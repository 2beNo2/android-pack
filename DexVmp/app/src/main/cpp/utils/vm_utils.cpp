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


unsigned char onCreate_codeItem[112] = {
        0x06, 0x00, //registers_size_
        0x02, 0x00, //ins_size_
        0x03, 0x00, //outs_size_
        0x00, 0x00, //tries_size_
        0xDA, 0x04, 0x00, 0x00,   //debug_info_off_
        0x30, 0x00, 0x00, 0x00, //insns_size_in_code_units_
        0x6F, 0x20, 0x03, 0x00, 0x54, 0x00,
        0x14, 0x00, 0x1C, 0x00, 0x0B, 0x7F,
        0x6E, 0x20, 0x12, 0x00,0x04, 0x00,
        0x14, 0x00, 0xB0, 0x01, 0x08, 0x7F,
        0x6E, 0x20, 0x10, 0x00, 0x04, 0x00,
        0x0C, 0x00,
        0x1F, 0x00, 0x04, 0x00,
        0x22, 0x01, 0x0B, 0x00,
        0x70, 0x10, 0x07, 0x00, 0x01, 0x00,
        0x1A, 0x02,0x01, 0x00,
        0x6E, 0x20, 0x09, 0x00, 0x21, 0x00,
        0x0C, 0x01,
        0x12, 0x52,
        0x13, 0x03, 0x08, 0x00,
        0x6E, 0x30, 0x0F, 0x00, 0x24, 0x03,
        0x0A, 0x02,
        0x6E, 0x20, 0x08, 0x00, 0x21, 0x00,
        0x0C, 0x01,
        0x6E, 0x10, 0x0A, 0x00, 0x01, 0x00,
        0x0C, 0x01,
        0x6E, 0x20, 0x01, 0x00, 0x10, 0x00,
        0x0E, 0x00
};

/*
 .method protected onCreate(Landroid/os/Bundle;)V
    00000434: 6f20 0300 5400          0000: invoke-super        {p0, p1}, Landroidx/appcompat/app/AppCompatActivity;->onCreate(Landroid/os/Bundle;)V # method@0003
    0000043a: 1400 1c00 0b7f          0003: const               v0, 0x7f0b001c
    00000440: 6e20 1200 0400          0006: invoke-virtual      {p0, v0}, Lorg/test/dexvmp/MainActivity;->setContentView(I)V # method@0012
    00000446: 1400 b001 087f          0009: const               v0, 0x7f0801b0
    0000044c: 6e20 1000 0400          000c: invoke-virtual      {p0, v0}, Lorg/test/dexvmp/MainActivity;->findViewById(I)Landroid/view/View; # method@0010
    00000452: 0c00                    000f: move-result-object  v0
    00000454: 1f00 0400               0010: check-cast          v0, Landroid/widget/TextView; # type@0004
    00000458: 2201 0b00               0012: new-instance        v1, Ljava/lang/StringBuilder; # type@000b
    0000045c: 7010 0700 0100          0014: invoke-direct       {v1}, Ljava/lang/StringBuilder;-><init>()V # method@0007
    00000462: 1a02 0100               0017: const-string        v2, "5 + 8 = " # string@0001
    00000466: 6e20 0900 2100          0019: invoke-virtual      {v1, v2}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder; # method@0009
    0000046c: 0c01                    001c: move-result-object  v1
    0000046e: 1252                    001d: const/4             v2, 0x5
    00000470: 1303 0800               001e: const/16            v3, 0x8
    00000474: 6e30 0f00 2403          0020: invoke-virtual      {p0, v2, v3}, Lorg/test/dexvmp/MainActivity;->MyAdd(I, I)I # method@000f
    0000047a: 0a02                    0023: move-result         v2
    0000047c: 6e20 0800 2100          0024: invoke-virtual      {v1, v2}, Ljava/lang/StringBuilder;->append(I)Ljava/lang/StringBuilder; # method@0008
    00000482: 0c01                    0027: move-result-object  v1
    00000484: 6e10 0a00 0100          0028: invoke-virtual      {v1}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String; # method@000a
    0000048a: 0c01                    002b: move-result-object  v1
    0000048c: 6e20 0100 1000          002c: invoke-virtual      {v0, v1}, Landroid/widget/TextView;->setText(Ljava/lang/CharSequence;)V # method@0001
    00000492: 0e00                    002f: return-void
.end method
 * */


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


