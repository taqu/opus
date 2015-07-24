#include <jni.h>
#include <stdlib.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <lcore/lcore.h>
#include <lsound/Context.h>
#include <lsound/UserPlayer.h>


#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL Java_com_example_lsound_sound_Audio_create(JNIEnv* env, jclass clazz)
{
    lcore::Log("create");
    lsound::Context::InitParam initParam;
    bool ret = lsound::Context::initialize(initParam);
}

JNIEXPORT int JNICALL Java_com_example_lsound_sound_Audio_proc(JNIEnv* env, jclass clazz)
{
    return 0;
}

JNIEXPORT void JNICALL Java_com_example_lsound_sound_Audio_pause(JNIEnv* env, jclass clazz, jboolean pause)
{
    lcore::Log("pause");
    if(lsound::Context::exists()){
        if(JNI_TRUE == pause){
            lsound::Context::getInstance().setPause(true);
        }else{
            lsound::Context::getInstance().setPause(false);
        }
    }
}

JNIEXPORT void JNICALL Java_com_example_lsound_sound_Audio_terminate(JNIEnv* env, jclass clazz)
{
    lcore::Log("terminate:start");
    lsound::Context::terminate();
    lcore::Log("terminate:end");
}

//JNIEXPORT jboolean JNICALL Java_com_example_lsound_sound_Audio_openAsset(JNIEnv* env, jclass clazz, jobject assetManager, jstring filename, jboolean stream)
//{
//    lcore::Log("openAsset");
//    const char* utf8Filename = env->GetStringUTFChars(filename, NULL);
//    if(NULL == utf8Filename){
//        return JNI_FALSE;
//    }
//    AAssetManager* manager = AAssetManager_fromJava(env, assetManager);
//    if(NULL == manager){
//        env->ReleaseStringUTFChars(filename, utf8Filename);
//        return JNI_FALSE;
//    }
//    AAsset* asset = AAssetManager_open(manager, utf8Filename, AASSET_MODE_RANDOM);
//    if(NULL != asset){
//        lcore::Log("open:%s", utf8Filename);
//        lcore::Log(" length:%d", AAsset_getLength(asset));
//        AAsset_close(asset);
//    }
//    env->ReleaseStringUTFChars(filename, utf8Filename);
//    return JNI_TRUE;
//}

JNIEXPORT jboolean JNICALL Java_com_example_lsound_sound_Audio_loadResourcePack(JNIEnv* env, jclass clazz, jint packId, jstring filename, jboolean stream)
{
    lcore::Log("loadResourcePack");
    if(!lsound::Context::exists()){
        return JNI_FALSE;
    }

    const char* utf8Filename = env->GetStringUTFChars(filename, NULL);
    if(NULL == utf8Filename){
        return JNI_FALSE;
    }
    lcore::s32 retID = lsound::Context::getInstance().loadResourcePack(packId, utf8Filename, stream);
    env->ReleaseStringUTFChars(filename, utf8Filename);
    return (0<=retID);
}

JNIEXPORT jboolean JNICALL Java_com_example_lsound_sound_Audio_loadResourcePackFromAsset(JNIEnv* env, jclass clazz, jobject assetManager, jint packId, jstring filename, jboolean stream)
{
    lcore::Log("loadResourcePackFromAsset");
    if(!lsound::Context::exists()){
        return JNI_FALSE;
    }

    const char* utf8Filename = env->GetStringUTFChars(filename, NULL);
    if(NULL == utf8Filename){
        return JNI_FALSE;
    }
    AAssetManager* manager = AAssetManager_fromJava(env, assetManager);
    if(NULL == manager){
        env->ReleaseStringUTFChars(filename, utf8Filename);
        return JNI_FALSE;
    }
    //AAsset* asset = AAssetManager_open(manager, utf8Filename, AASSET_MODE_RANDOM);
    //if(NULL != asset){
    //    lcore::Log("open:%s", utf8Filename);
    //    lcore::Log(" length:%d", AAsset_getLength(asset));
    //    AAsset_close(asset);
    //}
    lcore::s32 retID = lsound::Context::getInstance().loadResourcePackFromAsset(manager, packId, utf8Filename, stream);
    env->ReleaseStringUTFChars(filename, utf8Filename);
    return (0<=retID);
}

JNIEXPORT void JNICALL Java_com_example_lsound_sound_Audio_play(JNIEnv* env, jclass clazz, jint packId, jint id, jfloat volume)
{
    //lcore::Log("play");
    if(!lsound::Context::exists()){
        return;
    }
    lsound::Context::getInstance().play(packId, id, volume);
}

JNIEXPORT void JNICALL Java_com_example_lsound_sound_Audio_createUserPlayer(JNIEnv* env, jclass clazz, jint packId, jint id)
{
    lcore::Log("createUserPlayer");
    if(!lsound::Context::exists()){
        return;
    }
    lsound::UserPlayer* player = lsound::Context::getInstance().createUserPlayer(packId, id);
    player->setFlag(lsound::PlayerFlag_Loop);
    player->play();
}

#ifdef __cplusplus
}
#endif

