package com.cyanogenmod.settings.device;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.SystemProperties;
import android.preference.Preference;
import com.cyanogenmod.settings.device.R;

final class AttnFM implements Preference.OnPreferenceClickListener {

	Context context = DeviceSettings.mContext;
	int selected = -1;

	AttnFM(DeviceSettings arg0) {
	}

	public final boolean onPreferenceClick(final Preference preference) {

		String currentSetting = Command.getprop(DeviceSettings.PROP_FM_ATTN);
		if (currentSetting.length() == 1) {
			selected = Integer.parseInt(currentSetting);
		}

		AlertDialog.Builder dialog = new AlertDialog.Builder(context)
				.setTitle(context.getText(R.string.attn_fm))
				.setSingleChoiceItems(DeviceSettings.attn, selected, new OnClickListener() {

					public void onClick(DialogInterface arg0, int arg1) {

						SystemProperties.set(DeviceSettings.PROP_FM_ATTN, ""
								+ arg1);
						
						preference.setSummary(context
								.getText(R.string.current_setting) + " " + DeviceSettings.attn[arg1].toString());

						arg0.dismiss();

					}

				})

				.setNegativeButton(context.getText(R.string.cancel), null);
		dialog.show();
		return false;

	}
}
