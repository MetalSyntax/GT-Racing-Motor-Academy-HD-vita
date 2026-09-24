/*
 * input.c - Mapeo de controles completos (estilo Asphalt 5) para PS Vita
 * Soporta controles físicos (GamePad interno + acelerómetro sintético proporcional),
 * navegación en menús, botón Círculo (atrás/guardar) y botón Triángulo para ocultar/mostrar botones HUD.
 */
#include "input.h"
#include "utils/logger.h"

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <so_util/so_util.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern so_module so_mod;

#define INPUT_TARGET_W 960
#define INPUT_TARGET_H 544

#define TOUCH_PANEL_W 1920
#define TOUCH_PANEL_H 1088

// GT Racing solo admite pointerId 0 y 1 (2 slots max).
#define MAX_TOUCH_SLOTS 2

// Keycodes Android
#define KEY_DPAD_UP    19
#define KEY_DPAD_DOWN  20
#define KEY_DPAD_LEFT  21
#define KEY_DPAD_RIGHT 22
#define KEY_CENTER     23
#define KEY_BACK       4
#define KEY_MENU       82

static fn_touch_evt s_on_touch;
static fn_size_evt s_on_size;
static fn_key_evt s_key_down, s_key_up;
static fn_accel_evt s_accel;
static fn_orient_evt s_orient;

static int s_size_sent = 0;
static uint32_t s_prev_buttons = 0;

static int s_touch_slot_active[MAX_TOUCH_SLOTS];
static int s_touch_slot_x[MAX_TOUCH_SLOTS];
static int s_touch_slot_y[MAX_TOUCH_SLOTS];

// Control de botones en pantalla (HUD)
static bool s_hud_buttons_visible = false; // Ocultos por defecto

// Tipos de funciones internas del motor resueltas dinámicamente
typedef void *(*fn_GamePadManager_GetGamePad)(void *this_ptr, int index);
typedef void (*fn_GamePad_KeyboardKeyPressed)(void *this_ptr, int key);
typedef void (*fn_GamePad_KeyboardKeyReleased)(void *this_ptr, int key);
typedef void (*fn_RenderFX_SetVisible)(void *this_ptr, const char *name, bool visible);

static void **s_pMainGameClass = NULL;
static fn_GamePadManager_GetGamePad s_GamePadManager_GetGamePad = NULL;
static fn_GamePad_KeyboardKeyPressed s_GamePad_KeyboardKeyPressed = NULL;
static fn_GamePad_KeyboardKeyReleased s_GamePad_KeyboardKeyReleased = NULL;
static fn_RenderFX_SetVisible s_RenderFX_SetVisible = NULL;
static bool s_symbols_resolved = false;

static void resolve_engine_symbols(void) {
    if (s_symbols_resolved) return;
    s_pMainGameClass = (void **)so_symbol(&so_mod, "g_pMainGameClass");
    s_GamePadManager_GetGamePad = (fn_GamePadManager_GetGamePad)so_symbol(&so_mod, "_ZN14GamePadManager10GetGamePadEi");
    s_GamePad_KeyboardKeyPressed = (fn_GamePad_KeyboardKeyPressed)so_symbol(&so_mod, "_ZN7GamePad18KeyboardKeyPressedEi");
    s_GamePad_KeyboardKeyReleased = (fn_GamePad_KeyboardKeyReleased)so_symbol(&so_mod, "_ZN7GamePad19KeyboardKeyReleasedEi");
    s_RenderFX_SetVisible = (fn_RenderFX_SetVisible)so_symbol(&so_mod, "_ZN8RenderFX10SetVisibleEPKcb");
    s_symbols_resolved = true;
}

void input_init(fn_touch_evt on_touch, fn_size_evt on_size,
                fn_key_evt key_down, fn_key_evt key_up,
                fn_accel_evt accel, fn_orient_evt orient) {
    s_on_touch = on_touch;
    s_on_size = on_size;
    s_key_down = key_down;
    s_key_up = key_up;
    s_accel = accel;
    s_orient = orient;
    s_size_sent = 0;
    s_prev_buttons = 0;
    s_hud_buttons_visible = false;
    memset(s_touch_slot_active, 0, sizeof(s_touch_slot_active));

    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    resolve_engine_symbols();
}

static void touch_to_target(int px, int py, int *ox, int *oy) {
    *ox = px * INPUT_TARGET_W / TOUCH_PANEL_W;
    *oy = py * INPUT_TARGET_H / TOUCH_PANEL_H;
    if (*ox < 0) *ox = 0;
    if (*ox >= INPUT_TARGET_W) *ox = INPUT_TARGET_W - 1;
    if (*oy < 0) *oy = 0;
    if (*oy >= INPUT_TARGET_H) *oy = INPUT_TARGET_H - 1;
}

static void poll_real_touch(void *env, void *clazz) {
    if (!s_on_touch) return;
    SceTouchData touch;
    if (sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1) < 0) return;

    int seen[MAX_TOUCH_SLOTS] = {0};
    for (int i = 0; i < (int)touch.reportNum && i < MAX_TOUCH_SLOTS; i++) {
        int x, y;
        touch_to_target(touch.report[i].x, touch.report[i].y, &x, &y);
        seen[i] = 1;
        if (!s_touch_slot_active[i]) {
            s_touch_slot_active[i] = 1;
            s_touch_slot_x[i] = x;
            s_touch_slot_y[i] = y;
            s_on_touch(env, clazz, 1, x, y, i); // press
        } else if (s_touch_slot_x[i] != x || s_touch_slot_y[i] != y) {
            s_touch_slot_x[i] = x;
            s_touch_slot_y[i] = y;
            s_on_touch(env, clazz, 2, x, y, i); // drag
        }
    }
    for (int i = 0; i < MAX_TOUCH_SLOTS; i++) {
        if (s_touch_slot_active[i] && !seen[i]) {
            s_touch_slot_active[i] = 0;
            s_on_touch(env, clazz, 0, s_touch_slot_x[i], s_touch_slot_y[i], i); // release
        }
    }
}

static void key_edge(void *env, void *clazz, uint32_t now, uint32_t before,
                     uint32_t mask, int code) {
    int was = (before & mask) != 0, is = (now & mask) != 0;
    if (is && !was && s_key_down) s_key_down(env, clazz, code);
    if (!is && was && s_key_up) s_key_up(env, clazz, code);
}

static void update_hud_visibility(void) {
    if (!s_pMainGameClass || !s_RenderFX_SetVisible) return;
    void *game = *s_pMainGameClass;
    if (!game) return;

    void *pFxHUD = *(void **)((uintptr_t)game + 0x1a70);
    if (pFxHUD) {
        s_RenderFX_SetVisible(pFxHUD, "HUD.Direction", s_hud_buttons_visible);
        s_RenderFX_SetVisible(pFxHUD, "HUD.AccelBrake", s_hud_buttons_visible);
        s_RenderFX_SetVisible(pFxHUD, "HUD.RenderWheel", s_hud_buttons_visible);
        s_RenderFX_SetVisible(pFxHUD, "HUD.Gears", s_hud_buttons_visible);
    }
}

static void poll_pad_and_controls(void *env, void *clazz) {
    SceCtrlData pad;
    if (sceCtrlPeekBufferPositive(0, &pad, 1) < 0) return;
    uint32_t now = pad.buttons;
    uint32_t changed = now ^ s_prev_buttons;

    resolve_engine_symbols();

    // 1. Botón Triángulo: Alternar visibilidad de botones táctiles en pantalla (HUD)
    if ((changed & SCE_CTRL_TRIANGLE) && (now & SCE_CTRL_TRIANGLE)) {
        s_hud_buttons_visible = !s_hud_buttons_visible;
        l_info("input: botones en pantalla HUD %s", s_hud_buttons_visible ? "VISIBLES" : "OCULTOS");
    }
    update_hud_visibility();

    // 2. Botón Círculo: Regresar / Guardar en menús
    // El motor Android responde al toque en la esquina superior izquierda (x=35, y=35)
    static int circle_tap_active = 0;
    if ((changed & SCE_CTRL_CIRCLE) && (now & SCE_CTRL_CIRCLE)) {
        if (s_on_touch && !s_touch_slot_active[0]) {
            circle_tap_active = 1;
            s_on_touch(env, clazz, 1, 35, 35, 0); // Presiona botón Atrás
        }
    } else if (circle_tap_active && !(now & SCE_CTRL_CIRCLE)) {
        circle_tap_active = 0;
        if (s_on_touch) {
            s_on_touch(env, clazz, 0, 35, 35, 0); // Libera botón Atrás
        }
    }

    // 3. Botón START: Pausa (toque en x=35, y=35 durante carrera + KEY_MENU)
    static int start_tap_active = 0;
    if ((changed & SCE_CTRL_START) && (now & SCE_CTRL_START)) {
        if (s_on_touch && !s_touch_slot_active[0] && !circle_tap_active) {
            start_tap_active = 1;
            s_on_touch(env, clazz, 1, 35, 35, 0);
        }
    } else if (start_tap_active && !(now & SCE_CTRL_START)) {
        start_tap_active = 0;
        if (s_on_touch) {
            s_on_touch(env, clazz, 0, 35, 35, 0);
        }
    }
    key_edge(env, clazz, now, s_prev_buttons, SCE_CTRL_START, KEY_MENU);

    // 4. Navegación D-Pad en menús
    key_edge(env, clazz, now, s_prev_buttons, SCE_CTRL_UP, KEY_DPAD_UP);
    key_edge(env, clazz, now, s_prev_buttons, SCE_CTRL_DOWN, KEY_DPAD_DOWN);
    key_edge(env, clazz, now, s_prev_buttons, SCE_CTRL_LEFT, KEY_DPAD_LEFT);
    key_edge(env, clazz, now, s_prev_buttons, SCE_CTRL_RIGHT, KEY_DPAD_RIGHT);
    key_edge(env, clazz, now, s_prev_buttons, SCE_CTRL_CROSS, KEY_CENTER);

    // 5. Controles de Conducción en Carrera:
    // Freno: L Trigger o Botón Cuadrado o D-Pad Abajo
    int brake_down = (now & (SCE_CTRL_LTRIGGER | SCE_CTRL_SQUARE | SCE_CTRL_DOWN)) != 0;

    // Acelerador: R Trigger o Botón Cruz o D-Pad Arriba
    int gas_down = (now & (SCE_CTRL_RTRIGGER | SCE_CTRL_CROSS | SCE_CTRL_UP)) != 0;

    // Dirección: Stick Analógico Izquierdo y D-Pad Izquierda/Derecha (-1.0f a +1.0f)
    float synth_steer = 0.0f;
    if (now & SCE_CTRL_LEFT) {
        synth_steer = -1.0f;
    } else if (now & SCE_CTRL_RIGHT) {
        synth_steer = 1.0f;
    } else {
        int lx_diff = (int)pad.lx - 128;
        if (lx_diff < -20 || lx_diff > 20) {
            synth_steer = (float)lx_diff / 128.0f;
            if (synth_steer < -1.0f) synth_steer = -1.0f;
            if (synth_steer > 1.0f) synth_steer = 1.0f;
        }
    }

    int steer_left = (synth_steer < -0.2f);
    int steer_right = (synth_steer > 0.2f);

    // Actualizar GamePad interno del motor (para modo SideButtons / Teclado)
    if (s_pMainGameClass && s_GamePadManager_GetGamePad && s_GamePad_KeyboardKeyPressed && s_GamePad_KeyboardKeyReleased) {
        void *game = *s_pMainGameClass;
        if (game) {
            void *pMgr = *(void **)((uintptr_t)game + 0x1720);
            if (pMgr) {
                void *pPad = s_GamePadManager_GetGamePad(pMgr, 0);
                if (pPad) {
                    if (steer_left) s_GamePad_KeyboardKeyPressed(pPad, 1);
                    else s_GamePad_KeyboardKeyReleased(pPad, 1);

                    if (steer_right) s_GamePad_KeyboardKeyPressed(pPad, 2);
                    else s_GamePad_KeyboardKeyReleased(pPad, 2);

                    if (gas_down) s_GamePad_KeyboardKeyPressed(pPad, 4);
                    else s_GamePad_KeyboardKeyReleased(pPad, 4);

                    if (brake_down) s_GamePad_KeyboardKeyPressed(pPad, 8);
                    else s_GamePad_KeyboardKeyReleased(pPad, 8);
                }
            }
        }
    }

    // Actualizar Acelerómetro sintético proporcional (para modo Acelerómetro)
    // En modo horizontal (landscape):
    // La aceleración de giro se evalúa en el eje Y (con centro 0.0f y rango +/- 8.5 m/s²).
    // El eje X mantiene la gravedad normal horizontal (9.8 m/s²).
    if (s_accel) {
        float accel_x = 9.8f;
        float accel_y = synth_steer * 8.5f;
        float accel_z = 0.0f;
        s_accel(env, clazz, accel_x, accel_y, accel_z);
    }
    if (s_orient) {
        s_orient(env, clazz, 1); // Orientación horizontal 1 (Landscape)
    }

    s_prev_buttons = now;
}

void input_poll(void *env, void *clazz) {
    if (s_on_size && !s_size_sent) {
        s_on_size(env, clazz, INPUT_TARGET_W, INPUT_TARGET_H);
        s_size_sent = 1;
    }
    poll_real_touch(env, clazz);
    poll_pad_and_controls(env, clazz);
}
