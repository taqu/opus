#include <stdlib.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <lsound/Context.h>
#include <lsound/UserPlayer.h>

#ifdef __cplusplus
extern "C"
{
#endif

static JavaVM* javaVM_ = NULL;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    javaVM_ = vm;
    return JNI_VERSION_1_6; // minimum JNI version
}

void audioInitialize()
{
    lcore::Log("initializeAudio");
    lsound::Context::InitParam initParam;
    bool ret = lsound::Context::initialize(initParam);
}

void audioProc()
{
    if(lsound::Context::exists()){
        lsound::Context::getInstance().updateRequests();
    }
}

void audioPause(bool pause)
{
    lcore::Log("pause");
    if(lsound::Context::exists()){
        lsound::Context::getInstance().setPause(pause);
    }
}

void audioTerminate()
{
    lcore::Log("terminate:start");
    lsound::Context::terminate();
}

bool audioLoadResourcePack(int packId, const char* filename, bool stream)
{
    lcore::Log("loadResourcePack");
    if(!lsound::Context::exists()){
        return false;
    }


    lcore::s32 retID = lsound::Context::getInstance().loadResourcePack(packId, filename, stream);
    return (0<=retID);
}

bool audioLoadResourcePackFromAsset(void* assetManager, int packId, const char* filename, bool stream)
{
    lcore::Log("loadResourcePackFromAsset:%s", filename);
    if(!lsound::Context::exists()){
        return false;
    }
    if(NULL == javaVM_){
        return false;
    }
    JNIEnv* env=NULL;
	jint ret = javaVM_->GetEnv((void**)&env, JNI_VERSION_1_6);
	if (ret != JNI_OK) {
        lcore::Log("JavaVM:GetEnv:error");
		return false;
	}

    AAssetManager* manager = AAssetManager_fromJava(env, (jobject)assetManager);
    if(NULL == manager){
        lcore::Log("AAssetManager_fromJava:error");
        return false;
    }
    lcore::s32 retID = lsound::Context::getInstance().loadResourcePackFromAsset(manager, packId, filename, stream);
    lcore::Log("loadResourcePackFromAsset:%d", retID);
    return (0<=retID);
}

void audioPlay(int packId, int id, float volume)
{
    //lcore::Log("play");
    if(!lsound::Context::exists()){
        return;
    }
    lsound::Context::getInstance().play(packId, id, volume);
}

void* audioCreateUserPlayer(int packId, int id)
{
    lcore::Log("createUserPlayer");
    if(!lsound::Context::exists()){
        return NULL;
    }
    lsound::UserPlayer* player = lsound::Context::getInstance().createUserPlayer(packId, id);
    player->setFlag(lsound::PlayerFlag_Loop);
    //player->play();
    return player;
}

void audioDestroyUserPlayer(void* player)
{
    lcore::Log("destroyUserPlayer");
    if(!lsound::Context::exists()){
        return;
    }
    lsound::Context::getInstance().destroyUserPlayer(reinterpret_cast<lsound::UserPlayer*>(player));
}

void audioUserPlayerPlay(void* player)
{
    if(NULL == player){
        return;
    }
    reinterpret_cast<lsound::UserPlayer*>(player)->play();
}

void audioUserPlayerPause(void* player)
{
    if(NULL == player){
        return;
    }
    reinterpret_cast<lsound::UserPlayer*>(player)->pause();
}

#ifdef __cplusplus
}
#endif

