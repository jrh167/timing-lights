package io.github.igneel32.remote.serial

/**
 * Data class for the system's state.
 */
class SerialState {

    // struct State {
    //   bool countdown;    // If the unit should count down the seconds
    //   byte detail;       // The detail being displayed (none, A/B, or C/D)
    //   byte colour;       // The colour on the traffic light (red, amber, or green)
    //   bool timeEnabled;  // If the time should be displayed on the panel
    //   int time;          // The time to display
    //   int numBeeps;      // Number of beeps to sound at the end of the countdown
    //};

    // X = Emergency Stop
    // Y = Matchplay
    // C = Countdown
    // D = Detail
    // c = Colour
    // T = TimeEnabled
    // XYCD DccT

    // 00 = DETAIL_OFF, 01 = DETAIL_AB, 10 = DETAIL_CD
    // 00 = RED, 01 = AMBER, 10 = GREEN

    var countdownContinues: Boolean = false
    var lastEnd: Boolean = false
    var emergencyStop: Boolean = false
    var matchplay: Boolean = false
    var countdown: Boolean = false
    var detail: Detail = Detail.OFF
    var colour: Colour = Colour.RED
    var timeEnabled: Boolean = false
    var time: Int = 0
    var startNumBeeps: Int = 0
    var endNumBeeps: Int = 0

    fun pack(): Int {
        var ret = 0
        ret = ret or ((if (countdownContinues) 1 else 0) shl 9)
        ret = ret or ((if (lastEnd) 1 else 0) shl 8)
        ret = ret or ((if (emergencyStop) 1 else 0) shl 7)
        ret = ret or ((if (matchplay) 1 else 0) shl 6)
        ret = ret or ((if (countdown) 1 else 0) shl 5)
        ret = ret or (detail.ordinal shl 3)
        ret = ret or (colour.ordinal shl 1)
        ret = ret or (if (timeEnabled) 1 else 0)
        return ret
    }

    enum class Detail {
        OFF, AB, CD
    }

    enum class Colour {
        RED, AMBER, GREEN
    }
}