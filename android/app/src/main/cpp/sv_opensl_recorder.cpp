/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */

#include "sv_opensl_recorder.h"

namespace sv_recorder {

SVOpenSLRecorder::SVOpenSLRecorder(std::string file_path)
        :sl_engine_(nullptr), sl_object_(nullptr), buffer_len_(0){
  AV_LOGI("=== SVOpenSLRecorder Constructor ====");

  file_ = fopen( file_path.c_str(), "wb");
  CreateEngine();
}

SVOpenSLRecorder::~SVOpenSLRecorder() {
  AV_LOGI("=== SVOpenSLRecorder Deconstructor ===");
  DestroyAudioRecorder();
  if(file_) {
    fclose(file_);
    file_ = nullptr;
  }
}

// this callback handler is called every time a buffer finishes recording
void SVOpenSLRecorder::BufferQueueCallBack(SLAndroidSimpleBufferQueueItf bq, void* context) {
  auto recorder = static_cast<SVOpenSLRecorder*>(context);
  if(recorder) {
    recorder->ReadBufferQueue();
  } else {
    AV_LOGE("Input context is nullptr!");
  }
}

int SVOpenSLRecorder::InitRecording(int sample_rate, int channel) {

  size_t frames_per_buffer = sample_rate / SV_BUFFERS_PER_SECOND;
  buffer_len_ = frames_per_buffer * channel;
  audio_buffers_ = std::make_unique<std::unique_ptr<SLint16[]>[]>(SV_OPENSLES_BUFFERS_LEN);
  for(int i = 0; i < SV_OPENSLES_BUFFERS_LEN; i++) {
    audio_buffers_[i].reset(new SLint16[buffer_len_]);
  }

  // 1. configure audio source
  SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE,
                                    SL_IODEVICE_AUDIOINPUT,
                                    SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
  SLDataSource audioSrc = {&loc_dev, NULL};

  // 2. configure audio sink
  SLDataLocator_AndroidSimpleBufferQueue buffer_queue = {
          SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, SV_OPENSLES_BUFFERS_LEN};
  SLDataFormat_PCM format_pcm = {
          SL_DATAFORMAT_PCM,           static_cast<SLuint32>(channel),
          GetSamplePerSec(sample_rate),          SL_PCMSAMPLEFORMAT_FIXED_16,
          SL_PCMSAMPLEFORMAT_FIXED_16, GetChannelMask(channel),
          SL_BYTEORDER_LITTLEENDIAN};
  SLDataSink audioSink = {&buffer_queue, &format_pcm};

  // 3. create audio recorder
  // (requires the RECORD_AUDIO permission)
  const SLInterfaceID id[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                              SL_IID_ANDROIDCONFIGURATION};
  const SLboolean req[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

  SLresult result = (*sl_engine_)->CreateAudioRecorder(sl_engine_, &sl_record_obj_, &audioSrc,
                                &audioSink, arraysize(id), id, req);

  if (result != SL_RESULT_SUCCESS) {
    AV_LOGW("Create AudioRecorder failed: %s", GetSLErrorString(result));
    return SV_INIT_ERROR;
  }

  //4. Configure the audio recorder. before it it realized.
  SLAndroidConfigurationItf recorder_config;
  result = (*sl_record_obj_)->GetInterface(sl_record_obj_, SL_IID_ANDROIDCONFIGURATION, &recorder_config);
  if (result != SL_RESULT_SUCCESS) {
    AV_LOGW("Get record configuration failed: %s", GetSLErrorString(result));
    return SV_INIT_ERROR;
  }

  //5. set audio source.
  SLint32 stream_type = SL_ANDROID_RECORDING_PRESET_GENERIC; // AudioSource.DEFAULT
  result = (*recorder_config)->SetConfiguration(recorder_config, SL_ANDROID_KEY_RECORDING_PRESET, &stream_type, sizeof(SLint32));
  if (result != SL_RESULT_SUCCESS) {
    AV_LOGW("Set recorder preset failed: %s", GetSLErrorString(result));
    return SV_INIT_ERROR;
  }

  //6. set audio performance.
  SLuint32 performance_mode = SL_ANDROID_PERFORMANCE_LATENCY;
  SLuint32  value_size = sizeof(SLuint32);
  result = (*recorder_config)->GetConfiguration(recorder_config, SL_ANDROID_KEY_PERFORMANCE_MODE, &value_size, &performance_mode);
  if (result != SL_RESULT_SUCCESS) {
    AV_LOGW("Set recorder performance mode failed: %s", GetSLErrorString(result));
    return SV_INIT_ERROR;
  }

  //7. Realize record object.
  result = (*sl_record_obj_)->Realize(sl_record_obj_, SL_BOOLEAN_FALSE);
  if(result != SL_RESULT_SUCCESS) {
    AV_LOGW("sl_record_obj Realize failed: %s", GetSLErrorString(result));
    return SV_INIT_ERROR;
  }

  //8. Get record interface.
  result = (*sl_record_obj_)->GetInterface(sl_record_obj_, SL_IID_RECORD, &sl_record_);
  if(result != SL_RESULT_SUCCESS) {
    AV_LOGW("sl_record_obj GetInterface SL_IID_RECORD failed: %s", GetSLErrorString(result));
    return SV_INIT_ERROR;
  }

  //9. Get BufferQueue.
  result = (*sl_record_obj_)->GetInterface(sl_record_obj_, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &record_buffer_queue_);
  if(result != SL_RESULT_SUCCESS) {
    AV_LOGW("sl_record_obj GetInterface SL_IID_ANDROIDSIMPLEBUFFERQUEUE failed: %s",
            GetSLErrorString(result));
    return SV_INIT_ERROR;
  }

  //10. Register buffer queue.
  result = (*record_buffer_queue_)->RegisterCallback(record_buffer_queue_, BufferQueueCallBack, this);
  if(result != SL_RESULT_SUCCESS) {
    AV_LOGW("record_buffer_queue RegisterCallback failed: %s", GetSLErrorString(result));
    return SV_INIT_ERROR;
  }

  return SV_NO_ERROR;
}

int SVOpenSLRecorder::StartRecording() {

  AV_LOGI("StartRecording ....");

  if(!sl_record_) {
    AV_LOGW("StartRecording sl_record_ is nullptr.");
    return SV_START_RECORDING_ERROR;
  }

  SLresult result = (*sl_record_)->SetRecordState(sl_record_, SL_RECORDSTATE_STOPPED);
  if (result != SL_RESULT_SUCCESS) {
    return SV_START_RECORDING_ERROR;
  }

  SLAndroidSimpleBufferQueueState state;
  result = (*record_buffer_queue_)->GetState(record_buffer_queue_, &state);
  if (result != SL_RESULT_SUCCESS) {
    AV_LOGW("StartRecording GetState failed: %s", GetSLErrorString(result));
    return SV_START_RECORDING_ERROR;
  }

  auto buffer_count_in_queue = state.count;
  if (buffer_count_in_queue > 0) {
    (*record_buffer_queue_)->Clear(record_buffer_queue_);
  }

  for (int i = 0; i < SV_OPENSLES_BUFFERS_LEN - buffer_count_in_queue; i++) {
    auto audio_buffer = reinterpret_cast<SLint8 *>(audio_buffers_[0].get());
    auto len = buffer_len_ * 16 / 8;
    SLresult err = (*record_buffer_queue_)->Enqueue(record_buffer_queue_, audio_buffer, len);
    if (err != SL_RESULT_SUCCESS) {
      AV_LOGW("Enqueue failed, err: %s", GetSLErrorString(err));
      return SV_START_RECORDING_ERROR;
    }
  }

  result = (*sl_record_)->SetRecordState(sl_record_, SL_RECORDSTATE_RECORDING);
  if (result != SL_RESULT_SUCCESS) {
    AV_LOGW("StartRecording SetRecordState Recording failed.");
    return SV_START_RECORDING_ERROR;
  }

  return SV_NO_ERROR;
}

int SVOpenSLRecorder::StopRecording() {

  AV_LOGI("StopRecording ...");

  if(!sl_record_ || !record_buffer_queue_) {
    return SV_STOP_ERROR;
  }

  SLresult result = (*sl_record_)->SetRecordState(sl_record_, SL_RECORDSTATE_STOPPED);
  if(result != SL_RESULT_SUCCESS) {
    AV_LOGW("StopRecording SetRecordState failed.");
    return SV_STOP_ERROR;
  }
  (*record_buffer_queue_)->Clear(record_buffer_queue_);
  DestroyAudioRecorder();
  return SV_NO_ERROR;
}

SV_RESULT SVOpenSLRecorder::CreateEngine() {

  // Create the engine object in thread safe mode.
  const SLEngineOption option[] = {
          {SL_ENGINEOPTION_THREADSAFE, static_cast<SLuint32>(SL_BOOLEAN_TRUE)}};
  SLresult result = slCreateEngine(&sl_object_, 0, option, 0, NULL, NULL);
  if(result != SL_RESULT_SUCCESS) {
    AV_LOGW("slCreateEngine failed: %s", GetSLErrorString(result));
    return SV_CRATE_ERROR;
  }

  result = (*sl_object_)->Realize(sl_object_, SL_BOOLEAN_FALSE);
  if(result != SL_RESULT_SUCCESS) {
    AV_LOGW("sl_object Realize failed: %s", GetSLErrorString(result));
    return SV_CRATE_ERROR;
  }

  result = (*sl_object_)->GetInterface(sl_object_, SL_IID_ENGINE, &sl_engine_);
  if(result != SL_RESULT_SUCCESS) {
    AV_LOGW("sl_object SL_IID_ENGINE get Interface failed: %s", GetSLErrorString(result));
    return SV_CRATE_ERROR;
  }
  return SV_NO_ERROR;
}

void SVOpenSLRecorder::ReadBufferQueue() {

  SLuint32 state;
  SLresult result = (*sl_record_)->GetRecordState(sl_record_, &state);
  if(SL_RESULT_SUCCESS != result) {
    AV_LOGW("GetRecordState failed, err: %s", GetSLErrorString(result));
    return;
  }

  if(state != SL_RECORDSTATE_RECORDING) {
    AV_LOGW("Buffer callback in non-recording state! state: %d", state);
    return;
  }

  auto audio_buffer = reinterpret_cast<SLint8 *>(audio_buffers_[0].get());
  auto len = buffer_len_ * 16 / 8;
  AV_LOGI("audio buffer len: %u", len);

  result = (*record_buffer_queue_)->Enqueue(record_buffer_queue_, audio_buffer, len);
  if(SL_RESULT_SUCCESS != result) {
    AV_LOGW("Enqueue failed: err: %s", GetSLErrorString(result));
    return;
  }

  if(file_) {
    size_t write_len = fwrite(audio_buffer, 1, len, file_);
    AV_LOGW("write into file size: %d", write_len);
  }
}

void SVOpenSLRecorder::DestroyAudioRecorder() {
  (*record_buffer_queue_)->RegisterCallback(record_buffer_queue_, nullptr, nullptr);
  sl_record_obj_ = nullptr;
  sl_record_ = nullptr;
  record_buffer_queue_ = nullptr;
}

SLuint32 SVOpenSLRecorder::GetSamplePerSec(int sample_rate) {
  SLuint32 samplePerSec = SL_SAMPLINGRATE_16;
  switch (sample_rate) {
    case 8000:
      samplePerSec = SL_SAMPLINGRATE_8;
      break;
    case 16000:
      samplePerSec = SL_SAMPLINGRATE_16;
      break;
    case 22050:
      samplePerSec = SL_SAMPLINGRATE_22_05;
      break;
    case 24000:
      samplePerSec = SL_SAMPLINGRATE_24;
      break;
    case 32000:
      samplePerSec = SL_SAMPLINGRATE_32;
      break;
    case 44100:
      samplePerSec = SL_SAMPLINGRATE_44_1;
      break;
    case 48000:
      samplePerSec = SL_SAMPLINGRATE_48;
      break;
    case 64000:
      samplePerSec = SL_SAMPLINGRATE_64;
      break;
    case 88200:
      samplePerSec = SL_SAMPLINGRATE_88_2;
      break;
    case 96000:
      samplePerSec = SL_SAMPLINGRATE_96;
      break;
    default:
      AV_LOGW("Unsupported sample rate: %d, so set default value: 16000", sample_rate);
      break;
  }
  return samplePerSec;
}

SLuint32 SVOpenSLRecorder::GetChannelMask(int channels) {
  SLuint32 channelMask = SL_SPEAKER_FRONT_CENTER;
  if(channels == 2) {
    channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
  }
  return channelMask;
}

int SVOpenSLRecorder::Release() {
  sl_record_ = nullptr;
  if(sl_record_obj_)
    (*sl_record_obj_)->Destroy(sl_record_obj_);
  sl_engine_ = nullptr;
  if(sl_object_)
    (*sl_object_)->Destroy(sl_object_);
  return SV_NO_ERROR;
}

}

