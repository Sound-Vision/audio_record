package com.soundvision.aos_audio_record.common

interface IAudioRecorder {

    fun initRecording(sampleRate: Int, channel: Int): Int

    fun startRecording(): Int

    fun stopRecording(): Int

    fun release(): Int

}