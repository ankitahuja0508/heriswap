/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>

#include "base/Log.h"
#include "sac/base/Vector2.h"
#include "../sources/Game.h"
#include "sac/systems/RenderingSystem.h"
#include "sac/systems/SoundSystem.h"
#include "sac/systems/MusicSystem.h"
#include "sac/base/TouchInputManager.h"
#include "sac/base/EntityManager.h"

#include "sac/api/android/AssetAPIAndroidImpl.h"
#include "sac/api/android/MusicAPIAndroidImpl.h"
#include "sac/api/android/SoundAPIAndroidImpl.h"
#include "sac/api/android/LocalizeAPIAndroidImpl.h"
#include "sac/api/android/NameInputAPIAndroidImpl.h"
#include "sac/api/android/AdAPIAndroidImpl.h"

#include "api/android/StorageAPIAndroidImpl.h"

#include <png.h>
#include <algorithm>

#include <sys/time.h>
#define DT 1.0/30.

#ifndef _Included_net_damsy_soupeaucaillou_tilematch_TilematchJNILib
#define _Included_net_damsy_soupeaucaillou_tilematch_TilematchJNILib
#ifdef __cplusplus
extern "C" {
#endif

struct GameHolder;

class AndroidSuccessAPI : public SuccessAPI {
	public:
		GameHolder* holder;
		void successCompleted(const char* description, unsigned long successId);
        void openfeintLB(int mode, int diff);
        void openfeintSuccess();
};

struct GameHolder {
	Game* game;
	int width, height;
	NameInputAPIAndroidImpl* nameInput;
    StorageAPIAndroidImpl* storage;
	AndroidSuccessAPI success;
	LocalizeAPIAndroidImpl* localize;
    AdAPIAndroidImpl* ad;
    AssetAPIAndroidImpl* asset;

	struct __input {
		 int touching;
		 float x, y;
	} input;
	bool firstCall;
	struct timeval startup_time;
	float dtAccumuled, time;

	JNIEnv *gameThreadEnv, *renderThreadEnv;
	jobject assetManager;
	int openGLESVersion;


    ~GameHolder() {
        delete nameInput;
        delete storage;
        delete localize;
        delete ad;
        renderThreadEnv->DeleteGlobalRef(assetManager);
        gameThreadEnv = renderThreadEnv = 0;
    }
};

struct AndroidNativeTouchState : public NativeTouchState{
	GameHolder* holder;
	AndroidNativeTouchState(GameHolder* h) : holder(h) {}

	bool isTouching (Vector2* windowCoords) const {
		windowCoords->X = holder->input.x;
		windowCoords->Y = holder->input.y;

		return holder->input.touching;
	}
};

static char* loadTextfile(const char* assetName);
static char* loadPng(const char* assetName, int* width, int* height);

#define UPDATE_ENV_PTR(ptr, env) if (ptr != env) ptr = env

/*
 * Class:     net_damsy_soupeaucaillou_tilematch_TilematchJNILib
 * Method:    createGame
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_createGame
  (JNIEnv *env, jclass, jobject asset, jint openglesVersion) {
  	LOGW("%s -->", __FUNCTION__);
  	TimeUtil::init();
	GameHolder* hld = new GameHolder();
	hld->assetManager = (jobject)env->NewGlobalRef(asset);
	hld->localize = new LocalizeAPIAndroidImpl(env);
    hld->nameInput = new NameInputAPIAndroidImpl();
    hld->ad = new AdAPIAndroidImpl();
    hld->storage = new StorageAPIAndroidImpl(env);
    hld->asset = new AssetAPIAndroidImpl(env, hld->assetManager);
	hld->game = new Game(hld->asset, hld->storage, hld->nameInput, &hld->success, hld->localize, hld->ad);
	hld->renderThreadEnv = env;
	hld->openGLESVersion = openglesVersion;
	theRenderingSystem.assetAPI = hld->asset;
	theRenderingSystem.opengles2 = (hld->openGLESVersion == 2);
	theTouchInputManager.setNativeTouchStatePtr(new AndroidNativeTouchState(hld));
	hld->success.holder = hld;
	return (jlong)hld;
}

JNIEXPORT jlong JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_destroyGame
  (JNIEnv *env, jclass, jlong g) {
    GameHolder* hld = (GameHolder*) g;
    theMusicSystem.uninit();
    delete hld->game;
    hld->renderThreadEnv = env;
    delete hld;
}

/*
 * Class:     net_damsy_soupeaucaillou_tilematch_TilematchJNILib
 * Method:    init
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_initFromRenderThread
  (JNIEnv *env, jclass, jlong g, jint w, jint h) {
  LOGW("%s -->", __FUNCTION__);
	GameHolder* hld = (GameHolder*) g;
	UPDATE_ENV_PTR(hld->renderThreadEnv, env);
	hld->width = w;
	hld->height = h;

	hld->asset->init();
	hld->game->sacInit(hld->width, hld->height);
	LOGW("%s <--", __FUNCTION__);
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_initFromGameThread
  (JNIEnv *env, jclass, jlong g, jbyteArray jstate) {
  	GameHolder* hld = (GameHolder*) g;
	UPDATE_ENV_PTR(hld->gameThreadEnv, env);

	theMusicSystem.musicAPI = new MusicAPIAndroidImpl(env);
	theMusicSystem.assetAPI = new AssetAPIAndroidImpl(env, hld->assetManager);
	theSoundSystem.soundAPI = new SoundAPIAndroidImpl(env, hld->assetManager);
    hld->ad->init(env);
    hld->nameInput->init(env);
	hld->localize->env = env;
	hld->localize->init();
	theMusicSystem.init();
	theSoundSystem.init();
    hld->storage->env = env;
    hld->storage->init();
	theMusicSystem.assetAPI->init();

	uint8_t* state = 0;
	int size = 0;
	if (jstate) {
		size = env->GetArrayLength(jstate);
		state = (uint8_t*)env->GetByteArrayElements(jstate, NULL);
		LOGW("Restoring saved state (size:%d)", size);
	} else {
		LOGW("No saved state: creating a new Game instance from scratch");
	}

	hld->game->init(state, size);

	hld->firstCall = true;
	hld->dtAccumuled = 0;
}

/*
 * Class:     net_damsy_soupeaucaillou_tilematch_TilematchJNILib
 * Method:    step
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_step
  (JNIEnv *env, jclass, jlong g) {
  	GameHolder* hld = (GameHolder*) g;

	UPDATE_ENV_PTR(hld->gameThreadEnv, env);
	if (!hld->game)
  		return;

  	if (hld->firstCall) {
		hld->time = TimeUtil::getTime();
		hld->firstCall = false;
	}

	float dt;
	do {
		dt = TimeUtil::getTime() - hld->time;
		if (dt < DT) {
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = (DT - dt) * 1000000000LL;
			nanosleep(&ts, 0);
		}
	} while (dt < DT);

	hld->dtAccumuled += dt;
	hld->time = TimeUtil::getTime();

	float accum = hld->dtAccumuled;
	if (hld->dtAccumuled > 5 * DT) {
		LOGW("BIG DT: %.3f s", hld->dtAccumuled);
		accum = DT;
	}

	while (hld->dtAccumuled >= DT){
		hld->game->tick(accum);
		hld->dtAccumuled -= accum;
	}
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_resetTimestep
  (JNIEnv *env, jclass, jlong g) {
  	GameHolder* hld = (GameHolder*) g;

	if (!hld)
  		return;
  	hld->firstCall = true;
}

static int frameCount = 0;
static float tttttt = 0;

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_render
  (JNIEnv *env, jclass, jlong g) {
  	GameHolder* hld = (GameHolder*) g;
  	UPDATE_ENV_PTR(hld->renderThreadEnv, env);
	theRenderingSystem.render();

	frameCount++;
	if (frameCount >= 200) {
		LOGW("fps render: %.2f", 200.0 / (TimeUtil::getTime() - tttttt));
		tttttt = TimeUtil::getTime();
		frameCount = 0;
	}
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_pause
  (JNIEnv *env, jclass, jlong g) {
  	GameHolder* hld = (GameHolder*) g;
  	LOGW("%s -->", __FUNCTION__);
  	if (!hld->game)
  		return;

    // kill all music
    theMusicSystem.toggleMute(true);
	hld->game->togglePause(true);
	LOGW("%s <--", __FUNCTION__);
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_back
  (JNIEnv *env, jclass, jlong g) {
     GameHolder* hld = (GameHolder*) g;
     LOGW("%s -->", __FUNCTION__);
     if (!hld->game)
         return;

    hld->game->backPressed();
    LOGW("%s <--", __FUNCTION__);
}


JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_invalidateTextures
  (JNIEnv *env, jclass, jlong g) {
     GameHolder* hld = (GameHolder*) g;
     LOGW("%s -->", __FUNCTION__);
     if (!hld->game || !RenderingSystem::GetInstancePointer())
         return;

    // kill all music
    theRenderingSystem.invalidateAtlasTextures();
}

/*
 * Class:     net_damsy_soupeaucaillou_tilematch_TilematchJNILib
 * Method:    handleInputEvent
 * Signature: (JIFF)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_handleInputEvent
  (JNIEnv *env, jclass, jlong g, jint evt, jfloat x, jfloat y) {
	GameHolder* hld = (GameHolder*) g;

	/* ACTION_DOWN == 0 | ACTION_MOVE == 2 */
   if (evt == 0 || evt == 2) {
   	hld->input.touching = 1;
    	hld->input.x = x;
   	hld->input.y = y;
   }
   /* ACTION_UP == 1 */
   else if (evt == 1) {
    	hld->input.touching = 0;
   }
}

/*
 * Class:     net_damsy_soupeaucaillou_tilematch_TilematchJNILib
 * Method:    serialiazeState
 * Signature: (J)[B
 */
JNIEXPORT jbyteArray JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_serialiazeState
  (JNIEnv *env, jclass, jlong g) {
	LOGW("%s -->", __FUNCTION__);
	GameHolder* hld = (GameHolder*) g;
	uint8_t* state;
	int size = hld->game->saveState(&state);

	jbyteArray jb = 0;
	if (size) {
		jb = env->NewByteArray(size);
		env->SetByteArrayRegion(jb, 0, size, (jbyte*)state);
		LOGW("Serialized state size: %d", size);
	}

    // delete hld->game;
    // delete hld;

	LOGW("%s <--", __FUNCTION__);
	return jb;
}

/*
 * Class:     net_damsy_soupeaucaillou_tilematch_TilematchJNILib
 * Method:    restoreRenderingSystemState
 * Signature: (J[B)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_tilematch_TilematchJNILib_initAndReloadTextures
  (JNIEnv *env, jclass, jlong g) {
  LOGW("%s -->", __FUNCTION__);
  GameHolder* hld = (GameHolder*) g;
  UPDATE_ENV_PTR(hld->renderThreadEnv, env);
  theRenderingSystem.init();
  theRenderingSystem.reloadTextures();
  LOGW("%s <--", __FUNCTION__);
}

static char* loadAsset(JNIEnv *env, jobject assetManager, const std::string& assetName, int* length) {
	jclass util = env->FindClass("net/damsy/soupeaucaillou/tilematch/TilematchJNILib");
	if (!util) {
		LOGW("ERROR - cannot find class (%p)", env);
	}
	jmethodID mid = env->GetStaticMethodID(util, "assetToByteArray", "(Landroid/content/res/AssetManager;Ljava/lang/String;)[B");
    jstring asset = env->NewStringUTF(assetName.c_str());
    jobject _a = env->CallStaticObjectMethod(util, mid, assetManager, asset);

	if (_a) {
		jbyteArray a = (jbyteArray)_a;
		*length = env->GetArrayLength(a);
		jbyte* res = new jbyte[*length + 1];
		env->GetByteArrayRegion(a, 0, *length, res);
		res[*length] = '\0';
		return (char*)res;
	} else {
		LOGW("%s failed to load '%s'\n", __FUNCTION__, assetName.c_str());
		return 0;
	}
}

void read_from_buffer(png_structp png_ptr, png_bytep outBytes,
   png_size_t byteCountToRead) {
   if(png_ptr->io_ptr == NULL)
      return;   // add custom error handling here
   char* buffer = (char*)png_ptr->io_ptr;
   memcpy(outBytes, buffer, byteCountToRead);

	png_ptr->io_ptr = buffer + byteCountToRead;
}

void AndroidSuccessAPI::successCompleted(const char* description, unsigned long successId) {
	SuccessAPI::successCompleted(description, successId);
	// android spec stuff
	JNIEnv* env = holder->gameThreadEnv;
	jclass c = env->FindClass("net/damsy/soupeaucaillou/tilematch/TilematchJNILib");
	jmethodID mid = (env->GetStaticMethodID(c, "unlockAchievement", "(I)V"));
	int sid = (int) successId;
	env->CallStaticVoidMethod(c, mid, sid);
}

void AndroidSuccessAPI::openfeintLB(int mode, int diff) {
	JNIEnv* env = holder->gameThreadEnv;
	jclass c = env->FindClass("net/damsy/soupeaucaillou/tilematch/TilematchJNILib");
	jmethodID mid = env->GetStaticMethodID(c, "openfeintLeaderboard", "(II)V");
	env->CallStaticVoidMethod(c, mid, mode, diff);
}

void AndroidSuccessAPI::openfeintSuccess() {
	JNIEnv* env = holder->gameThreadEnv;
	jclass c = env->FindClass("net/damsy/soupeaucaillou/tilematch/TilematchJNILib");
	jmethodID mid = env->GetStaticMethodID(c, "openfeintSuccess", "()V");
	env->CallStaticVoidMethod(c, mid);
}
#ifdef __cplusplus
}
#endif
#endif
