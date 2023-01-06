//
// Created by huangchen on 2023/1/4.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jni.h>

#include "jni_utils.h"


JNIEnv *jni_get_jnienv(JavaVM* vm, int *attach)
{
    JNIEnv* env = nullptr;
    int status = 0;

    if (vm == nullptr) {
        return  nullptr;
    }
    *attach = 0;
    status = vm->GetEnv((void **)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED || env == nullptr) {
        status = vm->AttachCurrentThread(&env, nullptr);
        if (status < 0) {
            env = nullptr;
        } else {
            *attach = 1;
        }
    }
    return env;
}

void jni_del_jnienv(JavaVM* vm)
{
    vm->DetachCurrentThread();
}

jobject jni_get_application_object(JavaVM *vm)
{
    int attach = 0;
    jvalue result = {0};
    JNIEnv * env = nullptr;

    env = jni_get_jnienv(vm, &attach);
    if(env == nullptr && attach == 0){
        LOGE("[-] GetJNIEnv faild!");
        return nullptr;
    }

    jni_call_method(&result, env, nullptr, "android/app/ActivityThread",
                    "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject activityThreadObj = result.l;

    jni_call_method(&result, env, activityThreadObj,"android/app/ActivityThread",
                    "getApplication", "()Landroid/app/Application;");
    jobject applicationObj = result.l;

    if(attach == 1){
        jni_del_jnienv(vm);
    }
    return applicationObj;
}

void jni_jstring_2_cstring(char *out, JNIEnv *env, jobject objStr)
{
    jboolean isCopy = false;
    const char* tmpStr = env->GetStringUTFChars((jstring)objStr, &isCopy);
    if(!tmpStr || env->ExceptionCheck()){
        LOGW("[-] [%s] GetStringUTFChars Erros", __FUNCTION__);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    memcpy(out, tmpStr, strlen(tmpStr) + 1);
    env->ReleaseStringUTFChars((jstring)objStr, tmpStr);
}

static char jni_get_return_type(const char* sig)
{
    unsigned int sig_len;
    unsigned int tmp;
    sig_len = strlen(sig);
    if(!sig_len) return 0;

    for(int i = 0; i < sig_len; ++i){
        tmp = i + 1;
        if(sig[i] == ')' && tmp < sig_len){
            break;
        }
        if(sig_len == tmp) return 0;
    }
    return sig[tmp];
}

static jvalue jni_call_method_impl(JNIEnv *env, jobject objClz, jmethodID methodID, const char* sig, va_list va)
{
    jvalue result = {0};
    char retType = jni_get_return_type(sig);
    retType = retType - 'B';

    switch (retType) {
        case 0:{
            jbyte ret = env->CallByteMethodV(objClz, methodID, va);
            result.b = ret;
            break;
        }
        case 1: {
            jchar ret = env->CallCharMethodV(objClz, methodID, va);
            result.c = ret;
            break;
        }
        case 2: {
            jdouble ret = env->CallDoubleMethodV(objClz, methodID, va);
            result.d = ret;
            break;
        }
        case 4: {
            jfloat ret = env->CallFloatMethodV(objClz, methodID, va);
            result.f = ret;
            break;
        }
        case 7: {
            jint ret = env->CallIntMethodV(objClz, methodID, va);
            result.i = ret;
            break;
        }
        case 8: {
            jlong ret = env->CallLongMethodV(objClz, methodID, va);
            result.j = ret;
            break;
        }
        case 10:
        case 25: {
            jobject ret = env->CallObjectMethodV(objClz, methodID, va);
            result.l = ret;
            break;
        }
        case 17: {
            jshort ret = env->CallShortMethodV(objClz, methodID, va);
            result.s = ret;
            break;
        }
        case 20: {
            env->CallVoidMethodV(objClz, methodID, va);
            break;
        }
        case 24: {
            jboolean ret = env->CallBooleanMethodV(objClz, methodID, va);
            result.z = ret;
            break;
        }
        default:
            break;
    }

    return result;
}

static jvalue jni_call_static_method_impl(JNIEnv *env, jclass clazz, jmethodID methodID, const char* sig, va_list va)
{
    jvalue result = {0};
    char retType = jni_get_return_type(sig);
    retType = retType - 'B';

    switch (retType) {
        case 0:{
            jbyte ret = env->CallStaticByteMethodV(clazz, methodID, va);
            result.b = ret;
            break;
        }
        case 1: {
            jchar ret = env->CallStaticCharMethodV(clazz, methodID, va);
            result.c = ret;
            break;
        }
        case 2: {
            jdouble ret = env->CallStaticDoubleMethodV(clazz, methodID, va);
            result.d = ret;
            break;
        }
        case 4: {
            jfloat ret = env->CallStaticFloatMethodV(clazz, methodID, va);
            result.f = ret;
            break;
        }
        case 7: {
            jint ret = env->CallStaticIntMethodV(clazz, methodID, va);
            result.i = ret;
            break;
        }
        case 8: {
            jlong ret = env->CallStaticLongMethodV(clazz, methodID, va);
            result.j = ret;
            break;
        }
        case 10:
        case 25: {
            jobject ret = env->CallStaticObjectMethodV(clazz, methodID, va);
            result.l = ret;
            break;
        }
        case 17: {
            jshort ret = env->CallStaticShortMethodV(clazz, methodID, va);
            result.s = ret;
            break;
        }
        case 20: {
            env->CallStaticVoidMethodV(clazz, methodID, va);
            break;
        }
        case 24: {
            jboolean ret = env->CallStaticBooleanMethodV(clazz, methodID, va);
            result.z = ret;
            break;
        }
        default:
            break;
    }

    return result;
}

static jvalue jni_call_novirtual_method_impl(JNIEnv *env, jobject objClz, jclass clazz, jmethodID methodID, const char* sig, va_list va)
{
    jvalue result = {0};
    char retType = jni_get_return_type(sig);
    retType = retType - 'B';

    switch (retType) {
        case 0:{
            jbyte ret = env->CallNonvirtualByteMethodV(objClz, clazz, methodID, va);
            result.b = ret;
            break;
        }
        case 1: {
            jchar ret = env->CallNonvirtualCharMethodV(objClz, clazz, methodID, va);
            result.c = ret;
            break;
        }
        case 2: {
            jdouble ret = env->CallNonvirtualDoubleMethodV(objClz, clazz, methodID, va);
            result.d = ret;
            break;
        }
        case 4: {
            jfloat ret = env->CallNonvirtualFloatMethodV(objClz, clazz, methodID, va);
            result.f = ret;
            break;
        }
        case 7: {
            jint ret = env->CallNonvirtualIntMethodV(objClz, clazz, methodID, va);
            result.i = ret;
            break;
        }
        case 8: {
            jlong ret = env->CallNonvirtualLongMethodV(objClz, clazz, methodID, va);
            result.j = ret;
            break;
        }
        case 10:
        case 25: {
            jobject ret = env->CallNonvirtualObjectMethodV(objClz, clazz, methodID, va);
            result.l = ret;
            break;
        }
        case 17: {
            jshort ret = env->CallNonvirtualShortMethodV(objClz, clazz, methodID, va);
            result.s = ret;
            break;
        }
        case 20: {
            env->CallNonvirtualVoidMethodV(objClz, clazz, methodID, va);
            break;
        }
        case 24: {
            jboolean ret = env->CallNonvirtualBooleanMethodV(objClz, clazz, methodID, va);
            result.z = ret;
            break;
        }
        default:
            break;
    }

    return result;
}

static jvalue jni_get_field_impl(JNIEnv *env, jobject objClz, jfieldID fieldID, const char* sig)
{
    jvalue result = {0};
    char sigType;
    sigType = sig[0] - 'B';

    switch (sigType) {
        case 0:{
            jbyte ret = env->GetByteField(objClz, fieldID);
            result.b = ret;
            break;
        }
        case 1: {
            jchar ret = env->GetCharField(objClz, fieldID);
            result.c = ret;
            break;
        }
        case 2: {
            jdouble ret = env->GetDoubleField(objClz, fieldID);
            result.d = ret;
            break;
        }
        case 4: {
            jfloat ret = env->GetFloatField(objClz, fieldID);
            result.f = ret;
            break;
        }
        case 7: {
            jint ret = env->GetIntField(objClz, fieldID);
            result.i = ret;
            break;
        }
        case 8: {
            jlong ret = env->GetLongField(objClz, fieldID);
            result.j = ret;
            break;
        }
        case 10:
        case 25: {
            jobject ret = env->GetObjectField(objClz, fieldID);
            result.l = ret;
            break;
        }
        case 17: {
            jshort ret = env->GetShortField(objClz, fieldID);
            result.s = ret;
            break;
        }
        case 20: {
            break;
        }
        case 24: {
            jboolean ret = env->GetBooleanField(objClz, fieldID);
            result.z = ret;
            break;
        }
        default:
            break;
    }

    return result;
}

static jvalue jni_get_static_field_impl(JNIEnv *env, jclass clazz, jfieldID fieldID, const char* sig)
{
    jvalue result = {0};
    char sigType;
    sigType = sig[0] - 'B';

    switch (sigType) {
        case 0:{
            jbyte ret = env->GetStaticByteField(clazz, fieldID);
            result.b = ret;
            break;
        }
        case 1: {
            jchar ret = env->GetStaticCharField(clazz, fieldID);
            result.c = ret;
            break;
        }
        case 2: {
            jdouble ret = env->GetStaticDoubleField(clazz, fieldID);
            result.d = ret;
            break;
        }
        case 4: {
            jfloat ret = env->GetStaticFloatField(clazz, fieldID);
            result.f = ret;
            break;
        }
        case 7: {
            jint ret = env->GetStaticIntField(clazz, fieldID);
            result.i = ret;
            break;
        }
        case 8: {
            jlong ret = env->GetStaticLongField(clazz, fieldID);
            result.j = ret;
            break;
        }
        case 10:
        case 25: {
            jobject ret = env->GetStaticObjectField(clazz, fieldID);
            result.l = ret;
            break;
        }
        case 17: {
            jshort ret = env->GetStaticShortField(clazz, fieldID);
            result.s = ret;
            break;
        }
        case 20: {
            break;
        }
        case 24: {
            jboolean ret = env->GetStaticBooleanField(clazz, fieldID);
            result.z = ret;
            break;
        }
        default:
            break;
    }

    return result;
}

static void jni_set_field_impl(JNIEnv *env, jobject objClz, jfieldID fieldID, const char* sig, jvalue value)
{
    char sigType;
    sigType = sig[0] - 'B';

    switch (sigType) {
        case 0:{
            env->SetByteField(objClz, fieldID, value.b);
            break;
        }
        case 1: {
            env->SetCharField(objClz, fieldID, value.c);
            break;
        }
        case 2: {
            env->SetDoubleField(objClz, fieldID, value.d);
            break;
        }
        case 4: {
            env->SetFloatField(objClz, fieldID, value.f);
            break;
        }
        case 7: {
            env->SetIntField(objClz, fieldID, value.i);
            break;
        }
        case 8: {
            env->SetLongField(objClz, fieldID, value.j);
            break;
        }
        case 10:
        case 25: {
            env->SetObjectField(objClz, fieldID, value.l);
            break;
        }
        case 17: {
            env->SetShortField(objClz, fieldID, value.s);
            break;
        }
        case 20: {
            break;
        }
        case 24: {
            env->SetBooleanField(objClz, fieldID, value.z);
            break;
        }
        default:
            break;
    }
}

static void jni_set_static_field_impl(JNIEnv *env, jclass clazz, jfieldID fieldID, const char* sig, jvalue value)
{
    char sigType;
    sigType = sig[0] - 'B';

    switch (sigType) {
        case 0:{
            env->SetStaticByteField(clazz, fieldID, value.b);
            break;
        }
        case 1: {
            env->SetStaticCharField(clazz, fieldID, value.c);
            break;
        }
        case 2: {
            env->SetStaticDoubleField(clazz, fieldID, value.d);
            break;
        }
        case 4: {
            env->SetStaticFloatField(clazz, fieldID, value.f);
            break;
        }
        case 7: {
            env->SetStaticIntField(clazz, fieldID, value.i);
            break;
        }
        case 8: {
            env->SetStaticLongField(clazz, fieldID, value.j);
            break;
        }
        case 10:
        case 25: {
            env->SetStaticObjectField(clazz, fieldID, value.l);
            break;
        }
        case 17: {
            env->SetStaticShortField(clazz, fieldID, value.s);
            break;
        }
        case 20: {
            break;
        }
        case 24: {
            env->SetStaticBooleanField(clazz, fieldID, value.z);
            break;
        }
        default:
            break;
    }
}


int jni_call_novirtual_method(jvalue *result, JNIEnv *env, jobject objClz, const char* clsName, const char* methodName, const char* sig, ...)
{
    *result = {0};

    if(objClz == nullptr){
        LOGW("[-] [%s] [%s] objClz is nullptr", __FUNCTION__, clsName);
        return -1;
    }

    jclass clazz = env->FindClass(clsName);
    if(!clazz || env->ExceptionCheck()){
        LOGW("[-] [%s] [%s] FindClass Erros", __FUNCTION__, clsName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    jmethodID methodID = env->GetMethodID(clazz, methodName, sig);
    if (!methodID || env->ExceptionCheck()) {
        LOGW("[-] [%s] [%s] GetMethodID Erros", __FUNCTION__, methodName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    va_list args;
    va_start(args, sig);
    *result = jni_call_novirtual_method_impl(env, objClz, clazz, methodID, sig, args);
    va_end(args);
    if(env->ExceptionCheck()){
        LOGW("[-] [%s] [%s] CallJniMethod Erros", __FUNCTION__, methodName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    return 0;
}

int jni_call_method(jvalue *result, JNIEnv *env, jobject objClz, const char* clsName, const char* methodName, const char* sig, ...)
{
    *result = {0};

    jclass clazz = env->FindClass(clsName);
    if(!clazz || env->ExceptionCheck()){
        LOGW("[-] [%s] [%s] FindClass Erros", __FUNCTION__, clsName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    jmethodID methodID = nullptr;
    if(nullptr == objClz){
        //call static
        methodID = env->GetStaticMethodID(clazz, methodName, sig);
    } else{
        methodID = env->GetMethodID(clazz, methodName, sig);
    }
    if (!methodID || env->ExceptionCheck()) {
        LOGW("[-] [%s] [%s] GetMethodID Erros", __FUNCTION__, methodName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    va_list args;
    va_start(args, sig);
    if(nullptr == objClz){
        //call static
        *result = jni_call_static_method_impl(env, clazz, methodID, sig, args);
    } else{
        *result = jni_call_method_impl(env, objClz, methodID, sig, args);
    }
    va_end(args);
    if(env->ExceptionCheck()){
        LOGW("[-] [%s] [%s] CallJniMethod Erros", __FUNCTION__, methodName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    return 0;
}

int jni_get_field(jvalue *result, JNIEnv *env, jobject objClz, const char* clsName, const char* fieldName, const char* sig)
{
    *result = {0};

    jclass clazz = env->FindClass(clsName);
    if(!clazz || env->ExceptionCheck()){
        LOGW("[-] [%s] [%s] FindClass Erros", __FUNCTION__, clsName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    jfieldID fieldID = nullptr;
    if(nullptr == objClz){
        //static
        fieldID = env->GetStaticFieldID(clazz, fieldName, sig);
    } else{
        fieldID = env->GetFieldID(clazz, fieldName, sig);
    }
    if(!fieldID || env->ExceptionCheck()){
        LOGW("[-] [%s] [%s] GetFieldID Erros", __FUNCTION__, fieldName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    if(nullptr == objClz){
        //static
        *result = jni_get_static_field_impl(env, clazz, fieldID, sig);
    } else{
        *result = jni_get_field_impl(env, objClz, fieldID, sig);
    }
    if(env->ExceptionCheck()){
        LOGW("[-] [%s] [%s] GetField Erros", __FUNCTION__, fieldName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    return 0;
}

int jni_set_field(JNIEnv *env, jobject objClz, const char* clsName, const char* fieldName, const char* sig, jvalue value)
{
    jclass clazz = env->FindClass(clsName);
    if(!clazz || env->ExceptionCheck()){
        LOGW("[-] [%s] [%s] FindClass Erros", __FUNCTION__, clsName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    jfieldID fieldID = nullptr;
    if(nullptr == objClz){
        //static
        fieldID = env->GetStaticFieldID(clazz, fieldName, sig);
    } else{
        fieldID = env->GetFieldID(clazz, fieldName, sig);
    }
    if(!fieldID || env->ExceptionCheck()){
        LOGW("[-] [%s] [%s] GetFieldID Erros", __FUNCTION__, fieldName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    if(nullptr == objClz){
        //static
        jni_set_static_field_impl(env, clazz, fieldID, sig, value);
    } else{
        jni_set_field_impl(env, objClz, fieldID, sig, value);
    }
    if(env->ExceptionCheck()){
        LOGW("[-] [%s] [%s] GetField Erros", __FUNCTION__, fieldName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }

    return 0;
}

jobject jni_new_object(JNIEnv *env, const char* clsName, const char* methodName, const char* sig, ...)
{
    jclass clazz = env->FindClass(clsName);
    if (!clazz || env->ExceptionCheck()) {
        LOGW("[-] [%s] [%s] FindClass Erros", __FUNCTION__, clsName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }

    jmethodID methodID = env->GetMethodID(clazz, methodName, sig);
    if (env->ExceptionCheck()) {
        LOGW("[-] [%s] [%s] GetMethodID Erros", __FUNCTION__, methodName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }

    va_list args;
    va_start(args, sig);
    jobject obj = env->NewObjectV(clazz, methodID, args);
    va_end(args);
    if (!obj || env->ExceptionCheck()) {
        LOGW("[-] [%s] [%s] NewObjectV Erros", __FUNCTION__, clsName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    return obj;
}

