/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */

#ifndef AOS_AUDIO_RECORD_LOG_H
#define AOS_AUDIO_RECORD_LOG_H

#include <android/log.h>
#define TAG "av_native_record"

#define AV_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define AV_LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define AV_LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG,__VA_ARGS__)
#define AV_LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define AV_LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG,__VA_ARGS__)

#endif //AOS_AUDIO_RECORD_LOG_H
