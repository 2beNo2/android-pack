#include <jni.h>
#include <string>
#include <sys/stat.h>
#include <errno.h>

#include "log.h"
#include "jni_utils.h"
#include "pack_utils.h"

static jobject g_ObjContext;

JNIHIDE void JNICALL MyApp_onCreate(JNIEnv *env, jobject thiz);
JNIHIDE void JNICALL MyApp_attachBaseContext(JNIEnv *env, jobject thiz, jobject base);

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

    jclass clsMyApp =  env->FindClass("org/test/androidpack1/MyApp");
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        LOGE("[%s] FindClass Error name:%s", __FUNCTION__, "org/test/androidpack1/MyApp");
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


JNIHIDE void JNICALL MyApp_onCreate(JNIEnv *env, jobject thiz)
{
    // TODO: implement onCreate()
    int ret = -1;
    jvalue result = {0};
    ret = jni_call_novirtual_method(&result, env, thiz,"android/app/Application","onCreate","()V");
    if(0 > ret){
        LOGE("[%s] super.onCreate() Errors", __FUNCTION__);
        return;
    }
    LOGD("[%s] super.onCreate() Ok", __FUNCTION__);

    //调用被加壳dex的Application的初始化方法 attachBaseContext、onCreate
    replace_application(env, thiz, g_ObjContext);

    LOGD("[%s] Application.onCreate() over", __FUNCTION__);
}


JNIHIDE void JNICALL MyApp_attachBaseContext(JNIEnv *env, jobject thiz, jobject base)
{
    // TODO: implement attachBaseContext()
    int ret = -1;
    jvalue result = {0};

    ret = jni_call_novirtual_method(&result, env, thiz,
                                    "android/app/Application",
                                    "attachBaseContext",
                                    "(Landroid/content/Context;)V", base);
    if(0 > ret){
        LOGE("[%s] super.attachBaseContext() Errors", __FUNCTION__);
        return;
    }
    LOGD("[%s] super.attachBaseContext() Ok", __FUNCTION__);

    g_ObjContext = env->NewGlobalRef(base);

    //动态加载dex文件，并替换classLoader
    replace_classloder(env, thiz, base);
}


