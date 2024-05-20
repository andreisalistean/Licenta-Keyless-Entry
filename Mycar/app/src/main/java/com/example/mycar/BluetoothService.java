package com.example.mycar;

import android.app.Service;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;

import java.util.Queue;

public class BluetoothService extends Service {

    private Handler handler;
    private Runnable runnable;
    private BluetoothController controller=null;

    public static byte comand=0x00;


    public BluetoothService() {
    }
    @Override
    public void onCreate() {
        super.onCreate();


        handler = new Handler(Looper.getMainLooper());
        runnable = new Runnable() {
            @Override
            public void run() {
                //Log.d("data","merge baa "+Thread.currentThread().getName());

               if(controller.readyToSend)
               {
                   Log.d("data","controller "+String.valueOf(comand));
                   if(comand!=0x00)
                   {
                       controller.send(comand);
                       comand=0x00;
                   }
                   else {
                       controller.send((byte)0xA4);
                   }
               }

                handler.postDelayed(this, 800);
            }
        };
        handler.postDelayed(runnable, 3000);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d("data","merje ba");
        controller=BluetoothController.getInstance((BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE));
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d("data", "s-a apelat onDestroy");
    }
}
