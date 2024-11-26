/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */
package com.soundvision.aos_audio_record.common

interface IAudioRecorder {

    fun initRecording(sampleRate: Int, channel: Int): Int

    fun startRecording(): Int

    fun stopRecording(): Int

    fun release(): Int

}