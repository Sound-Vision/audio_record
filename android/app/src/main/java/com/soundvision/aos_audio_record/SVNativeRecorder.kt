/*
 * Copyright (c) 2024 声知视界 All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree.
 */
package com.soundvision.aos_audio_record

import android.util.Log
import com.soundvision.aos_audio_record.common.IAudioRecorder
import com.soundvision.aos_audio_record.common.*
import java.io.File

class SVNativeRecorder private constructor() : IAudioRecorder{

    private val tag = "SVNativeRecorder"

    companion object {
        val instance: SVNativeRecorder by lazy {
            SVNativeRecorder()
        }
    }

    init {
        System.loadLibrary("audio_record")
    }

    override fun initRecording(sampleRate: Int, channel: Int): Int {
        val dir = context?.filesDir
        Log.i(this.tag, "dir:${dir}")
        assert(dir != null) { "Please set application."}
        val svDir = File(dir, "sv_recorder")
        if (svDir.exists().not()) {
           val result = svDir.mkdirs()
           assert(result) { Log.w(tag, "mkdir sv_recorder failed.")}
        }
        val fileName = "_" + System.currentTimeMillis() + "_.pcm"
        val file = File(svDir, fileName)
        assert(file.createNewFile()) { Log.w(tag, "create .pcm file failed.") }
        Log.i(this.tag, "fileName: ${file.absolutePath}")

        set_record_type(0, file.absolutePath)
        return int_recording(sampleRate, channel)
    }

    override fun startRecording(): Int {
        return start_recording()
    }

    override fun stopRecording(): Int {
        return stop_recording()
    }

    override fun release(): Int {
        return release_recording()
    }

    external fun set_record_type(type: Int, filePath: String)
    external fun int_recording(sample_rate: Int, channel: Int): Int
    external fun start_recording(): Int
    external fun stop_recording(): Int
    external fun release_recording(): Int
}