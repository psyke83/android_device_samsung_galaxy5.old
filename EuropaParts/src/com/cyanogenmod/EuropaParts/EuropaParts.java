package com.cyanogenmod.EuropaParts;

import com.cyanogenmod.EuropaParts.R;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

public class EuropaParts extends PreferenceActivity {
	private static final String PROP_FAKE_DT = "persist.sys.fakedt";
	private static final String PROP_REODEX = "persist.sys.reodex";
	private static final String PROP_SAMSUNG_EXTAMP_FILTER = "persist.sys.extamp-filter";
	private static final String PROP_SWAP = "persist.sys.swap";

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		PreferenceManager.setDefaultValues(getBaseContext(), R.xml.europaparts, false);
		addPreferencesFromResource(R.xml.europaparts);
	}

   private void writeValue(String parameter, int value) {
      try {
          FileOutputStream fos = new FileOutputStream(new File(parameter));
          fos.write(String.valueOf(value).getBytes());
          fos.flush();
          fos.getFD().sync();
          fos.close();
      } catch (FileNotFoundException e) {
         e.printStackTrace();
      } catch (IOException e) {
         e.printStackTrace();
      }
   }

   @Override
   public void onPause() {
      super.onPause();
      SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
      // Automatic Re-Odex
      if(prefs.getBoolean("reodex", false))
         SystemProperties.set(PROP_REODEX, (String) "1");
      else
         SystemProperties.set(PROP_REODEX, (String) "0");

      // Fake Dual-Touch
      if(prefs.getBoolean("fake_dt", false))
         SystemProperties.set(PROP_FAKE_DT, (String) "1");
      else
         SystemProperties.set(PROP_FAKE_DT, (String) "0");

      // KSM
      if(prefs.getBoolean("ksm", false))
         writeValue("/sys/kernel/mm/ksm/run", 1);
      else
         writeValue("/sys/kernel/mm/ksm/run", 0);

      // Samsung extamp filter
      if(prefs.getBoolean("extamp", false))
         SystemProperties.set(PROP_SAMSUNG_EXTAMP_FILTER, (String) "1");
      else
         SystemProperties.set(PROP_SAMSUNG_EXTAMP_FILTER, (String) "0");

      // Swap
      if(prefs.getBoolean("swap", false))
         SystemProperties.set(PROP_SWAP, (String) "1");
      else
         SystemProperties.set(PROP_SWAP, (String) "0");
   }
}
