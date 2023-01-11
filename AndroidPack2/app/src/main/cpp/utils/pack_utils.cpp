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
#include <fcntl.h>
#include <sys/mman.h>
#include <string>
#include <dlfcn.h>
#include <vector>

#include "pack_utils.h"
#include "jni_utils.h"
#include "DexFile.h"

static char dexDir[260] = {0};
static char dexPath[260] = {0};
static char odexDir[260] = {0};
static char nativeLibraryDir[260] = {0};
static jobject g_ObjDexClassLoader = nullptr;
static int SDK_INT = -1;


typedef void* (*OPEN_MEMORY_21)(const void* base,
                                 size_t size,
                                 const std::string& location,
                                 uint32_t location_checksum,
                                 void* mem_map,
                                 std::string* error_msg);

typedef void* (*OPEN_MEMORY_23)(const void* base,
                                size_t size,
                                const std::string& location,
                                uint32_t location_checksum,
                                void* mem_map,
                                const void* oat_dex_file,
                                std::string* error_msg);


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


static int pack_get_sdk_level()
{
    if (SDK_INT > 0) {
        return SDK_INT;
    }
    char sdk[260] = {0};
    __system_property_get("ro.build.version.sdk", sdk);
    SDK_INT = atoi(sdk);
    return SDK_INT;
}

static void* pack_mmap_dex_from_file()
{
    int fd = -1;
    int ret = -1;
    char libdexPath[260] = {0};
    char dexHeaderData[112] = {0};
    DexHeader *pDexHeader = nullptr;
    void *result = nullptr;

    sprintf(libdexPath, "%s/libdex.so", nativeLibraryDir);
    LOGD("[%s] libdexPath:%s", __FUNCTION__, libdexPath);

    fd = open(libdexPath, O_RDONLY);
    if(fd < 0){
        LOGD("[%s] open error %s", __FUNCTION__ , strerror(errno));
        return result;
    }

    ret = read(fd, dexHeaderData, 0x70);
    if(ret < 0){
        LOGD("[%s] read error %s", __FUNCTION__ , strerror(errno));
        close(fd);
        return result;
    }
    lseek(fd, 0, SEEK_SET);

    pDexHeader = (DexHeader *)dexHeaderData;
    result = mmap(nullptr, pDexHeader->fileSize, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_FILE, fd, 0);
    if (nullptr == result){
        LOGD("[%s] mmap error %s", __FUNCTION__ , strerror(errno));
        close(fd);
        return result;
    }

    close(fd);
    return result;
}

static jlong pack_load_dex_21(void *base)
{
    DexHeader *pDexHeader = nullptr;
    OPEN_MEMORY_21 openMemory = nullptr;
    void* dexFile = nullptr;

    //获取OpenMemory的函数地址
    //7.0以后就不能使用dlopen了
    void* handle = dlopen("libart.so", RTLD_NOW);
    if (nullptr == handle) {
        LOGE("[%s] dlopen libart.so Error:%s", __FUNCTION__, dlerror());
        return (jlong)nullptr;
    }
    LOGD("[%s] libart.so handle:%p", __FUNCTION__, handle);

    openMemory = (OPEN_MEMORY_21)dlsym(handle, "_ZN3art7DexFile10OpenMemoryEPKhmRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPNS_6MemMapEPS9_");
    if (nullptr == openMemory) {
        LOGE("[%s] dlsym openMemory Error:%s", __FUNCTION__, dlerror());
        return (jlong)nullptr;
    }
    LOGD("[%s] openMemory addr:%p", __FUNCTION__, openMemory);

    //调用OpenMemory 加载dex到虚拟机
    std::string location("");
    pDexHeader = (DexHeader*)base;
    dexFile = (*openMemory)(base, pDexHeader->fileSize, location, pDexHeader->checksum, nullptr, nullptr);
    LOGD("[%s] openMemory dexFile:%p", __FUNCTION__, dexFile);

    std::vector<const void*> *dex_files = new std::vector<const void*>();
    dex_files->push_back(dexFile);

    return (jlong)dex_files;
}

static jobject pack_load_dex_23(void *base)
{
    DexHeader *pDexHeader = nullptr;
    OPEN_MEMORY_23 openMemory = nullptr;
    void* dexFile = nullptr;

    //获取OpenMemory的函数地址
    //7.0以后就不能使用dlopen了
    void* handle = dlopen("libart.so", RTLD_NOW);
    if (nullptr == handle) {
        LOGE("[%s] dlopen libart.so Error:%s", __FUNCTION__, dlerror());
        return nullptr;
    }
    LOGD("[%s] libart.so handle:%p", __FUNCTION__, handle);

    openMemory = (OPEN_MEMORY_23)dlsym(handle, "_ZN3art7DexFile10OpenMemoryEPKhmRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPNS_6MemMapEPKNS_10OatDexFileEPS9_");
    if (nullptr == openMemory) {
        LOGE("[%s] dlsym openMemory Error:%s", __FUNCTION__, dlerror());
        return nullptr;
    }
    LOGD("[%s] openMemory addr:%p", __FUNCTION__, openMemory);

    //调用OpenMemory 加载dex到虚拟机
    std::string location("");
    pDexHeader = (DexHeader*)base;
    dexFile = (*openMemory)(base, pDexHeader->fileSize, location, pDexHeader->checksum, nullptr, nullptr, nullptr);
    LOGD("[%s] openMemory dexFile:%p", __FUNCTION__, dexFile);

    return (jobject)dexFile;
}

static int pack_replace_mCookie_impl(JNIEnv *env, jobject objContext, jvalue mCookie)
{
    int ret = -1;
    jvalue result = {0};

    /*
     * 替换对应的mCookie，即DexFile
     * BaseDexClassLoader.pathList ==> private final DexPathList pathList;
     * dalvik.system.DexPathList.dexElements[] ==> private final Element[] dexElements;
     * DexPathList$Element ==> private final DexFile dexFile;
     * dexFile ==> private long mCookie;
     * */

    ret = jni_get_field(&result, env, objContext, "android/app/ContextImpl", "mPackageInfo", "Landroid/app/LoadedApk;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->mPackageInfo Errors", __FUNCTION__);
        return -1;
    }
    jobject objPackageInfo = result.l;
    LOGD("[%s] jni_get_field->mPackageInfo Ok, objmPackageInfo:%p", __FUNCTION__, objPackageInfo);

    ret = jni_get_field(&result, env, objPackageInfo, "android/app/LoadedApk", "mClassLoader", "Ljava/lang/ClassLoader;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->mClassLoader Errors", __FUNCTION__);
        return -1;
    }
    jobject objClassLoader = result.l;
    LOGD("[%s] jni_get_field->mClassLoader Ok objClassLoader:%p", __FUNCTION__, objClassLoader);

    ret = jni_get_field(&result, env, objClassLoader, "dalvik/system/BaseDexClassLoader", "pathList", "Ldalvik/system/DexPathList;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->pathList Errors", __FUNCTION__);
        return -1;
    }
    jobject objPathList = result.l;
    LOGD("[%s] jni_get_field->pathList Ok objPathList:%p", __FUNCTION__, objPathList);

    ret = jni_get_field(&result, env, objPathList, "dalvik/system/DexPathList", "dexElements", "[Ldalvik/system/DexPathList$Element;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->dexElements Errors", __FUNCTION__);
        return -1;
    }
    jobjectArray objDexElements = (jobjectArray)result.l;
    LOGD("[%s] jni_get_field->dexElements Ok objDexElements:%p", __FUNCTION__, objDexElements);

    jsize dexElementsCount = env->GetArrayLength(objDexElements);
    if(0 > dexElementsCount){
        LOGE("[%s] dexElementsCount Errors", __FUNCTION__);
        return -1;
    }

    jobject objDexElement = env->GetObjectArrayElement(objDexElements, 0);
    if(nullptr == objDexElement){
        LOGE("[%s] GetObjectArrayElement->objDexElement Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] GetObjectArrayElement Ok objDexElement:%p", __FUNCTION__, objDexElement);

    ret = jni_get_field(&result, env, objDexElement, "dalvik/system/DexPathList$Element", "dexFile", "Ldalvik/system/DexFile;");
    if(0 > ret){
        LOGE("[%s] jni_get_field->dexFile Errors", __FUNCTION__);
        return -1;
    }
    jobject objDexFile = result.l;
    LOGD("[%s] jni_get_field->dexFile Ok objDexFile:%p", __FUNCTION__, objDexFile);

    //修改mCookie
    switch (SDK_INT) {
        case 21:
            ret = jni_set_field(env, objDexFile, "dalvik/system/DexFile", "mCookie", "J", mCookie);
            break;
        default:
            break;
    }

    if(0 > ret){
        LOGE("[%s] jni_set_field->mCookie Errors", __FUNCTION__);
        return -1;
    }
    LOGD("[%s] jni_set_field->mCookie Ok ", __FUNCTION__);

    //释放对象引用
    env->DeleteLocalRef(objDexFile);
    env->DeleteLocalRef(objDexElement);
    env->DeleteLocalRef(objDexElements);
    env->DeleteLocalRef(objPathList);
    env->DeleteLocalRef(objClassLoader);
    env->DeleteLocalRef(objPackageInfo);
    return 0;
}


int pack_android_pack02_replace_mCookie(JNIEnv *env, jobject thiz, jobject objContext)
{
    void *dexAddr = nullptr;
    jlong mCookie_jlong = 0; //5.0
    jobject mCookie_jobject = nullptr; // SDK_INIT > 5.0
    jvalue value = {0};

    //映射dex文件到内存
    if(pack_get_needed_path(env, thiz) < 0){
        return -1;
    }
    dexAddr = pack_mmap_dex_from_file();
    if(nullptr == dexAddr){
        return -1;
    }
    LOGD("[%s] dexAddr:%p", __FUNCTION__, dexAddr);

    //内存加载dex文件到虚拟机
    pack_get_sdk_level();
    LOGD("[%s] SDK_INT:%d", __FUNCTION__, SDK_INT);
    switch (SDK_INT) {
        case 21:
        {
            mCookie_jlong = pack_load_dex_21(dexAddr);
            if(0 == mCookie_jlong){
                return -1;
            }
            value.j = mCookie_jlong;
            break;
        }
        case 23:
            mCookie_jobject = pack_load_dex_23(dexAddr);
            value.l = mCookie_jobject;
            break;

        default:
            break;
    }

    //替换对应的mCookie
    pack_replace_mCookie_impl(env, objContext, value);

    return 0;
}


