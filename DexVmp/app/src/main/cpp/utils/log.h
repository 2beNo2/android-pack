//
// Created by huangchen on 2023/1/13.
//

#ifndef DEXVMP_LOG_H
#define DEXVMP_LOG_H

#include <android/log.h>

#define DEBUG

#ifdef DEBUG
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "PACK_TEST", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "PACK_TEST", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "PACK_TEST", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  "PACK_TEST", __VA_ARGS__)
#else
#define LOGD(...)
#define LOGI(...)
#define LOGE(...)
#define LOGW(...)
#endif

#define JNIHIDE  __attribute__ ((visibility ("hidden")))

#endif //DEXVMP_LOG_H
