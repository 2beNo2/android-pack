#include <jni.h>
#include <string>

#include "utils/log.h"
#include "utils/vm_utils.h"

JNIHIDE jint JNICALL interface1(JNIEnv *env, jclass clazz, jobjectArray args);
JNIHIDE void JNICALL interface2(JNIEnv *env, jclass clazz, jobjectArray args);

JNIHIDE JNINativeMethod g_Methods[] =
{
"interface1", "([Ljava/lang/Object;)I", (void*)&interface1,
"interface2", "([Ljava/lang/Object;)V", (void*)&interface2,
};


JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env = nullptr;
    jint result = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (JNI_OK != result) {
        LOGE("[%s] GetEnv Error result:%d", __FUNCTION__, result);
        return result;
    }
    LOGD("[%s] env:%p", __FUNCTION__, env);

    //动态注册
    jclass clsMyApp =  env->FindClass("org/test/dexvmp/ProxyApp");
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        LOGE("[%s] FindClass Error name:%s", __FUNCTION__, "org/test/dexvmp/ProxyApp");
        return result;
    }

    result = env->RegisterNatives(clsMyApp, g_Methods, sizeof(g_Methods) / sizeof(g_Methods[0]));
    if (JNI_OK != result) {
        LOGE("[%s] RegisterNatives Error result:%d", __FUNCTION__, result);
        return result;
    }

    jni_save_JavaVM(vm);
    LOGD("[%s] RegisterNatives OK result:%d", __FUNCTION__, result);
    return JNI_VERSION_1_6;
}


JNIHIDE jint JNICALL interface1(JNIEnv *env, jclass clazz, jobjectArray args)
{
    return vm_Interpreter(env, clazz, args).i;
}

JNIHIDE void JNICALL interface2(JNIEnv *env, jclass clazz, jobjectArray args)
{
    vm_Interpreter(env, clazz, args);
}
