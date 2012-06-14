package com.cyanogenmod.EuropaParts;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

public class EuropaPartsStartup extends BroadcastReceiver
{
   private static final String PROP_FAKE_DT = "persist.sys.fakedt";
   private static final String PROP_REODEX = "persist.sys.reodex";
   private static final String PROP_SAMSUNG_EXTAMP_FILTER = "persist.sys.extamp-filter";
   private static final String PROP_SWAP = "persist.sys.swap";
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
   public void onReceive(final Context context, final Intent bootintent) {
      PreferenceManager.setDefaultValues(context, R.xml.europaparts, false);
      SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
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
