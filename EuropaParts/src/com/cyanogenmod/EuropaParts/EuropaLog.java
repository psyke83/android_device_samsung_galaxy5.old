package com.cyanogenmod.EuropaParts;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

public class EuropaLog extends Activity {
	final private static String TAG = "EuropaParts_BugReport";

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		Toast toast;
		try {
			Process p = Runtime.getRuntime().exec(new String[] {"su", "-c", "handle_europaparts bugreport"});
			BufferedReader commandResult = new BufferedReader(
					new InputStreamReader(new BufferedInputStream(
							p.getInputStream())));
			p.waitFor();
			String returned = commandResult.readLine();
			Log.d(TAG, returned);
			toast = Toast.makeText(EuropaLog.this, returned,
					Toast.LENGTH_LONG);

		} catch (Exception ex) {
			toast = Toast.makeText(EuropaLog.this,
					getString(R.string.toast_error) + " " + ex.getMessage(),
					Toast.LENGTH_LONG);
		}

		toast.show();
		finish();

	}

}
