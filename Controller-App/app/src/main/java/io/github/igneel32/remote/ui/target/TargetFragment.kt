package io.github.igneel32.remote.ui.target

import android.content.DialogInterface
import android.os.Bundle
import android.os.CountDownTimer
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.RadioGroup
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import io.github.igneel32.remote.MainActivity
import io.github.igneel32.remote.R
import io.github.igneel32.remote.Storage
import io.github.igneel32.remote.serial.SerialCommunications
import io.github.igneel32.remote.serial.SerialState

class TargetFragment : Fragment() {

    private lateinit var mainActivity: MainActivity
    private lateinit var serialComms: SerialCommunications
    private lateinit var targetViewModel: TargetViewModel
    private lateinit var storage: Storage

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        mainActivity = (activity as MainActivity)
        serialComms = mainActivity.serialComms
        targetViewModel =
            ViewModelProvider(this).get(TargetViewModel::class.java)
        storage = mainActivity.storage.value!!
        val root = inflater.inflate(R.layout.fragment_target, container, false)

        val segText = root.findViewById<TextView>(R.id.seg_text)
        segText.text = storage.targetMaxTime.toString()

        val maxTimeLabel = root.findViewById<TextView>(R.id.max_time)
        maxTimeLabel.text = storage.targetMaxTime.toString()

        val warnTimeLabel = root.findViewById<TextView>(R.id.warn_time)
        warnTimeLabel.text = storage.targetWarnTime.toString()

        val detailRadioGroup = root.findViewById<RadioGroup>(R.id.detail_group)
        val checkedRadioBtn = detailRadioGroup.checkedRadioButtonId

        val startBtn = root.findViewById<Button>(R.id.start_button)
        startBtn.setOnClickListener {
            mainActivity.serialState.countdownContinues = true
            mainActivity.serialState.emergencyStop = false
            mainActivity.serialState.matchplay = false
            mainActivity.serialState.countdown = true

            when (checkedRadioBtn) {
                R.id.radio_none -> mainActivity.serialState.detail = SerialState.Detail.OFF
                R.id.radio_ab -> mainActivity.serialState.detail = SerialState.Detail.AB
                R.id.radio_cd -> mainActivity.serialState.detail = SerialState.Detail.CD
            }

            mainActivity.serialState.colour = SerialState.Colour.AMBER
            mainActivity.serialState.timeEnabled = true
            mainActivity.serialState.time = 10
            mainActivity.serialState.startNumBeeps = 2
            mainActivity.serialState.endNumBeeps = 1
            mainActivity.serialComms.sendState()

            runCountdown(segText)
        }

        val scoreBtn = root.findViewById<Button>(R.id.score_button)
        scoreBtn.setOnClickListener {
            mainActivity.countDownTimer?.cancel()
            mainActivity.setAndSendState(
                timeEnabled = true,
                time = storage.targetMaxTime,
                endNumBeeps = 3
            )
        }

        val equipFailBtn = root.findViewById<Button>(R.id.equip_fail_btn)
        equipFailBtn.setOnClickListener {
            val builder = AlertDialog.Builder(requireContext())
            val view = inflater.inflate(R.layout.dialog_equip_fail, container, false)
            builder.setView(view)
            builder.setTitle(resources.getString(R.string.equip_fail))
            builder.setPositiveButton(R.string.start) { _, _ -> }
            builder.setNegativeButton(R.string.cancel) { dialog, _ ->
                dialog.cancel()
            }
            val alert: AlertDialog = builder.create()
            alert.show()

            val positive = alert.getButton(DialogInterface.BUTTON_POSITIVE)
            positive.setOnClickListener {
                //TODO: Implement error handling for null input
                val arrows =
                    view.findViewById<EditText>(R.id.num_arrows_to_shoot).text.toString().toInt()
                val timePer =
                    view.findViewById<EditText>(R.id.time_per_arrow).text.toString().toInt()
                runEquipmentFailurePhase(arrows, timePer, segText)
            }
        }

        return root
    }

    private fun runCountdown(segText: TextView) {
        // Begin initial 10 second countdown where archers go to the shooting line
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
                // Begin full countdown for shooting
                mainActivity.setAndSendState(
                    countdown = true, detail = mainActivity.serialState.detail,
                    colour = SerialState.Colour.GREEN, timeEnabled = true,
                    time = storage.targetMaxTime, endNumBeeps = 3
                )

                segText.text = storage.targetMaxTime.toString()
                mainActivity.countDownNumber = storage.targetMaxTime
                mainActivity.countDownTimer =
                    object : CountDownTimer(storage.targetMaxTime * 1000L, 1000) {
                        override fun onTick(millisUntilFinished: Long) {
                            if (mainActivity.countDownNumber < 0) {
                                // Emergency stop pressed
                                mainActivity.countDownTimer?.cancel()
                                mainActivity.countDownTimer = null
                                return
                            }

                            segText.text = (--mainActivity.countDownNumber).toString()
                        }

                        override fun onFinish() {}
                    }.start()
            }
        }.start()
    }

    private fun runEquipmentFailurePhase(numArrows: Int, timePerArrow: Int, segText: TextView) {
        val maxTime = numArrows * timePerArrow

        // Begin initial 10 second countdown where archers go to the shooting line
        mainActivity.countDownNumber = 10
        mainActivity.setAndSendState(
            countdownContinues = true,
            countdown = true,
            colour = SerialState.Colour.AMBER,
            timeEnabled = true, time = 10,
            startNumBeeps = 2,
            endNumBeeps = 1
        )
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
                // Begin full countdown for shooting
                mainActivity.setAndSendState(
                    countdown = true,
                    colour = SerialState.Colour.GREEN,
                    timeEnabled = true, time = maxTime,
                    endNumBeeps = 3
                )

                segText.text = maxTime.toString()
                mainActivity.countDownNumber = maxTime
                mainActivity.countDownTimer =
                    object : CountDownTimer(maxTime * 1000L, 1000) {
                        override fun onTick(millisUntilFinished: Long) {
                            if (mainActivity.countDownNumber < 0) {
                                // Emergency stop pressed
                                mainActivity.countDownTimer?.cancel()
                                mainActivity.countDownTimer = null
                                return
                            }

                            segText.text = (--mainActivity.countDownNumber).toString()
                        }

                        override fun onFinish() {}
                    }.start()
            }
        }.start()
    }
}