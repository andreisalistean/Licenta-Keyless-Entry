package com.example.mycar;
import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.pm.PackageManager;
import android.util.Log;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import java.util.Arrays;
import java.util.List;
import java.util.Random;
import java.util.UUID;


public class BluetoothController {

    private static BluetoothController instance;
    private BluetoothGatt btGatt;
    private BluetoothGattCharacteristic characteristicSend;
    private BluetoothGattCharacteristic characteristicRecive;
    private AES aes;
    private byte[] msg;
    private Random random;
    private byte[] TOKEN = {(byte)0x76,(byte)0x09,(byte)0x0e,(byte)0x23};
    public boolean isConnected = false;
    public boolean readyToSend = false;
    byte counterSend=0;


    private final BluetoothGattCallback bleutoothGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            Log.d("data", String.valueOf(status));
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    Log.e("data", "sleep failed");
                }
                MainActivity.bluetoothIcon.setImageResource(R.drawable.bluetooth_connected_2);
                if (ActivityCompat.checkSelfPermission(MainActivity.mna, android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                    Log.e("data", "nem permisiuni");
                    return;
                }
                Log.d("data", "conectat");
                isConnected = true;
                btGatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                isConnected = false;
                readyToSend=false;
                aes.destroySessionKey();

                MainActivity.mna.setViewsForDisconnect();
            }
        }

        @SuppressLint("MissingPermission")
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                getServices();
            } else if (status == BluetoothGatt.GATT_FAILURE) {
                btGatt.disconnect();
                isConnected = false;
            }
        }

        @SuppressLint("MissingPermission")
        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            super.onDescriptorWrite(gatt, descriptor, status);
            readyToSend=true;
        }

        @Override
        public void onCharacteristicChanged(@NonNull BluetoothGatt gatt, @NonNull BluetoothGattCharacteristic characteristic, @NonNull byte[] value) {
            if (characteristic == characteristicRecive) {
                Log.d("data","date primite");
            }
        }


    };

    public void send(byte comand) {
        msg = new byte[16];

        msg[0]=(byte)0x21;
        msg[1]=TOKEN[0];
        msg[2]=TOKEN[1];
        msg[3]=TOKEN[2];
        msg[4]=TOKEN[3];
        msg[5]=(byte)random.nextInt(255);

        if(comand==(byte)0xA4) {//RSSI
            msg[6] = 0x11;
        }
        else {
            msg[6] = 0x78;
        }
        msg[7]=comand;
        Log.e("da",String.valueOf(msg[6]));
        byte[] crypt = aes.encrypt(msg, true);
        if (ActivityCompat.checkSelfPermission(MainActivity.mna, android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            Log.e("err", "drepturi send");
            return;
        }
        btGatt.writeCharacteristic(characteristicSend, crypt, BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
        msg = null;
    }

    public static BluetoothController getInstance(BluetoothManager manager) {
        if (BluetoothController.instance == null) {
            BluetoothController.instance = new BluetoothController();
            BluetoothController.instance.start(manager);
        }
        return instance;
    }

    private void start(BluetoothManager manager) {
        aes = new AES();
        random = new Random();
        //byte[] arr = new byte[16];
        //for(short i =0;i<16;i++) arr[i]=(byte)0xFF;
        //Log.d("data",String.valueOf(checkSum(arr)));
        BluetoothDevice device = manager.getAdapter().getRemoteDevice("B0:A7:32:2F:7C:3A");
        if (device == null) {
            Log.e("data", "device==null");
            return;
        }
        if (ActivityCompat.checkSelfPermission(MainActivity.mna, android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            Log.e("data", "n.ai permisiuni bree");
            return;
        }
        btGatt = device.connectGatt(MainActivity.mna, true, instance.bleutoothGattCallback);
        if (btGatt == null) {
            Log.e("data", "btGatt == null");
        }

    }

    @SuppressLint("MissingPermission")
    private void getServices() {
        List<BluetoothGattService> list = btGatt.getServices();
        if (list == null) {
            Log.e("data", "list == null");
            return;
        }
        BluetoothGattService service = null;
        for (BluetoothGattService serv : list) {
            if (serv.getUuid().toString().equals("5daea2b4-1912-4839-8c9d-7657f074fa84")) {
                service = serv;
                break;
            }
        }
        if (service == null) {
            Log.e("data", "service == null");
            disconnect();
            return;
        }
        this.characteristicSend = service.getCharacteristic(UUID.fromString("f61ad6ec-247b-44e9-98e1-f2a61b9836ea"));
        this.characteristicRecive = service.getCharacteristic(UUID.fromString("df847cb0-69aa-43e1-8aa9-1fad496c60b5"));
        btGatt.setCharacteristicNotification(characteristicRecive, true);
        BluetoothGattDescriptor desc = characteristicRecive.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"));
        btGatt.writeDescriptor(desc, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        //if(!aes.createSessionKey()) disconnect();
        try{Thread.sleep(1000);}catch (Exception ex){Log.d("data","failed theread.sleep()");}

    }

    private void disconnect() {
        if (ActivityCompat.checkSelfPermission(MainActivity.mna, android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED)
            btGatt.disconnect();
    }

//    private void processHandshake(byte[] data, byte step) {
//
//            if (step == (byte) 0xA4) {
//                Log.e("data","a4");
//                TOKENS[0] = data[2];
//                TOKENS[1] = data[3];
//                TOKENS[2] = data[4];
//                TOKENS[3] = data[5];
//
//                byte[] dataForTransmission = new byte[16];
//                dataForTransmission[0] = 0x21;
//                dataForTransmission[1] = (byte) 0xB3;
//                for (short i = 0; i < 4; i++) {
//                    TOKENC[i] = (byte) random.nextInt(255);
//                    dataForTransmission[i + 2] = TOKENC[i];
//                }
//                dataForTransmission[6] = TOKENS[0];
//                dataForTransmission[7] = TOKENS[1];
//                dataForTransmission[8] = TOKENS[2];
//                dataForTransmission[9] = TOKENS[3];
//                dataForTransmission[10] = 0x00;
//                dataForTransmission[11] = (byte) random.nextInt(255);
//                dataForTransmission[12] = 0x00;
//                dataForTransmission[13] = 0x00;
//                dataForTransmission[14] = 0x00;
//
//                dataForTransmission[15] = checkSum(dataForTransmission);
//                Log.d("data",dataForTransmission[15]==checkSum(dataForTransmission)?"egal":"diferit");
//
//                byte[] newData = aes.encrypt(dataForTransmission, true);
//                if (ActivityCompat.checkSelfPermission(MainActivity.mna, android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
//                    Log.e("data","nu ai permisiuni bt handshake");
//                    return;
//                }
//                int cod=btGatt.writeCharacteristic(characteristicSend, newData, BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
//                Log.d("data",String.valueOf(cod));
//            } else if (step == (byte)0xC2) {
//                Log.e("data","c2");
//                if(data[15]==checkSum(data))
//                {
//                    for(short i=0;i<4;i++)
//                    {
//                        if(data[i+2]!=TOKENC[i]) {
//                            disconnect();
//                            Log.d("data","invalid token c / 3");
//                        }
//                    }
//                    readyToSend=true;
//                }
//                else {disconnect();Log.d("data","invalid checksum / 3");return;}
//            }
//
//    }

//    private byte checkSum(byte[] datt)
//    {
//        byte bits=0;
//        byte val;
//        for(short i=0;i<15;i++)
//        {
//            val=datt[i];
//            for (int j = 0; j < 8; j++) {
//                if ((val & (1 << j)) != 0) {
//                    bits++;
//                }
//            }
//        }
//        return bits;
//    }


}
