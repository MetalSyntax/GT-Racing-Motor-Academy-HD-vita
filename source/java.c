#include <falso_jni/FalsoJNI_Impl.h>
#include <falso_jni/FalsoJNI_ImplBridge.h>
#include "generated_jni_table.h"
#include "audio.h"
#include "video.h"
#include "utils/logger.h"
#include <string.h>

static const char * jstring_to_cstr(jobject jstr) {
    if (!jstr)
        return NULL;
    JavaString * js = (JavaString *) jstr;
    if (!js->utf8 || !js->utf8->array)
        return NULL;
    return (const char *) js->utf8->array;
}

jint GLMediaPlayer_loadMovie(jmethodID id, va_list args) {
    jobject arg0 = (jobject) va_arg(args, jobject);
    const char *name = jstring_to_cstr(arg0);
    l_info("[movie] GLMediaPlayer.loadMovie(%s): starting intro video playback", name ? name : "NULL");
    if (!name || strlen(name) == 0) {
        name = "gtracing.mp4";
    }
    video_play(name);
    l_info("[movie] GLMediaPlayer.loadMovie(%s): video_play finished", name);
    return 1;
}

/*
 * Tablas JNI reales. Entradas copiadas de generated_jni_table.h
 * (candidatos de psvita-toolkit jni-analyze): confirmar cada metodo
 * contra el Java real (jadx) antes de confiar en el cuerpo del stub.
 * Ver PORTING_PLAN.md seccion 3 y decompiled/apk_jadx/sources/.
 */

NameToMethodID nameToMethodId[] = {
    { 0, "GetPhoneManufacturer", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 1, "IsPriority", METHOD_TYPE_BOOLEAN },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 2, "MyOpenBrowser", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 3, "MyOpenGLive", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 4, "My_sendAppToBackground", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 5, "PushOldName", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 6, "ReinitSoundPool", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 7, "VGetChar", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 8, "VOutPutStrLen", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 9, "VResetInfo", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 10, "VirtualKeyBoard", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 11, "destroySoundPool", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 12, "detectPhoneLang", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 13, "init", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 14, "init", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 15, "initSoundPoolArray", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 16, "isMediaPlaying", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 17, "isSoundLoaded", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 18, "isSoundLoadedBig", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 19, "loadMovie", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 20, "loadSound", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 21, "loadSound", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 22, "loadSoundBig", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 23, "no_of_touch", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 24, "pauseSound", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 25, "pauseSoundBig", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 26, "playSound", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 27, "playSoundBig", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 28, "releaseSoundPool", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 29, "resetSound", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 30, "restoreMasterVolume", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 31, "resumeSound", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 32, "resumeSoundBig", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 33, "sendTrackingInfo", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 34, "setPitch", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 35, "setVolume", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 36, "setVolumeBig", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 37, "stopAllBig", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 38, "stopAllPool", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 39, "stopAllSounds", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 40, "stopSound", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 41, "stopSoundBig", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 42, "unloadSound", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 43, "unloadSoundBig", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer
    { 44, "getRawResource", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLResLoader
    { 45, "getResourceBytes", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLResLoader
    // NOTA: "getResourceFull" esta duplicado (int id 46 y String id 47). FalsoJNI
    // resuelve por NOMBRE y devuelve el primero: el nativo solo pide "(String)[B"
    // (ver GLResLoader_nativeInit en out_ghidra.c), asi que el String va primero.
    // Si se reordena, GetNativeResourceFull recibe el stub (I) y rompe assets.
    { 47, "getResourceFull", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLResLoader
    { 46, "getResourceFull", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLResLoader
    { 48, "getResourceLength", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLResLoader
    { 49, "getString", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLResLoader
    { 50, "init", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GLResLoader
    { 51, "DisableLaunchGame", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 52, "Displaydebug", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 53, "Exit", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 54, "GetDoubleOptionText1", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 55, "GetDoubleOptionText2", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 56, "GetDoubleOptionText3", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 57, "Get_PhoneManufacturer", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 58, "IncreaseLaunchTimes", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 59, "IsDemo", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 60, "IsDoubleOption", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 61, "LaunchBilling", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 62, "OpenGLive", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 63, "OpenIGP", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 64, "VZGetGameName", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 65, "VZGetGamePrice", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 66, "VZInitMobileNetwork", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 67, "VZIsErrorOcurred", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 68, "VZIsInProgress", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 69, "VZIsMobileNetworkReady", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 70, "VZRequestLogin", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 71, "VZRequestPurchaseGame", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 72, "VZRestoreNetworkState", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 73, "createNomedia", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 74, "isDemo", METHOD_TYPE_BOOLEAN },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 75, "isWifiAlive", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 76, "lockDemo", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 77, "openBrowser", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 78, "sendAppToBackground", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 79, "unlockDemo", METHOD_TYPE_INT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 80, "a", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 81, "b", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 82, "onAccuracyChanged", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 83, "onCreate", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 84, "onDestroy", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 85, "onKeyDown", METHOD_TYPE_BOOLEAN },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 86, "onKeyUp", METHOD_TYPE_BOOLEAN },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 87, "onPause", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 88, "onRestart", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 89, "onResume", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 90, "onSensorChanged", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 91, "onStart", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 92, "onStop", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 93, "onTrackballEvent", METHOD_TYPE_BOOLEAN },  // com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing
    { 94, "GameGLSurfaceView", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GameGLSurfaceView
    { 95, "onSizeChanged", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GameGLSurfaceView
    { 96, "onTouchEvent", METHOD_TYPE_BOOLEAN },  // com.gameloft.android.TBFV.GloftGTHP.ML.GameGLSurfaceView
    { 97, "onWindowFocusChanged", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GameGLSurfaceView
    { 98, "GameRenderer", METHOD_TYPE_OBJECT },  // com.gameloft.android.TBFV.GloftGTHP.ML.GameRenderer
    { 99, "onDrawFrame", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GameRenderer
    { 100, "onSurfaceChanged", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GameRenderer
    { 101, "onSurfaceCreated", METHOD_TYPE_VOID },  // com.gameloft.android.TBFV.GloftGTHP.ML.GameRenderer
};

MethodsBoolean methodsBoolean[] = {
    { 1, stub_GLMediaPlayer_IsPriority_1 },
    { 74, stub_GTRacing_isDemo_74 },
    { 85, stub_GTRacing_onKeyDown_85 },
    { 86, stub_GTRacing_onKeyUp_86 },
    { 93, stub_GTRacing_onTrackballEvent_93 },
    { 96, stub_GameGLSurfaceView_onTouchEvent_96 },
};
MethodsByte methodsByte[] = {};
MethodsChar methodsChar[] = {};
MethodsDouble methodsDouble[] = {};
MethodsFloat methodsFloat[] = {};
MethodsInt methodsInt[] = {
    { 0, stub_GLMediaPlayer_GetPhoneManufacturer_0 },
    { 7, stub_GLMediaPlayer_VGetChar_7 },
    { 8, stub_GLMediaPlayer_VOutPutStrLen_8 },
    { 12, stub_GLMediaPlayer_detectPhoneLang_12 },
    { 16, GLMediaPlayer_isMediaPlaying },
    { 17, GLMediaPlayer_isSoundLoaded },
    { 18, GLMediaPlayer_isSoundLoadedBig },
    { 19, GLMediaPlayer_loadMovie },
    { 23, stub_GLMediaPlayer_no_of_touch_23 },
    { 48, stub_GLResLoader_getResourceLength_48 },
    { 51, stub_GTRacing_DisableLaunchGame_51 },
    { 57, stub_GTRacing_Get_PhoneManufacturer_57 },
    { 59, stub_GTRacing_IsDemo_59 },
    { 60, stub_GTRacing_IsDoubleOption_60 },
    { 67, stub_GTRacing_VZIsErrorOcurred_67 },
    { 68, stub_GTRacing_VZIsInProgress_68 },
    { 69, stub_GTRacing_VZIsMobileNetworkReady_69 },
    { 75, stub_GTRacing_isWifiAlive_75 },
    { 79, stub_GTRacing_unlockDemo_79 },
};
MethodsLong methodsLong[] = {};
MethodsObject methodsObject[] = {
    { 44, stub_GLResLoader_getRawResource_44 },
    { 45, stub_GLResLoader_getResourceBytes_45 },
    { 46, stub_GLResLoader_getResourceFull_46 },
    { 47, stub_GLResLoader_getResourceFull_47 },
    { 49, stub_GLResLoader_getString_49 },
    { 54, stub_GTRacing_GetDoubleOptionText1_54 },
    { 55, stub_GTRacing_GetDoubleOptionText2_55 },
    { 56, stub_GTRacing_GetDoubleOptionText3_56 },
    { 64, stub_GTRacing_VZGetGameName_64 },
    { 65, stub_GTRacing_VZGetGamePrice_65 },
    { 94, stub_GameGLSurfaceView_GameGLSurfaceView_94 },
    { 98, stub_GameRenderer_GameRenderer_98 },
};
MethodsShort methodsShort[] = {};
MethodsVoid methodsVoid[] = {
    { 2, stub_GLMediaPlayer_MyOpenBrowser_2 },
    { 3, stub_GLMediaPlayer_MyOpenGLive_3 },
    { 4, stub_GLMediaPlayer_My_sendAppToBackground_4 },
    { 5, stub_GLMediaPlayer_PushOldName_5 },
    { 6, stub_GLMediaPlayer_ReinitSoundPool_6 },
    { 9, stub_GLMediaPlayer_VResetInfo_9 },
    { 10, stub_GLMediaPlayer_VirtualKeyBoard_10 },
    { 11, GLMediaPlayer_destroySoundPool },
    { 13, stub_GLMediaPlayer_init_13 },
    { 14, stub_GLMediaPlayer_init_14 },
    { 15, GLMediaPlayer_initSoundPoolArray },
    { 20, GLMediaPlayer_loadSound },
    { 21, GLMediaPlayer_loadSound },
    { 22, GLMediaPlayer_loadSoundBig },
    { 24, GLMediaPlayer_pauseSound },
    { 25, GLMediaPlayer_pauseSoundBig },
    { 26, GLMediaPlayer_playSound },
    { 27, GLMediaPlayer_playSoundBig },
    { 28, GLMediaPlayer_destroySoundPool },
    { 29, GLMediaPlayer_resetSound },
    { 30, stub_GLMediaPlayer_restoreMasterVolume_30 },
    { 31, GLMediaPlayer_resumeSound },
    { 32, GLMediaPlayer_resumeSoundBig },
    { 33, stub_GLMediaPlayer_sendTrackingInfo_33 },
    { 34, GLMediaPlayer_setPitch },
    { 35, GLMediaPlayer_setVolume },
    { 36, GLMediaPlayer_setVolumeBig },
    { 37, GLMediaPlayer_stopAllBig },
    { 38, GLMediaPlayer_stopAllPool },
    { 39, GLMediaPlayer_stopAllSounds },
    { 40, GLMediaPlayer_stopSound },
    { 41, GLMediaPlayer_stopSoundBig },
    { 42, GLMediaPlayer_unloadSound },
    { 43, GLMediaPlayer_unloadSoundBig },
    { 50, stub_GLResLoader_init_50 },
    { 52, stub_GTRacing_Displaydebug_52 },
    { 53, stub_GTRacing_Exit_53 },
    { 58, stub_GTRacing_IncreaseLaunchTimes_58 },
    { 61, stub_GTRacing_LaunchBilling_61 },
    { 62, stub_GTRacing_OpenGLive_62 },
    { 63, stub_GTRacing_OpenIGP_63 },
    { 66, stub_GTRacing_VZInitMobileNetwork_66 },
    { 70, stub_GTRacing_VZRequestLogin_70 },
    { 71, stub_GTRacing_VZRequestPurchaseGame_71 },
    { 72, stub_GTRacing_VZRestoreNetworkState_72 },
    { 73, stub_GTRacing_createNomedia_73 },
    { 76, stub_GTRacing_lockDemo_76 },
    { 77, stub_GTRacing_openBrowser_77 },
    { 78, stub_GTRacing_sendAppToBackground_78 },
    { 80, stub_GTRacing_a_80 },
    { 81, stub_GTRacing_b_81 },
    { 82, stub_GTRacing_onAccuracyChanged_82 },
    { 83, stub_GTRacing_onCreate_83 },
    { 84, stub_GTRacing_onDestroy_84 },
    { 87, stub_GTRacing_onPause_87 },
    { 88, stub_GTRacing_onRestart_88 },
    { 89, stub_GTRacing_onResume_89 },
    { 90, stub_GTRacing_onSensorChanged_90 },
    { 91, stub_GTRacing_onStart_91 },
    { 92, stub_GTRacing_onStop_92 },
    { 95, stub_GameGLSurfaceView_onSizeChanged_95 },
    { 97, stub_GameGLSurfaceView_onWindowFocusChanged_97 },
    { 99, stub_GameRenderer_onDrawFrame_99 },
    { 100, stub_GameRenderer_onSurfaceChanged_100 },
    { 101, stub_GameRenderer_onSurfaceCreated_101 },
};

/* JNI Fields (curados a mano, no generados) */
char WINDOW_SERVICE[] = "window";
const int SDK_INT = 19;

NameToFieldID nameToFieldId[] = {
		{ 0, "WINDOW_SERVICE", FIELD_TYPE_OBJECT }, 
		{ 1, "SDK_INT", FIELD_TYPE_INT },
};

FieldsBoolean fieldsBoolean[] = {};
FieldsByte fieldsByte[] = {};
FieldsChar fieldsChar[] = {};
FieldsDouble fieldsDouble[] = {};
FieldsFloat fieldsFloat[] = {};
FieldsInt fieldsInt[] = {
		{ 1, SDK_INT },
};
FieldsObject fieldsObject[] = {
		{ 0, WINDOW_SERVICE },
};
FieldsLong fieldsLong[] = {};
FieldsShort fieldsShort[] = {};

__FALSOJNI_IMPL_CONTAINER_SIZES
