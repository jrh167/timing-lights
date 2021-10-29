package io.github.igneel32.remote

import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.UsbManager
import android.os.Bundle
import android.os.CountDownTimer
import android.os.Handler
import android.os.Looper
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.MutableLiveData
import androidx.navigation.NavController
import androidx.navigation.findNavController
import com.google.gson.Gson
import com.google.gson.GsonBuilder
import com.hoho.android.usbserial.driver.UsbSerialProber
import io.github.igneel32.remote.serial.SerialCommunications
import io.github.igneel32.remote.serial.SerialState
import java.io.File

// TODO: handle crashes when sending with no device attached
class MainActivity : AppCompatActivity() {

    private var usbPermission = UsbPermission.Unknown
    private val mainLooper: Handler = Handler(Looper.getMainLooper())
    private val fileName = "config.json"

    lateinit var navController: NavController
    val serialComms: SerialCommunications = SerialCommunications()
    val serialState: SerialState = serialComms.serialState

    var storage = MutableLiveData<Storage>().apply {
        value = Storage(
            DEFAULT_TARGET_MAX_TIME,
            DEFAULT_MATCHPLAY_MAX_TIME,
            DEFAULT_TARGET_WARN_TIME,
            DEFAULT_MATCHPLAY_WARN_TIME,
            DEFAULT_AUTO_TOGGLE_DETAIL,
            DEFAULT_MATCHPLAY_NUM_ENDS,
            DEFAULT_PER_ARROW_TIME
        )
    }
    var countDownTimer: CountDownTimer? = null
    var countDownNumber: Int = 0

    private val broadcastReceiver: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            if (INTENT_ACTION_GRANT_USB == intent?.action) {
                usbPermission =
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false))
                        UsbPermission.Granted else UsbPermission.Denied
                if (usbPermission == UsbPermission.Granted) {
                    initUsb()
                }
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        loadStorage()
        setContentView(R.layout.activity_main)
        navController = findNavController(R.id.nav_host_fragment_activity_main)

        val targetButton = findViewById<Button>(R.id.target_button)
        val matchplayButton = findViewById<Button>(R.id.matchplay_button)
        val settingsButton = findViewById<Button>(R.id.settings_button)
        targetButton.setOnClickListener {
            navController.navigate(R.id.navigation_target)
        }
        matchplayButton.setOnClickListener {
            navController.navigate(R.id.navigation_matchplay)
        }
        settingsButton.setOnClickListener {
            navController.navigate(R.id.navigation_settings)
        }
    }

    override fun onResume() {
        super.onResume()
        registerReceiver(broadcastReceiver, IntentFilter(INTENT_ACTION_GRANT_USB))
        if (usbPermission == UsbPermission.Unknown || usbPermission == UsbPermission.Granted) {
            mainLooper.post { initUsb() }
        }
    }

    override fun onPause() {
        super.onPause()
        serialComms.disconnect()
        unregisterReceiver(broadcastReceiver)
    }

    /**
     * Initialize connection to the radio dongle attached to the device.
     */
    private fun initUsb() {
        // Find all available drivers from attached devices.
        val manager = getSystemService(Context.USB_SERVICE) as UsbManager?
        val availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(manager)
        if (availableDrivers.isEmpty()) {
            return
        }

        // Open a connection to the first available driver.
        val driver = availableDrivers[0]
        val connection = manager!!.openDevice(driver.device)
        if (connection == null && usbPermission == UsbPermission.Unknown && !manager.hasPermission(driver.device)) {
            usbPermission = UsbPermission.Requested
            val usbPermissionIntent =
                PendingIntent.getBroadcast(this, 0, Intent(INTENT_ACTION_GRANT_USB), 0)
            manager.requestPermission(driver.device, usbPermissionIntent)
            return
        }

        if (connection == null) {
            if (!manager.hasPermission(driver.device)) {
                //TODO: handle connection failed: permission denied
                Toast.makeText(this, "Permission denied", Toast.LENGTH_LONG).show()
            } else {
                //TODO: handle connection failed: open failed  (unknown reason)
                Toast.makeText(this, "Connection failed, unknown reason", Toast.LENGTH_LONG).show()
            }
            return
        }

        serialComms.connect(driver, connection)
    }

    /**
     * Load settings from a local settings file.
     */
    private fun loadStorage() {
        val file = File(filesDir, fileName)
        if (file.exists()) {
            file.bufferedReader().use {
                storage.value = GSON.fromJson(it, Storage::class.java)
            }
        }
    }

    /**
     * Persist settings to a local settings file.
     */
    fun saveStorage() {
        val file = File(filesDir, fileName)
        if (!file.exists()) {
            file.createNewFile()
        }
        file.bufferedWriter().use {
            GSON.toJson(storage.value, it)
        }
    }

    /**
     * Set the internal state with the given values, and send the updated state.
     * If some/all values are not provided, defaults to a blank idle state.
     */
    fun setAndSendState(countdownContinues: Boolean = false,
                        lastEnd: Boolean = false,
                        emergencyStop: Boolean = false,
                        matchplay: Boolean = false,
                        countdown: Boolean = false,
                        detail: SerialState.Detail = SerialState.Detail.OFF,
                        colour: SerialState.Colour = SerialState.Colour.RED,
                        timeEnabled: Boolean = false,
                        time: Int = 0,
                        startNumBeeps: Int = 0,
                        endNumBeeps: Int = 0) {
        serialState.countdownContinues = countdownContinues
        serialState.lastEnd = lastEnd
        serialState.emergencyStop = emergencyStop
        serialState.matchplay = matchplay
        serialState.countdown = countdown
        serialState.detail = detail
        serialState.colour = colour
        serialState.timeEnabled = timeEnabled
        serialState.time = time
        serialState.startNumBeeps = startNumBeeps
        serialState.endNumBeeps = endNumBeeps
        serialComms.sendState()
    }

    companion object {
        val GSON: Gson = GsonBuilder().setPrettyPrinting().create()
        const val INTENT_ACTION_GRANT_USB = BuildConfig.APPLICATION_ID + ".GRANT_USB"

        // Default values according to World Archery regulations
        const val DEFAULT_TARGET_MAX_TIME = 240
        const val DEFAULT_MATCHPLAY_MAX_TIME = 20
        const val DEFAULT_TARGET_WARN_TIME = 30
        const val DEFAULT_MATCHPLAY_WARN_TIME = 30
        const val DEFAULT_AUTO_TOGGLE_DETAIL = true
        const val DEFAULT_MATCHPLAY_NUM_ENDS = 3
        const val DEFAULT_PER_ARROW_TIME = 40
    }
}

private enum class UsbPermission { Unknown, Requested, Granted, Denied }
