//
// Created by huangchen on 2023/1/6.
//

#ifndef ANDROIDPACK_UTILS_H
#define ANDROIDPACK_UTILS_H

#include "log.h"

JNIHIDE void replace_application(JNIEnv *env, jobject thiz, jobject objContext);
JNIHIDE void replace_classloder(JNIEnv *env, jobject thiz, jobject base);


#endif //ANDROIDPACK_UTILS_H
