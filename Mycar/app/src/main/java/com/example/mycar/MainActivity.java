package com.example.mycar;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import java.nio.charset.StandardCharsets;
import java.security.Key;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.spec.EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.List;
import java.util.UUID;

import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import javax.crypto.Cipher;

public class MainActivity extends AppCompatActivity {

    public static MainActivity mna;
    public static ImageView bluetoothIcon=null;
    public static ImageView lightsIcon=null;
    public static ImageView lockIcon=null;
    public static ImageView engineIcon=null;
    public static TextView  gearText=null;
    public static TextView engineText=null;
    public static TextView voltageText=null;
    public static TextView tempText=null;

    public static boolean keylessEnabled=true;

    public void setData(byte a,byte b,byte c)
    {
        String data=a+"V";
        voltageText.setText(data);

        //tempText.setText(value[1]);

        if((c&0x80)>0x00)
            keylessEnabled=true;
        else
           keylessEnabled=false;

        if((c&0x40)>0x00)
            lockIcon.setImageResource(R.drawable.lock_3);
        else
            lockIcon.setImageResource(R.drawable.unlock_2);

        if((c&0x20)>0x00)
        {
            engineIcon.setImageResource(R.drawable.engine_on);
            String t="Engine On";
            engineText.setText(t);
        }
        else{
            engineIcon.setImageResource(R.drawable.engine_off_2);
            String e ="Engine off";
            engineText.setText(e);
        }
        if((c&0x10)==0x10){
            String v="In Gear";
            gearText.setText(v);
        }
        else{
            String v= "Not in Gear";
            gearText.setText(v);
        }
        if((c&0x08)==0x08)
            lightsIcon.setImageResource(R.drawable.light_on);
        else
            lightsIcon.setImageResource(R.drawable.lights);
    }
    private void setDisplayreferences()
    {
        bluetoothIcon=(ImageView) findViewById(R.id.statusConnection);
        gearText=(TextView) findViewById(R.id.valueGear);
        engineText=(TextView) findViewById(R.id.valueEngine);
        lightsIcon=(ImageView) findViewById(R.id.statusLights);
        engineIcon=(ImageView) findViewById(R.id.statusEngine);
        lockIcon=(ImageView) findViewById(R.id.statusLock);
        voltageText=(TextView) findViewById(R.id.valueVoltage);
        tempText=(TextView) findViewById(R.id.valueTemp);
    }


    public void setViewsForDisconnect() {
        bluetoothIcon.setImageResource(R.drawable.bluetooth_disconnect_2);
        gearText.setText("");
        engineText.setText("");
        lightsIcon.setImageDrawable(null);
        engineIcon.setImageDrawable(null);
        lockIcon.setImageDrawable(null);
        voltageText.setText("");
        tempText.setText("");

    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mna = this;
        setDisplayreferences();
        boolean running_Service=false;


//        Thread th = new Thread(new Runnable() {
//            @Override
//            public void run() {
//                startService(new Intent(MainActivity.mna, BluetoothService.class));
//            }
//        });
//
//        th.start();
            startService(new Intent(MainActivity.mna, BluetoothService.class));
    }


    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    public void openSettings(View view) {
        startActivity(new Intent(this, ActivitySettings.class).putExtra("keylessValue", keylessEnabled));
    }

    public void light(View view) {
        BluetoothService.comand=(byte)0xAA;
        Log.d("data","lomini trimis");
    }

    public void unlock(View view) {
        BluetoothService.comand=(byte)0x49;
    }

    public void lock(View view) {
        BluetoothService.comand=(byte)0xFC;
    }

    public void start_stop(View view) {
        BluetoothService.comand=(byte)0x1B;
    }
}