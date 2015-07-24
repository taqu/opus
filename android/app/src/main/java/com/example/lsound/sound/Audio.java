package com.example.lsound.sound;

import android.content.res.AssetManager;

/**
 * Created by sakai_takuro on 2015/07/13.
 */
public class Audio
{
    public static native void create();
    public static native int proc();
    public static native void pause(boolean isPause);
    public static native void terminate();

    public static native boolean loadResourcePack(int packId, String filename, boolean stream);
    public static native boolean loadResourcePackFromAsset(AssetManager assetManager, int packId, String filename, boolean stream);
    public static native void play(int packId, int id, float volume);
    public static native void createUserPlayer(int packId, int id);
    static{
        System.loadLibrary("audio");
    }
}
