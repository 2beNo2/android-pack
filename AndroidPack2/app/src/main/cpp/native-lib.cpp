#include <jni.h>
#include <string>
#include <sys/stat.h>
#include <errno.h>

#include "utils/log.h"
#include "utils/jni_utils.h"
#include "utils/pack_utils.h"

static jobject g_ObjContext;

JNIHIDE void JNICALL MyApp_onCreate(JNIEnv *env, jobject thiz);
JNIHIDE void JNICALL MyApp_attachBaseContext(JNIEnv *env, jobject thiz, jobject objContext);

JNIHIDE JNINativeMethod g_Methods[] = {
        "attachBaseContext", "(Landroid/content/Context;)V", (void*)&MyApp_attachBaseContext,
        "onCreate", "()V", (void*)&MyApp_onCreate,
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
    jclass clsMyApp =  env->FindClass("org/test/androidpack2/MyApp");
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        LOGE("[%s] FindClass Error name:%s", __FUNCTION__, "org/test/androidpack2/MyApp");
        return result;
    }

    result = env->RegisterNatives(clsMyApp, g_Methods, sizeof(g_Methods) / sizeof(g_Methods[0]));
    if (JNI_OK != result) {
        LOGE("[%s] RegisterNatives Error result:%d", __FUNCTION__, result);
        return result;
    }

    LOGD("[%s] RegisterNatives OK result:%d", __FUNCTION__, result);
    return JNI_VERSION_1_6;
}


JNIHIDE void JNICALL MyApp_attachBaseContext(JNIEnv *env, jobject thiz, jobject objContext)
{
    int ret = -1;
    jvalue result = {0};

    ret = jni_call_novirtual_method(&result, env, thiz, "android/app/Application",
                                    "attachBaseContext", "(Landroid/content/Context;)V", objContext);
    if(0 > ret){
        LOGE("[%s] super.attachBaseContext() Errors", __FUNCTION__);
        return;
    }
    LOGD("[%s] super.attachBaseContext() Ok", __FUNCTION__);

    g_ObjContext = env->NewGlobalRef(objContext);

    //Android 一代壳：动态加载dex文件，并替换classLoader
//    if(pack_android_pack01_replace_classLoader(env, thiz, objContext) < 0){
//        return;
//    }

    //Android 二代壳：从内存加载dex文件，并替换mCookie
    if(pack_android_pack02_replace_mCookie(env, thiz, objContext) < 0){
        return;
    }

}


JNIHIDE void JNICALL MyApp_onCreate(JNIEnv *env, jobject thiz)
{
    int ret = -1;
    jvalue result = {0};
    ret = jni_call_novirtual_method(&result, env, thiz,"android/app/Application","onCreate","()V");
    if(0 > ret){
        LOGE("[%s] super.onCreate() Errors", __FUNCTION__);
        return;
    }
    LOGD("[%s] super.onCreate() Ok", __FUNCTION__);

    //调用被加壳dex的Application的初始化方法 attachBaseContext、onCreate
    if(pack_replace_application(env, thiz, g_ObjContext) < 0){
        return;
    }
    LOGD("[%s] Application.onCreate() over", __FUNCTION__);
}



