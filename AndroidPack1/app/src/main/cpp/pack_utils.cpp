//
// Created by huangchen on 2023/1/6.
//

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "pack_utils.h"
#include "jni_utils.h"


static char dexDir[260] = {0};
static char dexPath[260] = {0};
static char odexDir[260] = {0};
static char nativeLibraryDir[260] = {0};
static jobject g_ObjDexClassLoader = nullptr;


static int pack_get_needed_path(JNIEnv *env, jobject thiz)
{
    int ret = -1;
    jvalue result = {0};
    char dataDir[260] = {0};

    //获取各种路径
    ret = jni_call_method(&result, env, thiz, "android/content/ContextWrapper",
                          "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
    if(0 > ret){
        LOGE("[%s] jni_call_method->getApplicationInfo Errors", __FUNCTION__);
        return -1;
    }
    jobject objApplicationInfo = result.l;
    LOGD("[%s] jni_call_method->getApplicationInfo Ok, objApplicationInfo:%p", __FUNCTION__, objApplicationInfo);

    ret = jni_get_field(&result, env, objApplicationInfo, "android/content/pm/ApplicationInfo", "dataDir", "Ljava/lang/String;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->dataDir Errors", __FUNCTION__);
        return -1;
    }
    jobject objDataDir = result.l;
    jni_jstring_2_cstring(dataDir, env, objDataDir);
    LOGD("[%s] jni_get_field->dataDir Ok, objDataDir:%p, dataDir:%s", __FUNCTION__, objDataDir, dataDir);

    ret = jni_get_field(&result, env, objApplicationInfo, "android/content/pm/ApplicationInfo", "nativeLibraryDir", "Ljava/lang/String;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->nativeLibraryDir Errors", __FUNCTION__);
        return -1;
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

    env->DeleteLocalRef(objApplicationInfo);
    env->DeleteLocalRef(objDataDir);
    env->DeleteLocalRef(objNativeLibraryDir);
    return 0;
}

static void pack_release_dex()
{
    char libdexPath[260] = {0};
    char cmd[260] = {0};

    //将dex文件释放出来
    sprintf(dexPath, "%s/classes.dex", dexDir);
    sprintf(libdexPath, "%s/libdex.so", nativeLibraryDir);
    LOGD("[%s] dexPath:%s", __FUNCTION__, dexPath);
    LOGD("[%s] libdexPath:%s", __FUNCTION__, libdexPath);

    sprintf(cmd, "cp %s %s", libdexPath, dexPath);
    system(cmd);
}

static int pack_load_dex_from_file(JNIEnv *env)
{
    int ret = -1;
    jvalue result = {0};

    /*
     * 动态加载dex文件
     * classLoader = new DexClassLoader(dexPath, odexDir, nativeLibraryDir,
     *                                  DexClassLoader.getSystemClassLoader());
     * java.lang.ClassLoader public static ClassLoader getSystemClassLoader()
     * */
    ret = jni_call_method(&result, env, nullptr, "java/lang/ClassLoader",
                          "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
    if(0 > ret){
        LOGE("[%s] jni_call_method->getSystemClassLoader Errors", __FUNCTION__);
        return -1;
    }
    jobject objSystemClassLoader = result.l;
    LOGD("[%s] jni_call_method->getSystemClassLoader Ok, objSystemClassLoader:%p", __FUNCTION__, objSystemClassLoader);

    jobject objDexClassLoader = jni_new_object(env, "dalvik/system/DexClassLoader", "<init>",
                                               "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V",
                                               env->NewStringUTF(dexPath),
                                               env->NewStringUTF(odexDir),
                                               env->NewStringUTF(nativeLibraryDir),
                                               objSystemClassLoader);
    if(nullptr == objDexClassLoader){
        LOGE("[%s] jni_new_object->DexClassLoader Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] jni_new_object->DexClassLoader Ok, objDexClassLoader:%p", __FUNCTION__, objDexClassLoader);

    g_ObjDexClassLoader = env->NewGlobalRef(objDexClassLoader);

    env->DeleteLocalRef(objSystemClassLoader);
    env->DeleteLocalRef(objDexClassLoader);
    return 0;
}

static int pack_replace_classLoader_impl(JNIEnv *env, jobject thiz, jobject objContext)
{
    int ret = -1;
    jvalue result = {0};

    /*
     * 替换classLoader
     * android.app.ContextImpl ==> LoadedApk mPackageInfo
     * android.app.LoadedApk ==> mClassLoader
     * */

    ret = jni_get_field(&result, env, objContext, "android/app/ContextImpl", "mPackageInfo", "Landroid/app/LoadedApk;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->mPackageInfo Errors", __FUNCTION__);
        return -1;
    }
    jobject objPackageInfo = result.l;
    LOGD("[%s] jni_get_field->mPackageInfo Ok, objmPackageInfo:%p", __FUNCTION__, objPackageInfo);

    jvalue value_loader = {0};
    value_loader.l = g_ObjDexClassLoader;
    ret = jni_set_field(env, objPackageInfo, "android/app/LoadedApk", "mClassLoader", "Ljava/lang/ClassLoader;", value_loader);
    if(0 > ret){
        LOGE("[%s] jni_get_field->mClassLoader Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] jni_get_field->mClassLoader Ok", __FUNCTION__);

    //释放对象引用
    env->DeleteLocalRef(objPackageInfo);
    return 0;
}


int pack_android_pack01_replace_classLoader(JNIEnv *env, jobject thiz, jobject objContext)
{
    if(pack_get_needed_path(env, thiz) < 0){
        return -1;
    }
    pack_release_dex();
    if(pack_load_dex_from_file(env) < 0){
        return -1;
    }
    if(pack_replace_classLoader_impl(env, thiz, objContext) < 0){
        return -1;
    }

    return 0;
}


int pack_replace_application(JNIEnv *env, jobject thiz, jobject objContext)
{
    int ret = -1;
    jvalue result = {0};
    jvalue value_set = {0};

    ret = jni_call_method(&result, env, thiz, "android/content/ContextWrapper",
                          "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
    if(0 > ret){
        LOGE("[%s] jni_call_method->getApplicationInfo Errors", __FUNCTION__);
        return -1;
    }
    jobject objApplicationInfo = result.l;
    LOGD("[%s] jni_call_method->getApplicationInfo Ok, objApplicationInfo:%p", __FUNCTION__, objApplicationInfo);

    jobject objClassName = env->NewStringUTF("org.test.dest.DestApp");
    value_set.l = objClassName;
    ret = jni_set_field(env, objApplicationInfo, "android/content/pm/ApplicationInfo","className", "Ljava/lang/String;", value_set);
    if(0 > ret){
        LOGE("[%s] jni_set_field->className Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] jni_set_field->className Ok", __FUNCTION__);

    ret = jni_get_field(&result, env, objContext, "android/app/ContextImpl", "mPackageInfo", "Landroid/app/LoadedApk;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->mPackageInfo Errors", __FUNCTION__);
        return -1;
    }
    jobject objLoadedApk = result.l;
    LOGD("[%s] jni_get_field->mPackageInfo Ok, objLoadedApk:%p", __FUNCTION__, objLoadedApk);

    value_set.l = nullptr;
    ret = jni_set_field(env, objLoadedApk, "android/app/LoadedApk","mApplication", "Landroid/app/Application;", value_set);
    if(0 > ret){
        LOGE("[%s] jni_set_field->mApplication Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] jni_set_field->mApplication Ok", __FUNCTION__);


    ret = jni_get_field(&result, env, objLoadedApk, "android/app/LoadedApk", "mActivityThread", "Landroid/app/ActivityThread;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->mActivityThread Errors", __FUNCTION__);
        return -1;
    }
    jobject objActivityThread = result.l;
    LOGD("[%s] jni_get_field->mActivityThread Ok, objActivityThread:%p", __FUNCTION__, objActivityThread);


    ret = jni_get_field(&result, env, objActivityThread, "android/app/ActivityThread", "mAllApplications", "Ljava/util/ArrayList;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->mAllApplications Errors", __FUNCTION__);
        return -1;
    }
    jobject objAllApplications = result.l;
    LOGD("[%s] jni_get_field->mAllApplications Ok, objAllApplications:%p", __FUNCTION__, objAllApplications);


    ret = jni_call_method(&result, env, objAllApplications, "java/util/ArrayList", "remove", "(Ljava/lang/Object;)Z", thiz);
    if(0 > ret){
        LOGE("[%s] jni_call_method->remove Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] jni_call_method->remove ok", __FUNCTION__);

    ret = jni_call_method(&result, env, objLoadedApk, "android/app/LoadedApk", "makeApplication",
                          "(ZLandroid/app/Instrumentation;)Landroid/app/Application;", false, nullptr);
    if(0 > ret){
        LOGE("[%s] jni_call_method->makeApplication Errors", __FUNCTION__);
        return -1;
    }
    jobject objNewApp = result.l;
    LOGD("[%s] jni_call_method->makeApplication ok objNewApp:%p", __FUNCTION__, objNewApp);

    jni_call_method(&result, env, objNewApp, "android/app/Application", "onCreate", "()V");
    if(0 > ret){
        LOGE("[%s] jni_call_method->onCreate Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] jni_call_method->onCreate ok", __FUNCTION__);

    //释放对象引用
    env->DeleteLocalRef(objApplicationInfo);
    env->DeleteLocalRef(objClassName);
    env->DeleteLocalRef(objLoadedApk);
    env->DeleteLocalRef(objActivityThread);
    env->DeleteLocalRef(objAllApplications);
    env->DeleteLocalRef(objNewApp);

    return 0;
}

