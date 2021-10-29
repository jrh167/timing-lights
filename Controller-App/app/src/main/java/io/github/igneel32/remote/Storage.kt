package io.github.igneel32.remote

data class Storage (
    var targetMaxTime: Int,
    var matchplayMaxTime: Int,
    var targetWarnTime: Int,
    var matchplayWarnTime: Int,
    var autoToggleDetail: Boolean,
    var matchplayNumEnds: Int,
    var equipFailTime: Int
)