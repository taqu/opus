using UnityEngine;
using System;
using System.Runtime.InteropServices;

public class PluginAudio
{

#if UNITY_ANDROID && !UNITY_EDITOR
    [DllImport("audio")]
    public static extern void audioInitialize();

    [DllImport("audio")]
    public static extern void audioProc();

    [DllImport("audio")]
    public static extern void audioPause(bool pause);

    [DllImport("audio")]
    public static extern void audioTerminate();

    [DllImport("audio")]
    public static extern bool audioLoadResourcePack(int packId, string filename, bool stream);

    [DllImport("audio")]
    public static extern bool audioLoadResourcePackFromAsset(IntPtr assetManager, int packId, string filename, bool stream);

    [DllImport("audio")]
    public static extern void audioPlay(int packId, int id, float volume);

    [DllImport("audio")]
    public static extern IntPtr audioCreateUserPlayer(int packId, int id);

    [DllImport("audio")]
    public static extern void audioDestroyUserPlayer(IntPtr player);

    public static bool loadResourcePackFromAsset(int packId, string filename, bool stream)
    {
        using(AndroidJavaClass unityPlayer = new AndroidJavaClass("com.unity3d.player.UnityPlayer"))
        using(AndroidJavaObject activity = unityPlayer.GetStatic<AndroidJavaObject>("currentActivity"))
        using(AndroidJavaObject assetManager = activity.Call<AndroidJavaObject>("getAssets"))
        {
            return audioLoadResourcePackFromAsset(assetManager.GetRawObject(), packId, filename, stream);
        }
    }


    [DllImport("audio")]
    public static extern void audioUserPlayerPlay(IntPtr player);

    [DllImport("audio")]
    public static extern void audioUserPlayerPause(IntPtr player);

#else

    public static void audioInitialize()
    {
    }

    public static void audioProc()
    {

    }

    public static void audioPause(bool pause)
    {

    }

    public static void audioTerminate()
    {

    }

    public static bool audioLoadResourcePack(int packId, string filename, bool stream)
    {
        return false;
    }

    public static bool audioLoadResourcePackFromAsset(IntPtr assetManager, int packId, string filename, bool stream)
    {
        return false;
    }

    public static void audioPlay(int packId, int id, float volume)
    {

    }

    public static IntPtr audioCreateUserPlayer(int packId, int id)
    {
        return IntPtr.Zero;
    }

    public static void audioDestroyUserPlayer(IntPtr player)
    {

    }

    public static bool loadResourcePackFromAsset(int packId, string filename, bool stream)
    {
        return false;
    }

    public static void audioUserPlayerPlay(IntPtr player)
    {

    }

    public static void audioUserPlayerPause(IntPtr player)
    {

    }
#endif
}
