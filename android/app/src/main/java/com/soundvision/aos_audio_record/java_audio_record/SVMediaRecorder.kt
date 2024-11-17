package com.soundvision.aos_audio_record.java_audio_record

import android.media.MediaRecorder
import android.util.Log
import com.soundvision.aos_audio_record.common.*
import java.io.File

class SVMediaRecorder private constructor() : IAudioRecorder {

    private val tag = "SVMediaRecorder"
    private var recorder: MediaRecorder? = null
    private var fileName: String? = null

    companion object {
        val instance: SVMediaRecorder by lazy {
            SVMediaRecorder()
        }
    }

    init {
        val dir = context?.filesDir
        Log.i(this.tag, "dir:${dir}")
        assert(dir != null) { "Please set application" }
        val svDir = File(dir, "sv_recorder")
        if(svDir.exists().not()) {
            val result = svDir.mkdirs()
            assert(result) { Log.w(tag, "mkdir sv_recorder failed.")}
        }
        val fileName = "_" + System.currentTimeMillis() + "_" + ".mp4"
        val file = File(svDir, fileName)
        assert(file.createNewFile()) { Log.w(tag, "create .mp4 file failed.")}
        this.fileName = file.absolutePath
        Log.i(this.tag, "fileName: $fileName")
    }

    override fun initRecording(sampleRate: Int, channel: Int): Int {
        recorder = MediaRecorder().apply {
            setAudioSource(MediaRecorder.AudioSource.MIC)
            setOutputFormat(MediaRecorder.OutputFormat.MPEG_4)
            setAudioEncoder(MediaRecorder.AudioEncoder.AAC)
            setOutputFile(fileName)
        }
        recorder?.setOnErrorListener { mr, what, extra ->
            when (what) {
                MediaRecorder.MEDIA_ERROR_SERVER_DIED -> {
                    Log.i(this.tag, "远程服务错误，MediaRecorder无法继续工作，需要重新初始化, extra:$extra")
                    mr.reset()
                }
                MediaRecorder.MEDIA_RECORDER_ERROR_UNKNOWN -> {
                    Log.i(this.tag, "MediaRecorder 内部出现未知错误, extra:$extra")
                    mr.reset()
                }
            }
        }
        return ErrorCode.SV_NO_ERROR.ordinal
    }

    override fun startRecording(): Int {
        var result = ErrorCode.SV_NO_ERROR
        runCatching {
            recorder?.prepare()
            recorder?.start()
        }.run {
            if(isFailure) {
                exceptionOrNull()?.printStackTrace()
                result = ErrorCode.SV_START_ERROR
            }
        }
        return result.ordinal
    }

    override fun stopRecording(): Int {
        var result = ErrorCode.SV_NO_ERROR
        runCatching {
            recorder?.stop()
        }.run {
            if(isFailure) {
                exceptionOrNull()?.printStackTrace()
                result = ErrorCode.SV_STOP_ERROR
            }
        }
        return result.ordinal
    }

    override fun release(): Int {
        var result = ErrorCode.SV_NO_ERROR
        runCatching {
            recorder?.reset()
            recorder?.release()
            recorder = null
        }.run {
            if(isFailure) {
                exceptionOrNull()?.printStackTrace()
                result = ErrorCode.SV_RELEASE_ERROR
            }
        }
        return result.ordinal
    }

}