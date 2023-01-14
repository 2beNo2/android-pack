//
// Created by huangchen on 2023/1/14.
//
#include <string>

#include "vm_core.h"
#include "jni_utils.h"

static const char* method_name_onCreate = "onCreate";
static const char* sig_onCreate = "(Landroid/os/Bundle;)V";

static const char* class_name_setContentView = "org/test/dexvmp/MainActivity";
static const char* method_name_setContentView = "setContentView";
static const char* sig_setContentView = "(I)V";

static const char* class_name_findViewById = "org/test/dexvmp/MainActivity";
static const char* method_name_findViewById = "findViewById";
static const char* sig_findViewById = "(I)Landroid/view/View;";

typedef int(*HANDLER)(vm_Context *);

inline uint4_t VRegA_35c(u1* insns){
    return static_cast<uint4_t>(*insns >> 4);
}

inline uint16_t VRegB_35c(u1* insns){
    return *insns;
}

inline uint4_t VRegC_35c(u1* insns){
    return static_cast<uint4_t>(*insns & 0x0f);
}

inline uint4_t VRegD_35c(u1* insns){
    return static_cast<uint4_t>((*insns >> 4) & 0x0f);
}

//AA|op CC|BB  ==>  高8位|低8位
static int vm_add(vm_Context *ctx)
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

static int vm_ret_int(vm_Context *ctx)
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

static int vm_invoke_super(vm_Context *ctx)
{
    /*
     * 6f20 0300 5400   invoke-super {p0, p1},
     *          Landroidx/appcompat/app/AppCompatActivity;->onCreate(Landroid/os/Bundle;)V # method@0003
     * 6e..72 35c -> 6f: invoke-super
     *        35c A|G|op BBBB F|E|D|C
     *            20 6f  0003 0054
     *            op = 6f
     *            A = 2  -> [A=2] op {vC, vD}, kind@BBBB
     *            G = 0
     *            BBBB = 3
     *            C = 4
     *            D = 5
     * */
    uint4_t VRegA = VRegA_35c(ctx->pc++);
    uint16_t VRegB = VRegB_35c(ctx->pc++);
    ctx->pc++;
    switch (VRegA) {
        case 0:
            LOGD("[%s] invoke-super:%d", __FUNCTION__, VRegA);
            break;
        case 1:
            LOGD("[%s] invoke-super:%d", __FUNCTION__, VRegA);
            break;
        case 2:{
            //A = 2  -> [A=2] op {vC, vD}, kind@BBBB
            uint4_t VRegC = VRegC_35c(ctx->pc);
            uint4_t VRegD = VRegD_35c(ctx->pc);
            ctx->pc++;
            ctx->pc++;
            LOGD("[%s] invoke-super VRegA:%d，methodID:%d", __FUNCTION__, VRegA, VRegB);

            jvalue result = {0};
            int attach = 0;
            JNIEnv * env = nullptr;
            int ret = -1;
            char class_name[260] = {0};
            JavaVM* vm = jni_get_JavaVM();
            //获取env
            env = jni_get_jnienv(vm, &attach);
            if(env == nullptr && attach == 0){
                LOGE("[%s] GetJNIEnv faild!", __FUNCTION__);
                return -1;
            }
            jobject thiz = (jobject)(size_t)ctx->v_regs[VRegC];
            jobject obj_param_1 = (jobject)(size_t)ctx->v_regs[VRegD];
            ret = jni_call_method(&result, env, thiz,
                                  "java/lang/Object", "getClass", "()Ljava/lang/Class;");
            if(ret < 0){
                LOGE("[%s] jni_call_method->getClass Failed", __FUNCTION__);
                return -1;
            }
            jobject obj_Class = result.l;

            //获取父类
            ret = jni_call_method(&result, env, obj_Class,
                                  "java/lang/Class", "getSuperclass", "()Ljava/lang/Class;");
            if(ret < 0){
                LOGE("[%s] jni_call_method->getSuperclass Failed", __FUNCTION__);
                return -1;
            }
            obj_Class = result.l;

            ret = jni_call_method(&result, env, obj_Class,
                                  "java/lang/Class", "getName", "()Ljava/lang/String;");
            if(ret < 0){
                LOGE("[%s] jni_call_method->getName Failed", __FUNCTION__);
                return -1;
            }
            jobject obj_ClassName = result.l;
            jni_jstring_2_cstring(class_name, env, obj_ClassName);
            for(int i = 0; i < strlen(class_name); ++i){
                if(class_name[i] == '.'){
                    class_name[i] = '/';
                }
            }
            /*
             * invoke-super:
             * 需要获取dex文件的内存地址，进行dex文件格式的解析，来获取方法的详细信息
             * 加固厂商可能已经提前收集好
             * */
            if(VRegB == 3){
                ret = jni_call_novirtual_method(&result, env, thiz,
                                                class_name, method_name_onCreate,
                                                sig_onCreate, obj_param_1);
            }

            if(ret < 0){
                LOGE("[%s] jni_call_method->getName Failed", __FUNCTION__);
                return -1;
            }

            env->DeleteLocalRef(obj_Class);
            env->DeleteLocalRef(obj_ClassName);

            //保存函数调用后的返回值
            ctx->ret_value = result;

            if(attach == 1){
                jni_del_jnienv(vm);
            }
            break;
        }
        default:
            LOGD("[%s] invoke-super:%d default", __FUNCTION__, VRegA);
            break;
    }

    return 0;
}

static int vm_const(vm_Context *ctx)
{
    /*
     * 1400 1c00 0b7f   0003: const v0, 0x7f0b001c
     * 31i const vAA, #+BBBBBBBB
     *            AA|op BBBBlo BBBBhi
     *            AA = 0
     *            BBBBlo = 001c
     *            BBBBhi = 7f0b
     * */
    u1 vAA = *ctx->pc++;
    uint32_t u_BBBB = *(uint32_t*)ctx->pc;
    ctx->v_regs[vAA] = u_BBBB;
    LOGD("[%s] v%d = %d", __FUNCTION__ , vAA, u_BBBB);
    ctx->pc += 4;
    return 0;
}

static int vm_invoke_virtual(vm_Context *ctx)
{
    /*
     * 6e20 1200 0400   invoke-virtual
     * 6e..72 35c -> 6e: invoke-virtual
     *        35c A|G|op BBBB  F|E|D|C
     *            20 6e  00012 0004
     *            op = 6f
     *            A = 2  -> [A=2] op {vC, vD}, kind@BBBB
     *            G = 0
     *            BBBB = 12
     *            C = 4
     *            D = 0
     * */
    uint4_t VRegA = VRegA_35c(ctx->pc++);
    uint16_t VRegB = VRegB_35c(ctx->pc++);
    ctx->pc++;
    switch (VRegA) {
        case 0:
            LOGD("[%s] invoke-virtual:%d", __FUNCTION__, VRegA);
            break;
        case 1:
            LOGD("[%s] invoke-virtual:%d", __FUNCTION__, VRegA);
            break;
        case 2:{
            //A = 2  -> [A=2] op {vC, vD}, kind@BBBB
            uint4_t VRegC = VRegC_35c(ctx->pc);
            uint4_t VRegD = VRegD_35c(ctx->pc);
            ctx->pc++;
            ctx->pc++;
            LOGD("[%s] invoke-virtual VRegA:%d，methodID:%d", __FUNCTION__, VRegA, VRegB);

            jvalue result = {0};
            int attach = 0;
            JNIEnv * env = nullptr;
            int ret = -1;
            char class_name[260] = {0};
            JavaVM* vm = jni_get_JavaVM();
            //获取env
            env = jni_get_jnienv(vm, &attach);
            if(env == nullptr && attach == 0){
                LOGE("[%s] GetJNIEnv faild!", __FUNCTION__);
                return -1;
            }
            jobject thiz = (jobject)(size_t)ctx->v_regs[VRegC];
            jobject obj_param_1 = (jobject)(size_t)ctx->v_regs[VRegD];

            /*
             * invoke-super:
             * 需要获取dex文件的内存地址，进行dex文件格式的解析，来获取方法的详细信息
             * 加固厂商可能已经提前收集好
             * */
            if(VRegB == 0x12){
                ret = jni_call_method(&result, env, thiz,
                                      class_name_setContentView, method_name_setContentView,
                                      sig_setContentView, obj_param_1);

            }else if(VRegB == 0x10){
                ret = jni_call_method(&result, env, thiz,
                                      class_name_findViewById, method_name_findViewById,
                                      sig_findViewById, obj_param_1);
            }
            if(ret < 0){
                LOGE("[%s] jni_call_method->getName Failed", __FUNCTION__);
                return -1;
            }

            //保存函数调用后的返回值
            ctx->ret_value = result;

            if(attach == 1){
                jni_del_jnienv(vm);
            }
            break;
        }
        default:
            LOGD("[%s] invoke-virtual:%d default", __FUNCTION__, VRegA);
            break;
    }

    return 0;
}

static int vm_move_result_object(vm_Context *ctx)
{
    /*
     * 0C00     move-result-object  v0
     * 0c 11x	move-result-object vAA
     *    11x   AA|op
     *    11x	op vAA
     *
     * */
    u1 vAA = *ctx->pc++;

    ctx->v_regs[vAA] = (uint32_t)(size_t)ctx->ret_value.l;
    LOGD("[%s] v%d = %d", __FUNCTION__ , vAA, (uint32_t)(size_t)ctx->ret_value.l);

    return 0;
}


static HANDLER vm_handlers[] = {
        &vm_add,
        &vm_ret_int,
        &vm_invoke_super,
        &vm_const,
        &vm_invoke_virtual,
        &vm_move_result_object,
};


void vm_Loop_impl(vm_Context* ctx)
{
    unsigned char op = *ctx->pc++;
    (*vm_handlers[op])(ctx);
}