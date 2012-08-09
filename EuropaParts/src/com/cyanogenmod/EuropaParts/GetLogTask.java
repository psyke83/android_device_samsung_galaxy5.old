package com.cyanogenmod.settings.device;

import java.text.SimpleDateFormat;
import java.util.Calendar;

import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Environment;
import android.widget.Toast;
import com.cyanogenmod.settings.device.R;

public class GetLogTask extends AsyncTask<Void, Void, String> {

	Context mContext = DeviceSettings.mContext;
	ProgressDialog mProgressDialog;

	@Override
	protected void onPreExecute() {

		mProgressDialog = new ProgressDialog(mContext);
		mProgressDialog.setMessage(mContext.getText(R.string.processing));
		mProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
		mProgressDialog.setCancelable(false);
		mProgressDialog.show();
	}

	@Override
	protected String doInBackground(Void... arg0) {

		if (!Environment.MEDIA_MOUNTED.equals(Environment
				.getExternalStorageState())) {
			String sderror = mContext.getText(R.string.sd_not_mounted)
					.toString();
			return sderror;
		}

		String date = new SimpleDateFormat("yyyyMMdd-HH.mm.ss").format(Calendar
				.getInstance().getTime());

		String bugfile = "bugreport_" + date + ".log";

		String command = "dumpstate > /sdcard/" + bugfile + ";"
				+ "bzip2 -9 /sdcard/" + bugfile;

		Command.su(command);
		String success = mContext.getText(R.string.bug_report_success)
				+ " /sdcard/" + bugfile + ".bz2";
		return success;
	}

	@Override
	protected void onPostExecute(String result) {

		mProgressDialog.cancel();

		Toast.makeText(mContext, result, 1).show();

	}

}
