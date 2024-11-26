/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */

#include <jni.h>
#include <string>
#include "sv_opensl_recorder.h"

SV_RECORD_TYPE g_record_type_ = UNDEFINED;
ISVNativeRecorder::Ptr g_recorder = nullptr;

void nativeSetRecordType(JNIEnv* env, jobject obj, jint type, jstring file_path) {

  if(g_record_type_ == UNDEFINED && g_recorder) {
    AV_LOGW("Please release pre g_recorder.");
    return;
  }

  if(g_record_type_ != UNDEFINED) {
    AV_LOGW("Please call release from Kotlin.");
    return ;
  }

  const char* c_path = env->GetStringUTFChars(file_path, nullptr);
  std::string path(c_path);
  if(type == SV_RECORD_TYPE::OPEN_SL) {
    g_recorder = std::make_shared<sv_recorder::SVOpenSLRecorder>(std::move(path));
  }
  g_record_type_ = SV_RECORD_TYPE::OPEN_SL;
  env->ReleaseStringUTFChars(file_path, c_path);
}

jint nativeInitRecording(JNIEnv* env, jobject obj, jint sample_rate, jint channels) {
  jint result = JNI_ERR;
  if(g_recorder) {
    result = g_recorder->InitRecording(sample_rate, channels);
  }
  return result;
}

jint nativeStartRecording(JNIEnv* env, jobject obj) {
  jint result = JNI_ERR;
  if(g_recorder) {
    result = g_recorder->StartRecording();
  }
  return result;
}

jint nativeStopRecording(JNIEnv* env, jobject obj) {
  jint result = JNI_ERR;
  if(g_recorder) {
    result = g_recorder->StopRecording();
  }
  return result;
}

jint nativeReleaseRecording(JNIEnv* env, jobject obj) {
  jint result = JNI_ERR;
  if(g_recorder) {
    result = g_recorder->Release();
  }
  g_recorder = nullptr;
  g_record_type_ = UNDEFINED;
  return result;
}

static JNINativeMethod gMethods[] = {
{"set_record_type", "(ILjava/lang/String;)V", (void*) nativeSetRecordType},
{"int_recording", "(II)I", (void*) nativeInitRecording},
{"start_recording", "()I", (void*) nativeStartRecording},
{"stop_recording", "()I", (void*) nativeStopRecording},
{"release_recording", "()I", (void*) nativeReleaseRecording},
};

static const char* className = "com/soundvision/aos_audio_record/SVNativeRecorder";

static int registerNativeMethods(JNIEnv* env) {
  jclass clazz;
  clazz = env->FindClass(className);

  if(!clazz) return JNI_FALSE;
  if(env->RegisterNatives(clazz, gMethods, sizeof(gMethods)/sizeof(gMethods[0])) < 0) {
      return JNI_FALSE;
  }
  return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  JNIEnv* env = nullptr;
  if(vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
      return JNI_ERR;
  }
  if(registerNativeMethods(env) != JNI_TRUE) {
    return JNI_ERR;
  }
  return JNI_VERSION_1_4;
}
