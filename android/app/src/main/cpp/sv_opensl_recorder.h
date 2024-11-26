/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */
#ifndef AOS_AUDIO_RECORD_SV_OPENSL_RECORDER_H
#define AOS_AUDIO_RECORD_SV_OPENSL_RECORDER_H

#include "log.h"
#include "sv_common.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace sv_recorder {

inline const char* GetSLErrorString(size_t code) {
  static const char* sl_error_strings[] = {
          "SL_RESULT_SUCCESS",                 // 0
          "SL_RESULT_PRECONDITIONS_VIOLATED",  // 1
          "SL_RESULT_PARAMETER_INVALID",       // 2
          "SL_RESULT_MEMORY_FAILURE",          // 3
          "SL_RESULT_RESOURCE_ERROR",          // 4
          "SL_RESULT_RESOURCE_LOST",           // 5
          "SL_RESULT_IO_ERROR",                // 6
          "SL_RESULT_BUFFER_INSUFFICIENT",     // 7
          "SL_RESULT_CONTENT_CORRUPTED",       // 8
          "SL_RESULT_CONTENT_UNSUPPORTED",     // 9
          "SL_RESULT_CONTENT_NOT_FOUND",       // 10
          "SL_RESULT_PERMISSION_DENIED",       // 11
          "SL_RESULT_FEATURE_UNSUPPORTED",     // 12
          "SL_RESULT_INTERNAL_ERROR",          // 13
          "SL_RESULT_UNKNOWN_ERROR",           // 14
          "SL_RESULT_OPERATION_ABORTED",       // 15
          "SL_RESULT_CONTROL_LOST",            // 16
  };

  if (code >= arraysize(sl_error_strings)) {
    return "SL_RESULT_UNKNOWN_ERROR";
  }
  return sl_error_strings[code];
}

class SVOpenSLRecorder : public ISVNativeRecorder {

  public:
    using Ptr = std::shared_ptr<SVOpenSLRecorder>;
    explicit SVOpenSLRecorder(std::string file_path);
    ~SVOpenSLRecorder();
    int InitRecording(int sample_rate, int channel) override;
    int StartRecording() override;
    int StopRecording() override;
    int Release() override;

  private:
    SV_RESULT CreateEngine();
    void ReadBufferQueue();
    void DestroyAudioRecorder();
    static SLuint32 GetSamplePerSec(int sample_rate);
    static SLuint32 GetChannelMask(int channels);
    static void BufferQueueCallBack(SLAndroidSimpleBufferQueueItf bq, void* context);

  private:
    size_t buffer_len_;
    FILE* file_;

  private:
    SLObjectItf sl_object_;
    SLEngineItf sl_engine_;
    SLObjectItf sl_record_obj_;
    SLRecordItf sl_record_;
    SLAndroidSimpleBufferQueueItf record_buffer_queue_;
    std::unique_ptr<std::unique_ptr<SLint16[]>[]> audio_buffers_;
};

}




#endif //AOS_AUDIO_RECORD_SV_OPENSL_RECORDER_H
