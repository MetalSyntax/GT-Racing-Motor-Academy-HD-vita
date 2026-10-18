#include "utils/init.h"
#include "utils/glutil.h"
#include "utils/logger.h"
#include "input.h"
#include "audio.h"
#include "video.h"

#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/power.h>

#include <falso_jni/FalsoJNI.h>
#include <so_util/so_util.h>

int _newlib_heap_size_user = 192 * 1024 * 1024;

#ifdef USE_SCELIBC_IO
int sceLibcHeapSize = 4 * 1024 * 1024;
#endif

so_module so_mod;

// Vita framebuffer / touch coordinate space (gl_init() uses 960x544).
#define SCREEN_W 960
#define SCREEN_H 544

// GTRacing.nativeInit(demo): jadx GameRenderer.onSurfaceCreated() passes
// isDemo()?1:0; k==0 on first boot -> demo build. Matches offline .ba1 data.
#define ARG_DEMO_FIRST_BOOT 1

// GTRacing.nativeSetPhone(widthPixels): Android passes portrait screen width
// (i9100 target device -> 480). We pass the Vita landscape width; if the first
// real log shows a wrong asset profile/scale, try 480 or 800 here.
#define ARG_PHONE_WIDTH 960

int main() {
    // Game/render thread owns core 0 (audio mixer is pinned to core 1, audio loader
    // and log writer to core 2), like Asphalt 6, so nothing preempts it mid-frame.
    sceKernelChangeThreadCpuAffinityMask(0, SCE_KERNEL_CPU_MASK_USER_0);
    soloader_init_all();

    // This .so has NO JNI_OnLoad and NO RegisterNatives (confirmed: 0 hits in
    // disasm + binary strings; 27 static Java_* exports). Static resolution via
    // so_symbol() only -- never call a NULL JNI_OnLoad.
    int (* JNI_OnLoad)(void *jvm) = (void *)so_symbol(&so_mod, "JNI_OnLoad");
    if (JNI_OnLoad) {
        l_info("boot: JNI_OnLoad @ %p, llamando...", JNI_OnLoad);
        JNI_OnLoad(&jvm);
    } else {
        l_info("boot: sin JNI_OnLoad (esperado: JNI estatica), sigo.");
    }

    gl_init();
    video_init();
    audio_init();

    // Lifecycle order from jadx GameRenderer.onSurfaceCreated():
    //   nativeGetJNIEnv() -> GLResLoader.init()[nativeInit(0)] ->
    //   GLMediaPlayer.init()[nativeInit(0)] -> GTRacing.nativeInit(demo) ->
    //   GameRenderer.nativeInit(1)
    // Every lookup is NULL-guarded: a missing export logs and continues so the
    // first real console log shows exactly which step is absent.
    void (* GameRenderer_nativeGetJNIEnv)(void *env, void *thiz) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameRenderer_nativeGetJNIEnv");
    void (* GLResLoader_nativeInit)(void *env, void *thiz, int arg) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GLResLoader_nativeInit");
    void (* GLMediaPlayer_nativeInit)(void *env, void *thiz, int arg) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GLMediaPlayer_nativeInit");
    void (* GTRacing_nativeInit)(void *env, void *thiz, int demo) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeInit");
    void (* GTRacing_nativeSetPhone)(void *env, void *thiz, int width) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeSetPhone");
    void (* GameRenderer_nativeInit)(void *env, void *thiz, int arg) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameRenderer_nativeInit");
    void (* GameRenderer_nativeResize)(void *env, void *thiz, int w, int h) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameRenderer_nativeResize");
    void (* GameRenderer_nativeRender)(void *env, void *thiz) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameRenderer_nativeRender");
    void (* GameGLSurfaceView_nativeOnSize)(void *env, void *thiz, int w, int h) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameGLSurfaceView_nativeOnSize");
    void (* GameGLSurfaceView_nativeOnTouch)(void *env, void *thiz, int tipo, int x, int y, int id) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameGLSurfaceView_nativeOnTouch");
    void (* GTRacing_nativeKeyDown)(void *env, void *thiz, int keycode) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeKeyDown");
    void (* GTRacing_nativeKeyUp)(void *env, void *thiz, int keycode) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeKeyUp");
    void (* GTRacing_nativeAccelerometer)(void *env, void *thiz, float x, float y, float z) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeAccelerometer");
    void (* GTRacing_nativeSetOrientation)(void *env, void *thiz, int orient) =
        (void *)so_symbol(&so_mod, "Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeSetOrientation");

    // NOTE (deferred, telemetry only): GTRacing.onCreate() also calls
    // nativeGetInfo(country, deviceId, MANUFACTURER_MODEL, RELEASE) and
    // nativeSetPhone(width). nativeSetPhone is wired above; nativeGetInfo needs
    // 4 jstrings and FalsoJNI here has no NewStringUTF helper yet -- skipped
    // until the first console log shows whether the .so path needs it.

    int * os_screen_w = (int *) so_symbol(&so_mod, "OS_SCREEN_W");
    if (os_screen_w) {
        *os_screen_w = SCREEN_W;
        l_info("boot: OS_SCREEN_W override to %d", SCREEN_W);
    }
    int * os_screen_h = (int *) so_symbol(&so_mod, "OS_SCREEN_H");
    if (os_screen_h) {
        *os_screen_h = SCREEN_H;
        l_info("boot: OS_SCREEN_H override to %d", SCREEN_H);
    } else {
        l_warn("boot: OS_SCREEN_H symbol not found");
    }

    if (GameRenderer_nativeGetJNIEnv) {
        l_info("boot: GameRenderer_nativeGetJNIEnv");
        GameRenderer_nativeGetJNIEnv(&jni, &jni);
    } else {
        l_error("boot: falta export GameRenderer_nativeGetJNIEnv");
    }

    if (GTRacing_nativeSetPhone) {
        l_info("boot: GTRacing_nativeSetPhone(%d)", ARG_PHONE_WIDTH);
        GTRacing_nativeSetPhone(&jni, &jni, ARG_PHONE_WIDTH);
    } else {
        l_error("boot: falta export GTRacing_nativeSetPhone");
    }

    if (GLResLoader_nativeInit) {
        l_info("boot: GLResLoader_nativeInit(0)");
        GLResLoader_nativeInit(&jni, &jni, 0);
    } else {
        l_error("boot: falta export GLResLoader_nativeInit");
    }

    if (GLMediaPlayer_nativeInit) {
        l_info("boot: GLMediaPlayer_nativeInit(0)");
        GLMediaPlayer_nativeInit(&jni, &jni, 0);
    } else {
        l_error("boot: falta export GLMediaPlayer_nativeInit");
    }

    if (GTRacing_nativeInit) {
        l_info("boot: GTRacing_nativeInit(%d)", ARG_DEMO_FIRST_BOOT);
        GTRacing_nativeInit(&jni, &jni, ARG_DEMO_FIRST_BOOT);
    } else {
        l_error("boot: falta export GTRacing_nativeInit");
    }

    if (GameRenderer_nativeInit) {
        l_info("boot: GameRenderer_nativeInit(1)");
        GameRenderer_nativeInit(&jni, &jni, 1);
        l_info("boot: GameRenderer_nativeInit completado");
        log_flush();
    } else {
        l_error("boot: falta export GameRenderer_nativeInit");
    }

    if (GameRenderer_nativeResize) {
        l_info("boot: GameRenderer_nativeResize(%d,%d)", SCREEN_W, SCREEN_H);
        GameRenderer_nativeResize(&jni, &jni, SCREEN_W, SCREEN_H);
        l_info("boot: GameRenderer_nativeResize completado");
        log_flush();
    } else {
        l_error("boot: falta export GameRenderer_nativeResize");
    }

    // Main loop mirrors GameRenderer.onDrawFrame(): input_poll() feeds touch,
    // keys and accelerometer like the Activity did, then nativeRender().
    // Throttle to ~33ms (~30 fps) like the Thread.sleep(33-dt) in Java.
    if (!GameRenderer_nativeRender)
        l_error("boot: falta export GameRenderer_nativeRender, loop sin render");

    input_init((void *)GameGLSurfaceView_nativeOnTouch,
               (void *)GameGLSurfaceView_nativeOnSize,
               (void *)GTRacing_nativeKeyDown, (void *)GTRacing_nativeKeyUp,
               (void *)GTRacing_nativeAccelerometer,
               (void *)GTRacing_nativeSetOrientation);

    l_info("boot: entrando en main loop de render");
    log_flush();

    while (1) {
        sceKernelPowerTick(0);
        unsigned int t0 = sceKernelGetProcessTimeLow();
        input_poll(&jni, &jni);
        unsigned int t1 = sceKernelGetProcessTimeLow();
        if (GameRenderer_nativeRender)
            GameRenderer_nativeRender(&jni, &jni);
        unsigned int t2 = sceKernelGetProcessTimeLow();
        gl_swap();
        unsigned int dt = sceKernelGetProcessTimeLow() - t0;
        // Frame budget breakdown (input / game update+GL calls / swap) to tell CPU from GPU bound.
        static unsigned int frame_counter = 0, sum_in = 0, sum_game = 0, sum_swap = 0, max_dt = 0;
        sum_in += t1 - t0; sum_game += t2 - t1; sum_swap += dt - (t2 - t0);
        if (dt > max_dt) max_dt = dt;
        if ((++frame_counter % 60) == 0) {
            // gl_draw = part of `game` spent inside vitaGL draw calls (CPU-side GL overhead).
            l_info("perf: avg input=%.1fms game=%.1fms (gl_draw=%.1fms, %u draws) swap=%.1fms total=%.1fms (max %.1fms)",
                   sum_in / 60000.0f, sum_game / 60000.0f, gl_draw_us / 60000.0f, gl_draw_count / 60,
                   sum_swap / 60000.0f, (sum_in + sum_game + sum_swap) / 60000.0f, max_dt / 1000.0f);
            sum_in = sum_game = sum_swap = max_dt = 0;
            gl_draw_us = gl_draw_count = 0;
            log_flush();
        }
        if (dt < 33000)
            sceKernelDelayThread(33000 - dt);
    }

    sceKernelExitDeleteThread(0);
    return 0;
}
