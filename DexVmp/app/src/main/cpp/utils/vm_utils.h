//
// Created by huangchen on 2023/1/13.
//

#ifndef DEXVMP_VM_UTILS_H
#define DEXVMP_VM_UTILS_H

#include <jni.h>

#include "log.h"
#include "jni_utils.h"


JNIHIDE jvalue vm_Interpreter(JNIEnv *env, jclass clazz, jobjectArray args);


#endif //DEXVMP_VM_UTILS_H
