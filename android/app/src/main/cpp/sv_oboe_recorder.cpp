/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */
#include "sv_oboe_recorder.h"
#include <cassert>
#include "log.h"

namespace sv_recorder {

using namespace oboe;

SVOboeRecorder::SVOboeRecorder(std::string file_path):
initialized_(false), recording_(false) {
  AV_LOGI("=== SVOboeRecorder CreateBuilder ===");
  file_ = fopen( file_path.c_str(), "wb");
}

SVOboeRecorder::~SVOboeRecorder() {
  AV_LOGI("=== SVOboeRecorder Release Recorder ====");
  DestroyRecorder();
  if(file_) {
    fclose(file_);
    file_ = nullptr;
  }
}

int SVOboeRecorder::InitRecording(int sample_rate, int channel) {

  if (initialized_) {
    AV_LOGI("oboe recorder has init already.");
    return SV_RESULT::SV_NO_ERROR;
  }

  builder.setDeviceId(0); //Get value from Java AudioManager.
  builder.setDirection(Direction::Input);
  builder.setPerformanceMode(PerformanceMode::LowLatency);
  builder.setSharingMode(SharingMode::Shared);
  builder.setFormat(AudioFormat::I16);
  builder.setChannelCount(channel);
  builder.setSampleRate(sample_rate);
  builder.setDataCallback(this);

  Result result = builder.openStream(mStream);
  if (result != Result::OK) {
    AV_LOGE("InitRecording openStream error:%s", convertToText(result));
    return SV_RESULT::SV_INIT_ERROR;
  }

  initialized_ = true;
  return SV_RESULT::SV_NO_ERROR;
}

int SVOboeRecorder::StartRecording() {

  if (!initialized_ ) {
    AV_LOGE("StartRecording failed, state invalid.");
    return SV_RESULT::SV_STATE_ERROR;
  }

  if (recording_) {
    AV_LOGW("Oboe has start recording already.");
    return SV_RESULT::SV_START_RECORDING_ERROR;
  }

  Result result = mStream->requestStart();
  if (result != Result::OK) {
    AV_LOGE("StartRecording requestStart error:%s", convertToText(result));
    return SV_RESULT::SV_START_RECORDING_ERROR;
  }

  recording_ = true;
  return SV_RESULT::SV_NO_ERROR;
}

int SVOboeRecorder::StopRecording() {

  Result result = mStream->requestStop();
  if (result != Result::OK) {
    AV_LOGE("StopRecording requestStop error:%s", convertToText(result));
    return SV_RESULT::SV_STOP_ERROR;
  }

  recording_ = false;
  return SV_RESULT::SV_NO_ERROR;
}

int SVOboeRecorder::Release() {
  DestroyRecorder();
  return SV_RESULT::SV_NO_ERROR;
}

void SVOboeRecorder::DestroyRecorder() {
  Result result = mStream->close();
  if (result != Result::OK) {
    AV_LOGE("oboe stream close error:%s", convertToText(result));
  }
  mStream = nullptr;
  initialized_ = false;
  recording_ = false;
}

oboe::DataCallbackResult
SVOboeRecorder::onAudioReady(oboe::AudioStream *oboeStream, void *audioData,
                             int32_t numFrames) {
  AV_LOGI("numFrames: %d", numFrames);
  int32_t data_len = oboeStream->getChannelCount() * numFrames * 2;
  if(file_) {
    size_t write_len = fwrite(audioData, 1, data_len, file_);
    AV_LOGI("Write into file data len:%d", write_len);
  }
  return oboe::DataCallbackResult::Continue;
}

}