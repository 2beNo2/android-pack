//
// Created by huangchen on 2023/1/6.
//

#ifndef PACK_UTILS_H
#define PACK_UTILS_H

#include "log.h"

JNIHIDE int pack_android_pack01_replace_classLoader(JNIEnv *env, jobject thiz, jobject objContext);
JNIHIDE int pack_replace_application(JNIEnv *env, jobject thiz, jobject objContext);

JNIHIDE int pack_android_pack02_replace_mCookie(JNIEnv *env, jobject thiz, jobject objContext);


#endif //PACK_UTILS_H
