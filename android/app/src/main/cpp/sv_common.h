/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */

#ifndef AOS_AUDIO_RECORD_SV_COMMON_H
#define AOS_AUDIO_RECORD_SV_COMMON_H

#include "string"

#define arraysize(array) (sizeof(ArraySizeHelper(array)))
const size_t SV_BUFFERS_PER_SECOND = 100;
const size_t SV_OPENSLES_BUFFERS_LEN = 2;

enum SV_RESULT: int16_t {
    SV_NO_ERROR,
    SV_CRATE_ERROR,
    SV_INIT_ERROR,
    SV_START_RECORDING_ERROR,
    SV_STOP_ERROR,
    SV_STATE_ERROR
};

enum SV_RECORD_TYPE : int32_t {
    UNDEFINED = -1,
    OPEN_SL = 0,
    AAUDIO = 1,
};

template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];

class ISVNativeRecorder {
public:
    using Ptr = std::shared_ptr<ISVNativeRecorder>;
    virtual ~ISVNativeRecorder() = default;
    virtual int InitRecording(int sample_rate, int channels) = 0;
    virtual int StartRecording() = 0;
    virtual int StopRecording() = 0;
    virtual int Release() = 0;
};

#endif //AOS_AUDIO_RECORD_SV_COMMON_H
