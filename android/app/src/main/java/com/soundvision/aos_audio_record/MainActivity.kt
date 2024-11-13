package com.soundvision.aos_audio_record

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.view.View
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import com.soundvision.aos_audio_record.common.*
import com.soundvision.aos_audio_record.java_audio_record.SVMediaRecorder

class MainActivity : AppCompatActivity() {

    private val tag: String = "MainActivity"
    private lateinit var recorder: IAudioRecorder

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContentView(R.layout.activity_main)
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }
        context = application
        recorder = SVMediaRecorder.instance
        if(initRecording().not()) {
            Log.w(this.tag, "init recording failed, need request permission.")
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.RECORD_AUDIO), SV_REQUEST_AUDIO_RECORD_PERMISSION_CODE)
        }
    }

    fun startRecording(view: View) {
        recorder.startRecording()
    }

    fun stopRecording(view: View) {
        recorder.stopRecording()
    }

    private fun initRecording(): Boolean {
        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.RECORD_AUDIO
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            return false
        }
        recorder.initRecording(48000, 2)
        return true
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if(requestCode == SV_REQUEST_AUDIO_RECORD_PERMISSION_CODE) {
            if(initRecording().not()) {
                Log.e(this@MainActivity.tag, "request audio record permission failed.")
            }
        }
    }
}