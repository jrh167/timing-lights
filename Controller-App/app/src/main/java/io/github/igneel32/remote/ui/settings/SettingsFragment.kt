package io.github.igneel32.remote.ui.settings

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.EditText
import android.widget.Toast
import androidx.appcompat.widget.SwitchCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import com.google.android.material.floatingactionbutton.FloatingActionButton
import io.github.igneel32.remote.MainActivity
import io.github.igneel32.remote.R
import io.github.igneel32.remote.Storage

class SettingsFragment : Fragment() {

    private lateinit var mainActivity: MainActivity
    private lateinit var settingsViewModel: SettingsViewModel
    private lateinit var storage: Storage

    private lateinit var tgtEndSecondsField : EditText
    private lateinit var tgtWarnTimeField : EditText
    private lateinit var tgtAutoDetailToggle : SwitchCompat
    private lateinit var mtchEndSecondsField : EditText
    private lateinit var mtchWarnTimeField : EditText
    private lateinit var mtchNumEndsField : EditText
    private lateinit var equipFailSecondsPerField : EditText

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        mainActivity = (activity as MainActivity)
        settingsViewModel =
            ViewModelProvider(this).get(SettingsViewModel::class.java)
        storage = mainActivity.storage.value!!
        val root = inflater.inflate(R.layout.fragment_settings, container, false)

        val saveBtn = root.findViewById<FloatingActionButton>(R.id.save_settings);
        saveBtn.setOnClickListener {
            saveSettings()
            Toast.makeText(mainActivity, R.string.saved, Toast.LENGTH_SHORT).show()
        }

        tgtEndSecondsField = root.findViewById(R.id.total_time_field)
        tgtWarnTimeField = root.findViewById(R.id.warning_time_field)
        tgtAutoDetailToggle = root.findViewById(R.id.auto_toggle_detail)
        mtchEndSecondsField = root.findViewById(R.id.matchplay_seconds_per_end_field)
        mtchWarnTimeField = root.findViewById(R.id.matchplay_warning_time_field)
        mtchNumEndsField = root.findViewById(R.id.num_times_swap)
        equipFailSecondsPerField = root.findViewById(R.id.equip_fail_seconds_field)

        fillFields()

        return root
    }

    private fun fillFields() {
        tgtEndSecondsField.setText(storage.targetMaxTime.toString())
        tgtWarnTimeField.setText(storage.targetWarnTime.toString())
        tgtAutoDetailToggle.isChecked = storage.autoToggleDetail
        mtchEndSecondsField.setText(storage.matchplayMaxTime.toString())
        mtchWarnTimeField.setText(storage.matchplayWarnTime.toString())
        mtchNumEndsField.setText(storage.matchplayNumEnds.toString())
        equipFailSecondsPerField.setText(storage.equipFailTime.toString())
    }

    private fun saveSettings() {
        storage.targetMaxTime = tgtEndSecondsField.text.toString().toInt()
        storage.targetWarnTime = tgtWarnTimeField.text.toString().toInt()
        storage.autoToggleDetail = tgtAutoDetailToggle.isChecked
        storage.matchplayMaxTime = mtchEndSecondsField.text.toString().toInt()
        storage.matchplayWarnTime = mtchWarnTimeField.text.toString().toInt()
        storage.matchplayNumEnds = mtchNumEndsField.text.toString().toInt()
        storage.equipFailTime = equipFailSecondsPerField.text.toString().toInt()

        mainActivity.saveStorage()
    }
}