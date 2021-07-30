package com.xmlenz.deviceport;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.pm.PackageManager;
import android.nfc.NfcAdapter;
import android.nfc.NfcManager;
import android.nfc.api.NFC;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.EditText;
import android.widget.Toast;



import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    private DevicePort mDevicePort;

    private Rc522 mRc522;
    private OutputStream out;
    private String lineStr="";

    private String strok="";

    private  NFC nfc;

    private EditText editText;

    private int nCheckId=0;
    private int nCanStep = 0;
    private String cmd =  "";
    private int lineNum=0;

    private int SendFlag= 0;//是否发送

    public static final int UPDATE = 0x1;

    private Handler handler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what){
                case UPDATE:
                    editText.setText(( String )msg.obj );
                    break;
            }
        }
    };

    public static boolean hasNfc(Context context) {
        boolean bRet = false;
        if (context == null)
            return bRet;
        NfcManager manager = (NfcManager) context.getSystemService(Context.NFC_SERVICE);
        NfcAdapter adapter = manager.getDefaultAdapter();
        if (adapter != null && adapter.isEnabled()) {
            // adapter存在，能启用
            bRet = true;
        }
        return bRet;
    }

        @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
//        TextView tv = findViewById(R.id.sample_text);
        editText = findViewById(R.id.editText);
        mDevicePort = new DevicePort();

        try {
            //B100 设备是ttyS0 115200
            mDevicePort.openSerial(new File("/dev/ttyS0"), 115200, 0);
//            mDevicePort.openSerial(new File("/dev/ttyS4"), 115200, 0);
            Log.d("=-==========","");
            //mDevicePort = new DevicePort(new File("/dev/ttyS5"), 115200, 0);
        } catch (IOException e) {
            System.out.println("找不到该设备文件");
            Log.d("=-==========","找不到该设备文件");
            e.printStackTrace();
        }


//        //===
//            Log.d("=-==========","IIIIIIIIIIIIII2222222222222CCCCCCCCCCCCCCCC");
//
//            int hi2c = mDevicePort.I2c_open("/dev/i2c-5");
//        if(hi2c>=0){
//
//            int[] rf=new int[2];
//            for(int i =0 ;i<255;i++){
//                int r = mDevicePort.I2c_read(hi2c,i,rf,2);
//                if(r>=0){
//                    Log.d("=-==========","IIIIIIIIIIIIII2222222222222CCCCCCCCCCCCCCCC");
//                }
//            }
//
//        }

            ///===


        Log.d("=-==========","rc522");
        mDevicePort.rc522_open();



        final InputStream inputStream = mDevicePort.getInputStream();
        out = mDevicePort.getOutputStream();

        cmd = "UART_XIEYI_HM";
        SendFlag = 1; //发送

//        new Thread(new Runnable() {
//            @Override
//            public void run() {
//                while (true) {
//                    try {
//                        Thread.sleep(2000);
//                        if(SendFlag != 1)continue;
//                        if(strok.length()==5) {
//                            if (strok.substring(0, 2).equals("OK")) {
//                                SendFlag = 0;
//                                strok = "";
//                                Log.d("=-==========", "OK");
//                                continue;
//                            }
//                        }
//                        if (out == null) {
//                            checkline("error", 0);
//                            continue;
//                        }
//
//                        byte[] mBuffer = cmd.getBytes();
//
//                        out.write(mBuffer);
//                        checkline(cmd , 0);
//                        out.flush();
//
//
//
//                    } catch (InterruptedException e) {
//                        e.printStackTrace();
//                    } catch (IOException e) {
//                        e.printStackTrace();
//                    }
//                }
//            }
//        }).start();




        //I2c 接口
        new Thread(new Runnable() {
            @Override
            public void run() {
                while(true){
                    try {
                        Thread.sleep(500);
                        byte[] bytes=mDevicePort.rc522_read();
                        if(bytes==null)continue;
                        if(bytes.length<=0) continue;
                        String uid =mDevicePort.bytesToHex(bytes);
                        if(uid.equals("00"))continue;
                        Message msg = new Message();
                        msg.what = UPDATE;
                        lineStr = lineStr+">>="+uid+"\r\n";
                        msg.obj = lineStr.toString();
                        handler.sendMessage(msg);
                        Log.d("==2",uid);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }).start();

        /* 开启一个线程进行读取 */
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    try {
                        Thread.sleep(100);
                        //checkline("11", 1);
                        int si= inputStream.available();
                        if(si<=0) continue;
                        byte[] buffer = new byte[si];

                        int size = inputStream.read(buffer);
                        byte[] readBytes = new byte[size];
                        System.arraycopy(buffer, 0, readBytes, 0, size);
                        if(size==5) {
                            strok = new String(readBytes);
                        }
                        String receive = new String(readBytes);//mDevicePort.bytesToHex(readBytes);//String.valueOf(buffer);
//                        String receive =mDevicePort.bytesToHex(readBytes);
                        checkline(receive, 1);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }).start();

//        tv.setText(mDevicePort.stringFromJNI());
    }

    private void checkline(String sline,int direction){
        if(lineNum >=40){
            lineNum=0;
            lineStr="";
        }
        lineNum = lineNum +1;
        if(direction == 0)
            lineStr = lineStr+">>="+sline+"\r\n";
        else
            lineStr = lineStr+"<<="+sline+"\r\n";

//        Log.d("==",lineStr);
        Message msg = new Message();
        msg.what = UPDATE;
        msg.obj = lineStr.toString();
        handler.sendMessage(msg);
    }
}
