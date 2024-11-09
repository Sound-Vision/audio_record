package com.soundvision.aos_audio_record.java_audio_record

import android.Manifest
import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder.AudioSource
import android.os.Process
import android.util.Log
import android.widget.Toast
import androidx.annotation.RequiresPermission
import com.soundvision.aos_audio_record.common.*
import java.io.File
import java.io.FileOutputStream
import java.nio.ByteBuffer
import java.util.Arrays

class SVAudioRecorder private constructor() : IAudioRecorder {

    private val tag: String = "SVAudioRecorder"
    private var mSampleRate: Int = SV_SAMPLE_RATE
    private var mChannel: Int = SV_CHANNEL_NUM

    private var audioRecord: AudioRecord? = null
    private lateinit var byteBuffer: ByteBuffer

    private var captureThread: AudioCaptureThread? = null

    companion object {
        val instance: SVAudioRecorder by lazy {
            SVAudioRecorder()
        }
    }

    private fun ensureHwBufferSize(): Int {
        val bytesPerFrame: Int = mChannel * (SV_BITS_PER_SAMPLE / 8)
        val framesPerBuffer: Int = mSampleRate / SV_BUFFERS_PER_SECOND

        byteBuffer = ByteBuffer.allocateDirect(bytesPerFrame * framesPerBuffer)
        Log.i(this.tag, "buffer.capacity: ${byteBuffer.capacity()}")

        var minBufferSize = AudioRecord.getMinBufferSize(mSampleRate, mChannel, AudioFormat.ENCODING_PCM_16BIT)
        if(minBufferSize == AudioRecord.ERROR || minBufferSize == AudioRecord.ERROR_BAD_VALUE) {
            Log.i(this.tag, "AudioRecord.getMinBufferSize failed: $minBufferSize")
        }

        if(minBufferSize < byteBuffer.capacity()) {
            minBufferSize = byteBuffer.capacity()
        }

        val bufferSizeInBytes = Math.max(SV_BUFFER_SIZE_FACTOR * minBufferSize, byteBuffer.capacity())
        Log.i(this.tag, "bufferSizeInBytes: $bufferSizeInBytes")

        return bufferSizeInBytes
    }

    private fun configurationChannelConfig(channel: Int): Int {
        var channelConfig = AudioFormat.CHANNEL_IN_MONO
        if(channel != 1) {
            channelConfig = AudioFormat.CHANNEL_IN_STEREO
        }
        return channelConfig
    }

    @RequiresPermission(Manifest.permission.RECORD_AUDIO)
    override fun initRecording(sampleRate: Int, channel: Int): Int {

        var result = ErrorCode.SV_NO_ERROR.ordinal
        mSampleRate = sampleRate
        mChannel = channel

        val mHwBufferSize = ensureHwBufferSize()
        val channelConfig = configurationChannelConfig(mChannel)
        runCatching {
            audioRecord = AudioRecord(AudioSource.VOICE_COMMUNICATION, sampleRate, channelConfig, AudioFormat.ENCODING_PCM_16BIT, mHwBufferSize)
        }.let {
            if(it.isFailure) {
                it.exceptionOrNull()?.printStackTrace()
                result = ErrorCode.SV_INIT_ERROR.ordinal
            }
        }

        if(audioRecord == null || audioRecord!!.state != AudioRecord.STATE_INITIALIZED) {
            result = ErrorCode.SV_INIT_ERROR.ordinal
        }
        return result
    }

    override fun startRecording(): Int {
        Log.i(this.tag, "startRecording.")
        if(audioRecord == null) return ErrorCode.SV_STATE_ERROR.ordinal
        if(captureThread != null) return ErrorCode.SV_INIT_ERROR.ordinal

        var result = ErrorCode.SV_NO_ERROR
        try {
            audioRecord?.startRecording()
        } catch (err: IllegalStateException) {
            err.printStackTrace()
            result = ErrorCode.SV_START_ERROR
        }
        captureThread = AudioCaptureThread()
        captureThread?.start() ?: { result = ErrorCode.SV_START_ERROR }
        return result.ordinal
    }

    override fun stopRecording(): Int {
        if(audioRecord == null || captureThread == null) {
            return ErrorCode.SV_START_ERROR.ordinal
        }
        var result = ErrorCode.SV_NO_ERROR
        runCatching {
            captureThread?.stopCapture()
            if(!joinUniterruptibly(captureThread!!, SV_AUDIO_THREAD_JOIN_TIMEOUT_MS)) {
                Log.e(this.tag, "Join audio_cap_thread timed out.")
            }
            captureThread = null
        }.let {
            if(it.isFailure)
                result = ErrorCode.SV_STOP_ERROR
        }
        return result.ordinal
    }

    override fun release(): Int {
        Log.i(this.tag, "release hw resource.")
        audioRecord?.release()
        audioRecord = null
        return ErrorCode.SV_NO_ERROR.ordinal
    }

    inner class AudioCaptureThread : Thread("audio_cap_thread") {

        private val tag: String = "AudioCaptureThread"
        @Volatile private var keepAlive: Boolean = true
        private var fos: FileOutputStream? = null

        init {
            val filesDir = context?.filesDir
            assert(filesDir != null) { "Please set application" }
            val svDir = File(filesDir, "sv_recorder")
            if(svDir.exists().not()) {
                val result = svDir.mkdirs()
                assert(result) { "mkdir sv dir failed." }
            }
            val fileName =
                "_" + mSampleRate + "_" + mChannel + "_" + System.currentTimeMillis() + "_.pcm"
            val file = File(svDir, fileName)
            file.run {
                assert(createNewFile()) { "create pcm file failed."}
                fos = FileOutputStream(this)
            }
            Toast.makeText(context, "file dir: ${file.path}", Toast.LENGTH_SHORT).show()
        }

        override fun run() {
            super.run()
            Process.setThreadPriority(Process.THREAD_PRIORITY_AUDIO)
            if(audioRecord!!.recordingState != AudioRecord.RECORDSTATE_RECORDING) {
                Log.i(this.tag, "AudioRecord is not recording state, ${audioRecord?.recordingState}")
                return
            }
            runCatching {
                while (keepAlive) {
                    val readSize = audioRecord!!.read(byteBuffer, byteBuffer.capacity())
                    if(readSize == byteBuffer.capacity()) {
                        val data = Arrays.copyOf(byteBuffer.array(), byteBuffer.capacity())
                        synchronized(this@AudioCaptureThread) {
                            fos?.write(data)
                        }
                    } else {
                        Log.e(this.tag, "AudioRecord.read failed: $readSize")
                        Log.e(this.tag, "AudioRecord.state: ${audioRecord?.state}, recordingState: ${audioRecord?.recordingState}")
                    }
                }
                runCatching {
                    audioRecord?.stop()
                }.let {
                    if(it.isFailure) {
                        Log.e(this.tag, "AudioRecord.stop failed: ${it.exceptionOrNull()?.printStackTrace()}")
                    }
                }
            }
        }

        fun stopCapture() {
            this.keepAlive = false
            synchronized(this) {
                fos = null
            }
        }
    }

}