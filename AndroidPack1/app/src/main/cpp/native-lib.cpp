#include <jni.h>
#include <string>
#include <sys/stat.h>
#include <errno.h>

#include "log.h"
#include "jni_utils.h"

static jobject g_context;


JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env = nullptr;
    jint result = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (JNI_OK != result) {
        LOGE("[%s] GetEnv Error result:%d", __FUNCTION__, result);
        return result;
    }
    LOGD("[%s] env:%p", __FUNCTION__, env);
    return JNI_VERSION_1_6;
}


JNIHIDE void replace_application(JNIEnv *env, jobject thiz)
{
    int ret = -1;
    jvalue result = {0};

    ret = jni_call_method(&result, env, thiz, "android/content/ContextWrapper",
                          "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
    if(0 > ret){
        LOGE("[%s] jni_call_method->getApplicationInfo Errors", __FUNCTION__);
        return;
    }
    jobject objApplicationInfo = result.l;
    LOGD("[%s] jni_call_method->getApplicationInfo Ok, objApplicationInfo:%p", __FUNCTION__, objApplicationInfo);

    jobject objClassName = env->NewStringUTF("org.test.dest.DestApp");
    jvalue value_set = {0};
    value_set.l = objClassName;
    ret = jni_set_field(env, objApplicationInfo, "android/content/pm/ApplicationInfo","className", "Ljava/lang/String;", value_set);
    if(0 > ret){
        LOGE("[%s] jni_set_field->className Errors", __FUNCTION__);
        return;
    }
    LOGD("[%s] jni_set_field->className Ok", __FUNCTION__);

    ret = jni_get_field(&result, env, g_context, "android/app/ContextImpl", "mPackageInfo", "Landroid/app/LoadedApk;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->mPackageInfo Errors", __FUNCTION__);
        return;
    }
    jobject objLoadedApk = result.l;
    LOGD("[%s] jni_get_field->mPackageInfo Ok, objLoadedApk:%p", __FUNCTION__, objLoadedApk);

    value_set.l = nullptr;
    ret = jni_set_field(env, objLoadedApk, "android/app/LoadedApk","mApplication", "Landroid/app/Application;", value_set);
    if(0 > ret){
        LOGE("[%s] jni_set_field->mApplication Errors", __FUNCTION__);
        return;
    }
    LOGD("[%s] jni_set_field->mApplication Ok", __FUNCTION__);


    ret = jni_get_field(&result, env, objLoadedApk, "android/app/LoadedApk", "mActivityThread", "Landroid/app/ActivityThread;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->mActivityThread Errors", __FUNCTION__);
        return;
    }
    jobject objActivityThread = result.l;
    LOGD("[%s] jni_get_field->mActivityThread Ok, objActivityThread:%p", __FUNCTION__, objActivityThread);


    ret = jni_get_field(&result, env, objActivityThread, "android/app/ActivityThread", "mAllApplications", "Ljava/util/ArrayList;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->mAllApplications Errors", __FUNCTION__);
        return;
    }
    jobject objAllApplications = result.l;
    LOGD("[%s] jni_get_field->mAllApplications Ok, objAllApplications:%p", __FUNCTION__, objAllApplications);


    ret = jni_call_method(&result, env, objAllApplications, "java/util/ArrayList", "remove", "(Ljava/lang/Object;)Z", thiz);
    if(0 > ret){
        LOGE("[%s] jni_call_method->remove Errors", __FUNCTION__);
        return;
    }
    LOGD("[%s] jni_call_method->remove ok", __FUNCTION__);

    ret = jni_call_method(&result, env, objLoadedApk, "android/app/LoadedApk",
                          "makeApplication",
                          "(ZLandroid/app/Instrumentation;)Landroid/app/Application;"
                            ,false, nullptr);
    if(0 > ret){
        LOGE("[%s] jni_call_method->makeApplication Errors", __FUNCTION__);
        return;
    }
    jobject objNewApp = result.l;
    LOGD("[%s] jni_call_method->makeApplication ok objNewApp:%p", __FUNCTION__, objNewApp);

    jni_call_method(&result, env, objNewApp, "android/app/Application","onCreate","()V");
    if(0 > ret){
        LOGE("[%s] jni_call_method->onCreate Errors", __FUNCTION__);
        return;
    }
    LOGD("[%s] jni_call_method->onCreate ok", __FUNCTION__);

    return;
}


extern "C"
JNIEXPORT void JNICALL
Java_org_test_androidpack1_MyApp_onCreate(JNIEnv *env, jobject thiz)
{
    // TODO: implement onCreate()
    int ret = -1;
    jvalue result = {0};
    ret = jni_call_novirtual_method(&result, env, thiz,
                                    "android/app/Application",
                                    "onCreate",
                                    "()V");
    if(0 > ret){
        LOGE("[%s] super.onCreate() Errors", __FUNCTION__);
        return;
    }
    LOGD("[%s] super.onCreate() Ok", __FUNCTION__);

    //调用被加壳dex的Application的初始化方法 attachBaseContext、onCreate
    replace_application(env, thiz);

    LOGD("[%s] Application.onCreate() over", __FUNCTION__);
}


static void replace_classloder(JNIEnv *env, jobject thiz, jobject base)
{
    int ret = -1;
    jvalue result = {0};

    char dexPath[260] = {0};
    char odexDir[260] = {0};
    char nativeLibraryDir[260] = {0};
    char dataDir[260] = {0};
    char dexDir[260] = {0};
    char libdexPath[260] = {0};
    char cmd[260] = {0};

    //获取各种路径
    ret = jni_call_method(&result, env, thiz, "android/content/ContextWrapper",
                          "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
    if(0 > ret){
        LOGE("[%s] jni_call_method->getApplicationInfo Errors", __FUNCTION__);
        return;
    }
    jobject objApplicationInfo = result.l;
    LOGD("[%s] jni_call_method->getApplicationInfo Ok, objApplicationInfo:%p", __FUNCTION__, objApplicationInfo);

    ret = jni_get_field(&result, env, objApplicationInfo, "android/content/pm/ApplicationInfo", "dataDir", "Ljava/lang/String;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->dataDir Errors", __FUNCTION__);
        return;
    }
    jobject objDataDir = result.l;
    jni_jstring_2_cstring(dataDir, env, objDataDir);
    LOGD("[%s] jni_get_field->dataDir Ok, objDataDir:%p, dataDir:%s", __FUNCTION__, objDataDir, dataDir);

    ret = jni_get_field(&result, env, objApplicationInfo, "android/content/pm/ApplicationInfo", "nativeLibraryDir", "Ljava/lang/String;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->nativeLibraryDir Errors", __FUNCTION__);
        return;
    }
    jobject objNativeLibraryDir = result.l;
    jni_jstring_2_cstring(nativeLibraryDir, env, objNativeLibraryDir);
    LOGD("[%s] jni_get_field->nativeLibraryDir Ok, objNativeLibraryDir:%p, nativeLibraryDir:%s", __FUNCTION__, objNativeLibraryDir, nativeLibraryDir);

    sprintf(odexDir, "%s/odex", dataDir);
    if (mkdir(odexDir, 0777) < 0 && errno != EEXIST) {
        LOGE("[%s] mkdir:%s error:%s", __FUNCTION__, odexDir, strerror(errno));
    }
    else {
        LOGD("[%s] mkdir:%s ok", __FUNCTION__, odexDir);
    }

    sprintf(dexDir, "%s/dex", dataDir);
    if (mkdir(dexDir, 0777) < 0 && errno != EEXIST) {
        LOGE("[%s] mkdir:%s error:%s", __FUNCTION__, dexDir, strerror(errno));
    }
    else {
        LOGD("[%s] mkdir:%s ok", __FUNCTION__, dexDir);
    }

    //将dex文件释放出来
    sprintf(dexPath, "%s/classes.dex", dexDir);
    sprintf(libdexPath, "%s/libdex.so", nativeLibraryDir);
    LOGD("[%s] dexPath:%s", __FUNCTION__, dexPath);
    LOGD("[%s] libdexPath:%s", __FUNCTION__, libdexPath);

    sprintf(cmd, "cp %s %s", libdexPath, dexPath);
    system(cmd);

    //动态加载dex文件
    //classLoader = new DexClassLoader(dexFile.getAbsolutePath(), odexDir, nativeLibraryDir, DexClassLoader.getSystemClassLoader());
    //java.lang.ClassLoader public static ClassLoader getSystemClassLoader()
    ret = jni_call_method(&result, env, nullptr, "java/lang/ClassLoader",
                          "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
    if(0 > ret){
        LOGE("[%s] jni_call_method->getSystemClassLoader Errors", __FUNCTION__);
        return;
    }
    jobject objSystemClassLoader = result.l;
    LOGD("[%s] jni_call_method->getSystemClassLoader Ok, objSystemClassLoader:%p", __FUNCTION__, objSystemClassLoader);

    jobject objDexClassLoader = jni_new_object(env, "dalvik/system/DexClassLoader", "<init>",
                                               "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V",
                                               env->NewStringUTF(dexPath),
                                               env->NewStringUTF(dexDir),
                                               env->NewStringUTF(nativeLibraryDir),
                                               objSystemClassLoader);
    if(nullptr == objDexClassLoader){
        LOGE("[%s] jni_new_object->DexClassLoader Errors", __FUNCTION__);
        return;
    }
    LOGD("[%s] jni_new_object->DexClassLoader Ok, objDexClassLoader:%p", __FUNCTION__, objDexClassLoader);

    //替换classLoader
    //android.content.ContextWrapper ==> ContextImpl mBase
    //android.app.ContextImpl ==> LoadedApk mPackageInfo
    //android.app.LoadedApk ==> mClassLoader
    ret = jni_get_field(&result, env, base, "android/app/ContextImpl", "mPackageInfo", "Landroid/app/LoadedApk;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->mPackageInfo Errors", __FUNCTION__);
        return;
    }
    jobject objPackageInfo = result.l;
    LOGD("[%s] jni_get_field->mPackageInfo Ok, objmPackageInfo:%p", __FUNCTION__, objPackageInfo);

    jvalue value_loader = {0};
    value_loader.l = objDexClassLoader;
    ret = jni_set_field(env, objPackageInfo, "android/app/LoadedApk", "mClassLoader", "Ljava/lang/ClassLoader;", value_loader);
    if(0 > ret){
        LOGE("[%s] jni_get_field->mClassLoader Errors", __FUNCTION__);
        return;
    }
    LOGD("[%s] jni_get_field->mClassLoader Ok", __FUNCTION__);


    //释放对象引用
    env->DeleteLocalRef(objApplicationInfo);
    env->DeleteLocalRef(objDataDir);
    env->DeleteLocalRef(objNativeLibraryDir);
    env->DeleteLocalRef(objSystemClassLoader);
    env->DeleteLocalRef(objDexClassLoader);
    env->DeleteLocalRef(objPackageInfo);
}


extern "C"
JNIEXPORT void JNICALL
Java_org_test_androidpack1_MyApp_attachBaseContext(JNIEnv *env, jobject thiz, jobject base)
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

    g_context = env->NewGlobalRef(base);

    //动态加载dex文件，并替换classLoader
    replace_classloder(env, thiz, base);
}


