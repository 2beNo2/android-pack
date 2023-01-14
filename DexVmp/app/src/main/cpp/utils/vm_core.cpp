//
// Created by huangchen on 2023/1/14.
//

#include "vm_core.h"
#include "jni_utils.h"


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
            LOGD("[%s] invoke-super VRegA:%d，methodID:%d", __FUNCTION__, VRegA, VRegB);
            //JNIHIDE int jni_call_novirtual_method(jvalue *result, JNIEnv *env,
            // jobject objClz, const char* clsName, const char* methodName, const char* sig, ...);
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

            //需要获取dex文件的内存地址，进行dex文件格式的解析

            //invoke-super
            jni_call_novirtual_method(&result, env, thiz, class_name, "", "");

            env->DeleteLocalRef(obj_Class);
            env->DeleteLocalRef(obj_ClassName);
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

static HANDLER vm_handlers[] = {
        &vm_add,
        &vm_ret_int,
        &vm_invoke_super,
};


void vm_Loop_impl(vm_Context* ctx)
{
    unsigned char op = *ctx->pc++;
    (*vm_handlers[op])(ctx);
}