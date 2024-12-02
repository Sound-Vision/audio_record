/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */
#include "sv_aaudio_recorder.h"
#include <cassert>
#include "log.h"

namespace sv_recorder {

SVAAudioRecorder::SVAAudioRecorder(std::string file_path)
  : builder_(nullptr), stream_(nullptr), initialized_(false), recording_(false) {
  AV_LOGI("=== SVAAudioRecorder CreateBuilder ===");
  file_ = fopen( file_path.c_str(), "wb");
  assert(AAudio_createStreamBuilder(&builder_) == AAUDIO_OK);
}

SVAAudioRecorder::~SVAAudioRecorder() {
  AV_LOGI("=== SVAAudioRecorder Release Recorder ====");
  DestroyRecorder();
  if(file_) {
    fclose(file_);
    file_ = nullptr;
  }
}

int SVAAudioRecorder::InitRecording(int sample_rate, int channel) {

  //step1: set configure.
  AAudioStreamBuilder_setDeviceId(builder_, AAUDIO_UNSPECIFIED);
  AAudioStreamBuilder_setSampleRate(builder_, sample_rate);
  AAudioStreamBuilder_setChannelCount(builder_, channel);
  AAudioStreamBuilder_setFormat(builder_, AAUDIO_FORMAT_PCM_I16);
  AAudioStreamBuilder_setSharingMode(builder_, AAUDIO_SHARING_MODE_SHARED);
  AAudioStreamBuilder_setDirection(builder_, AAUDIO_DIRECTION_INPUT);
  AAudioStreamBuilder_setPerformanceMode(builder_, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
  AAudioStreamBuilder_setDataCallback(builder_, AVDataCallback, this);
  AAudioStreamBuilder_setErrorCallback(builder_, AVErrorCallback, this);

  //step2: open stream.
  auto result = AAudioStreamBuilder_openStream(builder_, &stream_);
  if (result != AAUDIO_OK) {
    AV_LOGW("InitRecording error: %d, reason: %s", result, AAudio_convertResultToText(result));
    return SV_INIT_ERROR;
  }

  initialized_ = true;
  return SV_NO_ERROR;
}

int SVAAudioRecorder::StartRecording() {

  if(!initialized_) {
    AV_LOGW("StartRecording error, aaudio not initialized.");
    return SV_STATE_ERROR;
  }

  aaudio_stream_state_t stream_state =  AAudioStream_getState(stream_);
  if (stream_state != AAUDIO_STREAM_STATE_OPEN) {
    AV_LOGW("StartRecording error,  Invalid state.");
    return SV_START_RECORDING_ERROR;
  }

  aaudio_result_t result = AAudioStream_requestStart(stream_);
  if (result != AAUDIO_OK) {
    AV_LOGW("StartRecording error:%d, reason:%s", result, AAudio_convertResultToText(result));
    return SV_START_RECORDING_ERROR;
  }
  recording_ = true;
  return SV_NO_ERROR;
}

int SVAAudioRecorder::StopRecording() {

  if(!initialized_ || !recording_) {
    AV_LOGW("StopRecording error, Invalid state.");
    return SV_STATE_ERROR;
  }

  aaudio_result_t result = AAudioStream_requestStop(stream_);
  if (result != AAUDIO_OK) {
    AV_LOGW("StopRecording error: %d, reason:%s", result, AAudio_convertResultToText(result));
    return SV_STOP_ERROR;
  }
  recording_ = false;
  initialized_ = false;
  return SV_NO_ERROR;
}

void SVAAudioRecorder::DestroyRecorder() {
  AAudioStream_close(stream_);
  stream_ = nullptr;
  recording_ = false;
  initialized_ = false;
}

int SVAAudioRecorder::Release() {

  if(!initialized_) {
    AV_LOGW("Release error, Invalid state.");
    return SV_STATE_ERROR;
  }

  DestroyRecorder();
  return SV_NO_ERROR;
}

aaudio_data_callback_result_t SVAAudioRecorder::AVDataCallback(AAudioStream *stream, void *userData, void *audioData, int32_t numFrames) {
  AV_LOGI("==== onDataCallback ====, numFrames:%d", numFrames);
  auto recorder = reinterpret_cast<SVAAudioRecorder *>(userData);

  int32_t data_len = AAudioStream_getChannelCount(stream) * numFrames * 2;
  if(recorder->file_) {
    size_t write_len = fwrite(audioData, 1, data_len, recorder->file_);
    AV_LOGI("Write into file data len:%d", write_len);
  }
  return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void SVAAudioRecorder::AVErrorCallback(AAudioStream *stream, void *userData, aaudio_result_t error) {
  AV_LOGI("=== onErrorCallback ====, error:%d", error);
}

} //namespace av_recorder


