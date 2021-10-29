package io.github.igneel32.remote.ui.matchplay

import android.os.Bundle
import android.os.CountDownTimer
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.widget.SwitchCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import io.github.igneel32.remote.MainActivity
import io.github.igneel32.remote.R
import io.github.igneel32.remote.Storage
import io.github.igneel32.remote.serial.SerialCommunications
import io.github.igneel32.remote.serial.SerialState

class MatchplayFragment : Fragment() {

    private lateinit var mainActivity: MainActivity
    private lateinit var serialComms: SerialCommunications
    private lateinit var matchplayViewModel: MatchplayViewModel
    private lateinit var storage: Storage
    private lateinit var resetOnSwapSwitch: SwitchCompat

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        mainActivity = (activity as MainActivity)
        serialComms = mainActivity.serialComms
        matchplayViewModel =
            ViewModelProvider(this).get(MatchplayViewModel::class.java)
        storage = mainActivity.storage.value!!
        val root = inflater.inflate(R.layout.fragment_matchplay, container, false)

        val leftText = root.findViewById<TextView>(R.id.left_seg_text)
        leftText.text = storage.matchplayMaxTime.toString()

        val rightText = root.findViewById<TextView>(R.id.right_seg_text)
        rightText.text = storage.matchplayMaxTime.toString()

        val maxTimeLabel = root.findViewById<TextView>(R.id.max_time)
        maxTimeLabel.text = storage.matchplayMaxTime.toString()

        val warnTimeLabel = root.findViewById<TextView>(R.id.warn_time)
        warnTimeLabel.text = storage.matchplayWarnTime.toString()

        val numEndsLabel = root.findViewById<TextView>(R.id.num_swaps)
        numEndsLabel.text = storage.matchplayNumEnds.toString()

        resetOnSwapSwitch = root.findViewById(R.id.reset_on_swap)

        val startLeftBtn = root.findViewById<Button>(R.id.start_left)
        startLeftBtn.setOnClickListener {
            runStartCountdowns(leftText, SerialState.Detail.AB)
        }

        val startRightBtn = root.findViewById<Button>(R.id.start_right)
        startRightBtn.setOnClickListener {
            runStartCountdowns(rightText, SerialState.Detail.CD)
        }

        return root
    }

    private fun runStartCountdowns(segText: TextView, detail: SerialState.Detail) {
        mainActivity.setAndSendState(
            matchplay = true, countdown = true, detail = detail, colour = SerialState.Colour.AMBER,
            timeEnabled = true, time = 10, startNumBeeps = 2, endNumBeeps = 1
        )

        mainActivity.countDownNumber = 10
        mainActivity.countDownTimer = object : CountDownTimer(10000, 1000) {
            override fun onTick(millisUntilFinished: Long) {
                if (mainActivity.countDownNumber < 0) {
                    // Emergency stop pressed
                    mainActivity.countDownTimer?.cancel()
                    mainActivity.countDownTimer = null
                    return
                }

                segText.text = (--mainActivity.countDownNumber).toString()
            }

            override fun onFinish() {
                mainActivity.setAndSendState(
                    matchplay = true, countdown = true, detail = detail,
                    colour = SerialState.Colour.GREEN, timeEnabled = true,
                    time = storage.matchplayMaxTime, endNumBeeps = 3
                )

                segText.text = (storage.matchplayMaxTime).toString()
                mainActivity.countDownNumber = storage.matchplayMaxTime
                mainActivity.countDownTimer =
                    object : CountDownTimer(storage.matchplayMaxTime * 1000L, 1000) {
                        override fun onTick(millisUntilFinished: Long) {
                            if (mainActivity.countDownNumber < 0) {
                                // Emergency stop pressed
                                mainActivity.countDownTimer?.cancel()
                                mainActivity.countDownTimer = null
                                return
                            }

                            segText.text = (--mainActivity.countDownNumber).toString()
                        }

                        override fun onFinish() {
                            runCountdownLoop(segText, detail)
                        }
                    }
            }
        }.start()
    }

    private fun runCountdownLoop(segText: TextView, originalDetail: SerialState.Detail) {
//        resetOnSwapSwitch.isChecked
//        storage.matchplayNumEnds * 2 - 1

//        val oppositeDetail =
//            if (originalDetail == SerialState.Detail.AB) SerialState.Detail.CD else SerialState.Detail.AB
        // TODO: This condition will always be true until the - is changed to be adaptive
//        val detail = if ((storage.matchplayNumEnds * 2 - 1) % 2 == 1) oppositeDetail else originalDetail
//        mainActivity.setAndSendState(
//            matchplay = true, countdown = true, detail = detail,
//            colour = SerialState.Colour.GREEN, timeEnabled = true,
//            time = storage.matchplayMaxTime, endNumBeeps = 3
//        )
    }
}
