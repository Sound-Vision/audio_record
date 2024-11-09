package com.soundvision.aos_audio_record.common

import android.app.Application
import android.os.SystemClock

var context: Application ?= null

fun joinUniterruptibly(thread: Thread, timeoutMs: Long): Boolean {
    val startTimeMs = SystemClock.elapsedRealtime()
    var timeRemainingMs = timeoutMs
    var wasInterrupted = false

    while (timeRemainingMs > 0L) {
        val it = runCatching {
            thread.join(timeRemainingMs)
        }
        if(it.isSuccess) break
        if(it.isFailure) {
            wasInterrupted = true
            val elapsedTimeMs = SystemClock.elapsedRealtime() - startTimeMs
            timeRemainingMs = timeoutMs - elapsedTimeMs
        }
    }
    if(wasInterrupted) {
        Thread.currentThread().interrupt()
    }
    return !thread.isAlive
}