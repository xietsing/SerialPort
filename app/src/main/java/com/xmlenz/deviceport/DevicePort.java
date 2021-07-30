/*
 * Copyright 2009 Cedric Priscal
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */

package com.xmlenz.deviceport;

import android.util.Log;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class DevicePort {

	private static final String TAG = "DevicePort";

	/*
	 * Do not remove or rename the field mFd: it is used by native method close();
	 */
	private FileDescriptor mFd;
	private FileInputStream mFileInputStream;
	private FileOutputStream mFileOutputStream;

	private int mi2c=-1;

	public DevicePort() {

	}

	public void openSerial(File device, int baudrate, int flags) throws SecurityException, IOException{
		mFd = SerialPort_open(device.getAbsolutePath(), baudrate, flags);
		if (mFd == null) {
			Log.e(TAG, "native open returns null");
			throw new IOException();
		}
		mFileInputStream = new FileInputStream(mFd);
		mFileOutputStream = new FileOutputStream(mFd);
	}

	public int openI2C(String path) throws SecurityException, IOException{
		mi2c = I2c_open(path);
		if (mi2c == -1) {
			Log.e(TAG, "native open returns null");
			throw new IOException();
		}
		return mi2c;
	}

	// Getters and setters
	public InputStream getInputStream() {
		return mFileInputStream;
	}

	public OutputStream getOutputStream() {
		return mFileOutputStream;
	}



	/**
	 * 字节转十六进制
	 * @param b 需要进行转换的byte字节
	 * @return  转换后的Hex字符串
	 */
	public static String byteToHex(byte b){
		String hex = Integer.toHexString(b & 0xFF);
		if(hex.length() < 2){
			hex = "0" + hex;
		}
		return hex;
	}

	/**
	 * 字节数组转16进制
	 * @param bytes 需要转换的byte数组
	 * @return 转换后的Hex字符串
	 */
	public static String bytesToHex(byte[] bytes) {
		StringBuffer sb = new StringBuffer();
		for (int i = 0; i < bytes.length; i++) {
			String hex = Integer.toHexString(bytes[i] & 0xFF);
			if (hex.length() < 2) {
				sb.append(0);
			}
			sb.append(hex);
		}
		return sb.toString();
	}

	/**
	 * Hex字符串转byte
	 *
	 * @param inHex 待转换的Hex字符串
	 * @return 转换后的byte
	 */
	public static byte hexToByte(String inHex) {
		return (byte) Integer.parseInt(inHex, 16);
	}

	/**
	 * hex字符串转byte数组
	 *
	 * @param inHex 待转换的Hex字符串
	 * @return 转换后的byte数组结果
	 */
	public static byte[] hexToByteArray(String inHex) {
		int hexlen = inHex.length();
		byte[] result;
		if (hexlen % 2 == 1) {
			//奇数
			hexlen++;
			result = new byte[(hexlen / 2)];
			inHex = "0" + inHex;
		} else {
			//偶数
			result = new byte[(hexlen / 2)];
		}
		int j = 0;
		for (int i = 0; i < hexlen; i += 2) {
			result[j] = hexToByte(inHex.substring(i, i + 2));
			j++;
		}
		return result;
	}
	// JNI
	/**
	 * A native method that is implemented by the 'native-lib' native library,
	 * which is packaged with this application.
	 */
	public native String stringFromJNI();
	private native static FileDescriptor SerialPort_open(String path, int baudrate, int flags);
	public native void SerialPort_close();

	public native  static int I2c_open(String path);
	public native  int I2c_read(int arg1,int arg2,int[] arg3,int arg4);
	public native  static int I2c_write(int arg1,int arg2,int arg3,int[] arg4,int arg5);
	public native  static void I2c_close(int handler);


	public native  byte[]  rc522_read();
	public native  void rc522_open();


	static {
		System.loadLibrary("native-lib");
	}

}
