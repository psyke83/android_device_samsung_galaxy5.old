package com.cyanogenmod.settings.device;

import android.content.Context;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import com.cyanogenmod.settings.device.R;

public class DeviceSettings extends PreferenceActivity {

	public static Context mContext;

	public static String PROP_FAKE_DT = "persist.sys.fakedt";
	public static String PROP_REODEX = "persist.sys.reodex";
	public static String PROP_SAMSUNG_EXTAMP_FILTER = "persist.sys.extamp-filter";
	public static String PROP_SWAP = "persist.sys.swap";
	public static String PROP_SPEAKER_ATTN = "persist.sys.speaker-attn";
	public static String PROP_HEADSET_ATTN = "persist.sys.headset-attn";
	public static String PROP_FM_ATTN = "persist.sys.fm-attn";
	public static String PROP_COMPCACHE = "persist.service.compcache";
	public static String PROP_KSM = "persist.sys.ksm";

	public static CharSequence[] attn = { "Disabled", "1 dB",
			"2 dB", "3 dB", "4 dB", "5 dB", "6 dB" };
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mContext = this;

		PreferenceScreen PrefScreen = getPreferenceManager()
				.createPreferenceScreen(this);

		PreferenceCategory ROMCat = new PreferenceCategory(this);
		ROMCat.setTitle(getText(R.string.rom_cat));
		PrefScreen.addPreference(ROMCat);

		// FAKE DUALTOUCH
		final CheckBoxPreference DT = new CheckBoxPreference(this);
		DT.setTitle(getText(R.string.fake_dt));
		DT.setSummary(getText(R.string.fake_dt_sum));
		String isDTON = Command.getprop(PROP_FAKE_DT);
		if (isDTON.equals("1")) {
			DT.setChecked(true);
		} else {
			DT.setChecked(false);
		}
		DT.setEnabled(true);
		DT.setOnPreferenceClickListener(new OnPreferenceClickListener() {

			public boolean onPreferenceClick(Preference arg0) {
				if (DT.isChecked()) {
					SystemProperties.set(PROP_FAKE_DT, "1");
				} else {
					SystemProperties.set(PROP_FAKE_DT, "0");

				}

				return false;
			}
		});
		ROMCat.addPreference(DT);

		PreferenceCategory AudioCat = new PreferenceCategory(this);
		AudioCat.setTitle(getText(R.string.audio_cat));
		PrefScreen.addPreference(AudioCat);

		// EXTAMP FILTER
		final CheckBoxPreference Extamp = new CheckBoxPreference(this);
		Extamp.setTitle(getText(R.string.extamp));
		Extamp.setSummaryOn(getText(R.string.extamp_sum_on));
		Extamp.setSummaryOff(getText(R.string.extamp_sum_off));
		String isExtampON = Command.getprop(PROP_SAMSUNG_EXTAMP_FILTER);
		if (isExtampON.equals("1")) {
			Extamp.setChecked(true);
		} else {
			Extamp.setChecked(false);
		}
		Extamp.setEnabled(true);
		Extamp.setOnPreferenceClickListener(new OnPreferenceClickListener() {

			public boolean onPreferenceClick(Preference arg0) {
				if (Extamp.isChecked()) {
					SystemProperties.set(PROP_SAMSUNG_EXTAMP_FILTER, "1");
				} else {
					SystemProperties.set(PROP_SAMSUNG_EXTAMP_FILTER, "0");

				}

				return false;
			}
		});
		AudioCat.addPreference(Extamp);

		// ATTENUATION SPEAKER
		Preference AttnS = new Preference(this);
		AttnS.setTitle(getText(R.string.attn_speaker));
		String currentAttnS = Command.getprop(PROP_SPEAKER_ATTN);
		int currentAttnSi = 0;
		if (currentAttnS.length() == 1) {
			currentAttnSi = Integer.parseInt(currentAttnS);
		}
		AttnS.setSummary(getText(R.string.current_setting) + " " + attn[currentAttnSi]);
		AttnS.setEnabled(true);
		AttnS.setOnPreferenceClickListener(new AttnSpeaker(this));
		AudioCat.addPreference(AttnS);

		// ATTENUATION HEADSET
		Preference AttnH = new Preference(this);
		AttnH.setTitle(getText(R.string.attn_headset));
		String currentAttnH = Command.getprop(PROP_HEADSET_ATTN);
		if (currentAttnH.equals("0")) {
			currentAttnH = getText(R.string.disabled).toString();
		}
		int currentAttnHi = 0;
		if (currentAttnH.length() == 1) {
			currentAttnHi = Integer.parseInt(currentAttnH);
		}
		AttnH.setSummary(getText(R.string.current_setting) + " " + attn[currentAttnHi]);
		AttnH.setEnabled(true);
		AttnH.setOnPreferenceClickListener(new AttnHeadset(this));
		AudioCat.addPreference(AttnH);

		// ATTENUATION FM
		Preference AttnFM = new Preference(this);
		AttnFM.setTitle(getText(R.string.attn_fm));
		String currentAttnFM = Command.getprop(PROP_FM_ATTN);
		
		int currentAttnFMi = 0;
		if (currentAttnFM.length() == 1) {
			currentAttnFMi = Integer.parseInt(currentAttnFM);
		}
		AttnFM.setSummary(getText(R.string.current_setting) + " " + attn[currentAttnFMi]);

		AttnFM.setEnabled(true);
		AttnFM.setOnPreferenceClickListener(new AttnFM(this));
		AudioCat.addPreference(AttnFM);

		PreferenceCategory Memory = new PreferenceCategory(this);
		Memory.setTitle(getText(R.string.memory_man));
		PrefScreen.addPreference(Memory);

		// KSM
		final CheckBoxPreference KSM = new CheckBoxPreference(this);
		KSM.setTitle("Kernel samepage merging");
		KSM.setSummary(getText(R.string.ksm_sum));
		String isKsmON = Command.getprop(PROP_KSM);
		if (isKsmON.equals("1")) {
			KSM.setChecked(true);
		} else {
			KSM.setChecked(false);
		}
		KSM.setEnabled(true);
		KSM.setOnPreferenceClickListener(new OnPreferenceClickListener() {

			public boolean onPreferenceClick(Preference arg0) {
				if (KSM.isChecked()) {
					SystemProperties.set(PROP_KSM, "1");
				} else {
					SystemProperties.set(PROP_KSM, "0");

				}

				return false;
			}
		});
		Memory.addPreference(KSM);

		// Swap
		final CheckBoxPreference Swap = new CheckBoxPreference(this);
		Swap.setTitle("Swap");
		Swap.setSummary(getText(R.string.swap_sum));
		String isSwapON = Command.getprop(PROP_SWAP);
		if (isSwapON.equals("1")) {
			Swap.setChecked(true);
		} else {
			Swap.setChecked(false);
		}

		String isCompcacheON = Command.getprop(PROP_COMPCACHE);
		if (isCompcacheON.equals("0")) {
			Swap.setEnabled(true);
		} else {
			Swap.setEnabled(false);
			Swap.setSummary(getText(R.string.swap_cce));

		}

		Swap.setOnPreferenceClickListener(new OnPreferenceClickListener() {

			public boolean onPreferenceClick(Preference arg0) {
				if (Swap.isChecked()) {
					SystemProperties.set(PROP_SWAP, "1");
				} else {
					SystemProperties.set(PROP_SWAP, "0");

				}

				return false;
			}
		});
		Memory.addPreference(Swap);


		PreferenceCategory LogCat = new PreferenceCategory(this);
		LogCat.setTitle(getText(R.string.log_cat));
		PrefScreen.addPreference(LogCat);
		
		
		Preference Dump = new Preference(this);
		Dump.setTitle(getText(R.string.gen_bug_report));
		Dump.setSummary(getText(R.string.gen_bug_report_summary));
		Dump.setEnabled(true);
		Dump.setOnPreferenceClickListener(new OnPreferenceClickListener() {

			public boolean onPreferenceClick(Preference arg0) {
				new GetLogTask().execute();
				return false;
			}
		});
		LogCat.addPreference(Dump);
		
		setPreferenceScreen(PrefScreen);

	}
}
