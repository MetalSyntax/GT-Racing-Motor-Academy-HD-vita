/*
 * Input del port: panel tactil frontal + botones + acelerometro de la Vita
 * hacia los entry points nativos del motor (ver GameGLSurfaceView.java y
 * GTRacing.java en decompiled/apk_jadx/sources/):
 *
 *   GameGLSurfaceView.nativeOnSize(env,clazz,w,h)
 *   GameGLSurfaceView.nativeOnTouch(env,clazz,tipo,x,y,id)
 *     tipo: 1 = press, 2 = move/drag, 0 = release (rama sin TouchFix de jadx);
 *     x,y enteros en 960x544; id = pointerId (0..N, chico y estable).
 *   GTRacing.nativeKeyDown/Up(env,clazz,keycode) + nativegetState(int)
 *     keycodes Android ( DPAD 19-22, CENTER 23, BACK 4, MENU 82 ).
 *   GTRacing.nativeAccelerometer(env,clazz,x,y,z) + nativeSetOrientation(+-1)
 *     el sensor suma +1.0f a Y y dispara orientacion con |x| > 8.0 (jadx).
 *
 * Sin calibrar en consola real: el mapeo de botones es inicial y razonable
 * (dpad + CROSS=CENTER + CIRCLE=BACK + START=MENU); ajustar con el primer log
 * si los menus no responden. Sin touches sinteticos posicionados: el layout
 * del HUD se desconoce hasta ver screenshots reales.
 */
#ifndef GTRACING_INPUT_H
#define GTRACING_INPUT_H

#include <falso_jni/FalsoJNI.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* fn_touch_evt)(void *env, void *clazz, jint tipo, jint x, jint y, jint id);
typedef void (* fn_size_evt)(void *env, void *clazz, jint w, jint h);
typedef void (* fn_key_evt)(void *env, void *clazz, jint keycode);
typedef void (* fn_accel_evt)(void *env, void *clazz, jfloat x, jfloat y, jfloat z);
typedef void (* fn_orient_evt)(void *env, void *clazz, jint orient);

/** Guarda los entry points nativos resueltos. Cualquiera puede ser NULL. */
void input_init(fn_touch_evt on_touch, fn_size_evt on_size,
                fn_key_evt key_down, fn_key_evt key_up,
                fn_accel_evt accel, fn_orient_evt orient);

/** Lee touch + pad + motion y dispara los callbacks. Una vez por frame. */
void input_poll(void *env, void *clazz);

#ifdef __cplusplus
};
#endif

#endif // GTRACING_INPUT_H
