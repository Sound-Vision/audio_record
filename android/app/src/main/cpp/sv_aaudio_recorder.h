/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */
#ifndef AOS_AUDIO_RECORD_SV_AAUDIO_RECORDER_H
#define AOS_AUDIO_RECORD_SV_AAUDIO_RECORDER_H

#include "sv_common.h"
#include <aaudio/AAudio.h>

namespace sv_recorder {

class SVAAudioRecorder : public ISVNativeRecorder{

public:
    explicit SVAAudioRecorder(std::string file_path);
    ~SVAAudioRecorder();
    int InitRecording(int sample_rate, int channel) override;
    int StartRecording() override;
    int StopRecording() override;
    int Release() override;

private:
    void DestroyRecorder();

public:
    static aaudio_data_callback_result_t AVDataCallback(AAudioStream *stream, void *userData, void *audioData, int32_t numFrames);
    static void AVErrorCallback(AAudioStream *stream, void *userData, aaudio_result_t error);

private:
    AAudioStreamBuilder *builder_;
    AAudioStream* stream_;
    bool initialized_;
    bool recording_;
    FILE* file_;
};

}



#endif //AOS_AUDIO_RECORD_SV_AAUDIO_RECORDER_H
