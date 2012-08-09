package com.cyanogenmod.settings.device;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

public class Command {

	public static void su(String command) {
		Runtime runtime = Runtime.getRuntime();
		Process proc = null;
		OutputStreamWriter osw = null;
		try {
			proc = runtime.exec("su");
			osw = new OutputStreamWriter(proc.getOutputStream());
			osw.write(command);
			osw.flush();
			osw.close();
			proc.waitFor();

		} catch (InterruptedException e) {
		} catch (IOException e) {
		}
		return;
	}
	
	public static void nosu(String command) {
		Runtime runtime = Runtime.getRuntime();
		try {
			runtime.exec(command);

		} catch (IOException e) {
		}
		return;
	}

	public static String getprop(String command) {
		Runtime runtime = Runtime.getRuntime();
		Process proc = null;
		try {
			proc = runtime.exec("getprop " + command);

			BufferedReader reader = new BufferedReader(new InputStreamReader(
					proc.getInputStream()));
			int read;
			char[] buffer = new char[4096];
			StringBuffer output = new StringBuffer();

			while ((read = reader.read(buffer)) > 0) {
				output.append(buffer, 0, read);

			}
			String exit = output.toString().replace("\n", "").replace(" ", "");
			return exit;
		} catch (IOException e) {
			return "";
		}

	}

	public static String read(String command) {
		Runtime runtime = Runtime.getRuntime();
		Process proc = null;
		try {
			proc = runtime.exec(command);

			BufferedReader reader = new BufferedReader(new InputStreamReader(
					proc.getInputStream()));
			int read;
			char[] buffer = new char[4096];
			StringBuffer output = new StringBuffer();

			while ((read = reader.read(buffer)) > 0) {
				output.append(buffer, 0, read);

			}
			String exit = output.toString().replace("\n", "").replace(" ", "");
			return exit;
		} catch (IOException e) {
			return "";
		}

	}
}
