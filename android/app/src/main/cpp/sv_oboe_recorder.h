/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */
#ifndef AOS_AUDIO_RECORD_SV_OBOE_RECORDER_H
#define AOS_AUDIO_RECORD_SV_OBOE_RECORDER_H
#include <oboe/Oboe.h>
#include "sv_common.h"

namespace sv_recorder {

class SVOboeRecorder : public ISVNativeRecorder, public oboe::AudioStreamDataCallback {

public:
  explicit SVOboeRecorder(std::string file_path);
  ~SVOboeRecorder();
  int InitRecording(int sample_rate, int channel) override;
  int StartRecording() override;
  int StopRecording() override;
  int Release() override;

private:
  oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) override;
  void DestroyRecorder();

private:
  oboe::AudioStreamBuilder builder;
  std::shared_ptr<oboe::AudioStream> mStream;
  FILE* file_;
  bool initialized_;
  bool recording_;
};

}


#endif //AOS_AUDIO_RECORD_SV_OBOE_RECORDER_H
