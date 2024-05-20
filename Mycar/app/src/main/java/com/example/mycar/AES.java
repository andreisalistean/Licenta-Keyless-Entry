package com.example.mycar;

import android.util.Log;
import java.util.Random;
import java.security.spec.AlgorithmParameterSpec;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

public class AES {

    private byte[] key={0x43, 0x52,0x4f,0x41,0x5a,0x49,0x45,0x52,0x41,0x20,0x50,0x45,0x20,0x4e, 0x49,0x4c};
    private byte[] iv={0x4c,0x45,0x43,0x20,0x41,0x43,0x41, 0x53,0x41, 0x4d, 0x41,0x49,0x4e,0x45,0x20,0x50};
    public  byte[] sessionKey={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    SecretKey Skey;

    SecretKey SSessionKey;
    AlgorithmParameterSpec Siv;

    Random random;


    public AES()
    {
        Skey= new SecretKeySpec(key, "AES");
        Siv=new IvParameterSpec(iv);
        random = new Random();
    }

    public boolean createSessionKey()
    {
        for(short i=0;i<16;i++) sessionKey[i] = (byte) random.nextInt(255);
        try{
            SSessionKey=new SecretKeySpec(sessionKey,"AES");
        }catch (Exception ex) {
            return false;
        }
        return true;
    }



    public byte[] encrypt(byte[] plain, boolean session)
    {
        try{
            Cipher cipher = Cipher.getInstance("AES/CBC/NoPadding");
            cipher.init(Cipher.ENCRYPT_MODE,Skey, Siv);
            return cipher.doFinal(plain);
        }catch(Exception e){
            Log.e("cript", "encrypt");}
        return null;
    }

    public byte[] decrypt(byte[] cript)
    {
        try{
            Cipher cipher = Cipher.getInstance("AES/CBC/NoPadding");
            cipher.init(Cipher.DECRYPT_MODE,SSessionKey,Siv);
            return cipher.doFinal(cript);
        }catch(Exception e){
            Log.wtf("data", "deencrypt"+e.toString());
            return null;
        }
    }

    public void destroySessionKey()
    {
        for(short i=0;i<16;i++)
            sessionKey[i]=0x00;
        SSessionKey=null;
    }

}
