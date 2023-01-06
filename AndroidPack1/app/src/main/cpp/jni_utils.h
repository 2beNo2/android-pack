//
// Created by huangchen on 2023/1/6.
//

#ifndef ANDROIDPACK1_JNI_UTILS_H
#define ANDROIDPACK1_JNI_UTILS_H

#include "log.h"

JNIHIDE void jni_jstring_2_cstring(char *out, JNIEnv *env, jobject objStr);

JNIHIDE int jni_call_method(jvalue *result, JNIEnv *env, jobject objClz, const char* clsName, const char* methodName, const char* sig, ...);
JNIHIDE int jni_call_novirtual_method(jvalue *result, JNIEnv *env, jobject objClz, const char* clsName, const char* methodName, const char* sig, ...);
JNIHIDE int jni_get_field(jvalue *result, JNIEnv *env, jobject objClz, const char* clsName, const char* fieldName, const char* sig);
JNIHIDE int jni_set_field(JNIEnv *env, jobject objClz, const char* clsName, const char* fieldName, const char* sig, jvalue value);

JNIHIDE jobject jni_new_object(JNIEnv *env, const char* clsName, const char* methodName, const char* sig, ...);
#endif //ANDROIDPACK1_JNI_UTILS_H
