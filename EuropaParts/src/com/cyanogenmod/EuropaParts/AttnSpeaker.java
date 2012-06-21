package com.cyanogenmod.EuropaParts;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.SystemProperties;
import android.preference.Preference;
import com.cyanogenmod.EuropaParts.R;

final class AttnSpeaker implements Preference.OnPreferenceClickListener {

	Context context = MainActivity.mContext;
	int selected = -1;

	AttnSpeaker(MainActivity arg0) {
	}

	public final boolean onPreferenceClick(final Preference preference) {

		String currentSetting = Command.getprop(MainActivity.PROP_SPEAKER_ATTN);
		if (currentSetting.length() == 1) {
			selected = Integer.parseInt(currentSetting);
		}

		AlertDialog.Builder dialog = new AlertDialog.Builder(context)
				.setTitle(context.getText(R.string.attn_speaker))
				.setSingleChoiceItems(MainActivity.attn, selected, new OnClickListener() {

					public void onClick(DialogInterface arg0, int arg1) {

						SystemProperties.set(MainActivity.PROP_SPEAKER_ATTN, ""
								+ arg1);
						
						preference.setSummary(context
								.getText(R.string.current_setting) + " " + MainActivity.attn[arg1].toString());

						arg0.dismiss();

					}

				})

				.setNegativeButton(context.getText(R.string.cancel), null);
		dialog.show();
		return false;

	}
}
