/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */
package com.soundvision.aos_audio_record.common

const val SV_SAMPLE_RATE: Int = 44100
const val SV_CHANNEL_NUM: Int = 2

const val SV_BITS_PER_SAMPLE = 16
const val SV_CALLBACK_BUFFER_SIZE_MS = 10
const val SV_BUFFERS_PER_SECOND = 1000 / SV_CALLBACK_BUFFER_SIZE_MS

const val SV_BUFFER_SIZE_FACTOR = 2

const val SV_AUDIO_THREAD_JOIN_TIMEOUT_MS = 2000L

const val SV_REQUEST_AUDIO_RECORD_PERMISSION_CODE = 10000

enum class ErrorCode {
    SV_NO_ERROR,
    SV_INIT_ERROR,
    SV_STATE_ERROR,
    SV_START_ERROR,
    SV_STOP_ERROR,
    SV_RELEASE_ERROR
}