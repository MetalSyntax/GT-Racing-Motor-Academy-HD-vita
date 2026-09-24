# Plan de Port — GT Racing Motor Academy HD (PS Vita)

> Generado por psvita-port-toolkit el 2026-09-08. Punto de partida con lo detectado automáticamente --
confirmar todo con objdump/Ghidra/jadx a mano antes de asumirlo como cierto.

## 0. Contexto

- **Juego:** GT Racing Motor Academy HD
- **Paquete Java:** com.gameloft.android.TBFV.GloftGTHP.ML
- **APK original:** `GTRacing_3.2.8.apk`
- **TITLEID asignado:** `PSVGTRMA1`

**¿Motor conocido?** Revisar si algún port hermano (bajo la misma BASE_DIR) comparte motor antes de
reusar su código -- confirmar con símbolos JNI reales, no por analogía superficial.

## 1. Detección automática

- **ABI(s):** armeabi
- **ABI elegida:** armeabi
- **Nota de arquitectura:** Solo armeabi (ARMv6, soft-float) -- Vita lo ejecuta igual (ARMv7 es superset), sin NEON de v7a.
- **Versión de GLES:** AndroidManifest.xml no declara glEsVersion -- usar heurística (GLES1 (pipeline fijo: glVertexPointer/glClearColorx/glTexParameterx))

## 2. .so encontrados (ABI armeabi)

- `gtracingmotoracademyhd_extract/lib/armeabi/libGTRacing.so` (4284 KB)


## 3. Exports JNI (convención `Java_*`)

- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GLMediaPlayer_nativeGetTotalSounds`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GLMediaPlayer_nativeGetTotalSoundsOfSameInstance`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GLMediaPlayer_nativeInit`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GLMediaPlayer_nativeSetStopOnMusic`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GLResLoader_nativeInit`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeAccelerometer`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeCanInterrupt`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeGetCurrentStateID`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeGetInfo`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeInit`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeKeyDown`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeKeyUp`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeOpenIGM`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativePause`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeQuitGame`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeResume`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeResumeInternal`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeSetOrientation`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeSetPhone`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativegetState`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GTRacing_nativeonTrackballEvent`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameGLSurfaceView_nativeOnSize`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameGLSurfaceView_nativeOnTouch`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameRenderer_nativeDone`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameRenderer_nativeGetJNIEnv`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameRenderer_nativeInit`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameRenderer_nativeRender`
- `Java_com_gameloft_android_TBFV_GloftGTHP_ML_GameRenderer_nativeResize`


## 4. Checklist

- [x] Repo creado desde soloader-boilerplate, git init, .gitignore anti-DMCA.
- [x] APK decompilado (jadx) y .so decompilado(s) (Ghidra) -- ver sección 2/3.
- [x] Análisis del motor real (2026-09-22, ver sección 6 -- NO reusar Hyundai: otro paquete/ABI JNI).
- [x] Bootstrap del loader: so_file_load/so_relocate/so_resolve, primer build.
- [x] Tabla JNI (FalsoJNI): 102 candidatos cableados en java.c (curaduría contra jadx pendiente).
- [ ] Primer arranque en consola real (eboot.bin/VPK compilan en host -- falta deploy + log).
- [ ] Gráficos (wrappers GL según versión detectada: GLES1 confirmado, dynlib.c ya mapea).
- [ ] Input, Audio, Assets, LiveArea/VPK.
- [ ] Pruebas en hardware real.

## 6. Hallazgos confirmados del motor real (2026-09-22, sin asumir)

- **Motor:** Gameloft propietario `TBFV/GloftGTHP.ML` (nada que ver con Unity/Cocos).
- **JNI 100% estática:** 27 exports `Java_*`, 0 `JNI_OnLoad`, 0 `RegisterNatives`
  (confirmado en strings del .so + disasm). `main.c` NO llama `JNI_OnLoad` (NULL-guarded).
- **GLES1 confirmado a nivel ELF:** NEEDED `libGLESv1_CM.so` (además de la heurística
  glVertexPointer/glTexParameterx). `dynlib.c` ya mapea pipeline fijo + EGL.
- **Ciclo de vida (jadx, orden real):**
  `onCreate: nativeSetPhone(width)+nativeGetInfo(c4 strings)` →
  `onSurfaceCreated: nativeGetJNIEnv() → GLResLoader.nativeInit(0) → GLMediaPlayer.nativeInit(0) → GTRacing.nativeInit(demo?1:0) → GameRenderer.nativeInit(1)` →
  `onSurfaceChanged: nativeResize(w,h)` →
  `onDrawFrame loop @~30fps: nativePause/Resume(1) por flags, drenar nativeKeyDown/Up, nativeRender()`.
  Touch: `nativeOnSize(w,h)` + `nativeOnTouch(tipo,x,y,id)`; sensor: `nativeAccelerometer(x,y+1,z)` + `nativeSetOrientation(±1)`; foco: `nativeResumeInternal()/nativeOpenIGM()`.
- **Hermano Hyundai NO reutilizable:** paquete `ANMP.GloftGTHY`, ABI v7a, sin exports
  `Java_*` (RegisterNatives), ciclo `nativeOnCreate/nativeOnDrawFrame(GL10)` distinto.
  Se reusó solo el *patrón* de su `main.c`/`CMakeLists` (llamadas `(&jni,NULL)` NULL-guarded,
  `-Wl,--allow-multiple-definition`, `converter.c`), no su código de motor.
- **Audio dual:** Java `SoundPool`/`MediaPlayer` (rutas `/sdcard/gameloft/games/GTRacing/`)
  + OpenAL nativo (`alSourcePlay` x4, `alBufferData` x2). `dynlib.c` aún SIN mapeos OpenAL.
- **Assets:** pack offline ya en `ux0_data/gtracingmotoracademyhd/GTRacing/*.ba1`;
  `GLResLoader.getResourceBytes/Full/Length` son load-bearing (el .so NO usa AAssetManager:
  0 hits) -- sus stubs generados devuelven NULL/0 y habrá que implementarlos contra FIOS/APK.
- **Build host 2026-09-22:** `eboot.bin` + VPK compilan (Debug). Fixes aplicados:
  `generated_jni_stubs.c` en CMakeLists, `converter.c` en CMakeLists,
  `-Wl,--allow-multiple-definition` (reimpl/egl.c vs vitaGL nueva),
  `#include FalsoJNI_Logger.h` + redefinición `fjni_logv_dbg` con `##__VA_ARGS__` en stubs,
  `lib/falso_jni` clonado de upstream (submódulo vacío).
- **Diferidos al primer log real:** `nativeGetInfo` (necesita 4 jstrings, FalsoJNI sin
  NewStringUTF aún), `nativeSetPhone(960)` (fallback 480 de i9100), input/audio/assets.
- **Estructura de datos Vita (2026-09-22, `ux0_data/gtracingmotoracademyhd/`):**
  `GTRacing/` (625 archivos: .ba1 + AI.bin, *.bar, *.swf sueltos) ← `/sdcard/.../GTRacing/`;
  `saves/` ← `/data/data/<paquete>` (única otra ruta absoluta del .so);
  `res/` (19 MB) + `assets/` servidos por stubs GLResLoader file-backed;
  `logs/` = vita_logs_dir; `libGTRacing.so` idéntico al del APK (SO_PATH apunta ahí).
  `io.c` redirige ambos prefijos (tolera `//`); stubs `getResourceBytes/Full(String)` y
  `getResourceLength` leen archivos reales. Diferidos: `getRawResource(int)`,
  `getResourceFull(int)`, `getString(int)` (necesitan mapa resources.arsc).
- **Avance sin consola (2026-09-22, ver port_progress.md Fase 7):** cobertura de imports
  291/291 en `dynlib.c` (21 `__aeabi_*` de libgcc + `modff`/`inet_addr` + stub
  `getprotobyname`=NULL + `__dso_handle` dummy; tipos de reloc verificados con readelf);
  `align-check` limpio; `so-patch` sin SDKs nativos (URLs Java de ads/billing/GLive no
  corren en Vita; lista completa al final de este archivo); LiveArea válida;
  `source/input.c` (touch 1:1 960x544, dpad/CROSS/BACK/MENU, acelerómetro+orientación);
  `glres_data.inc` generado (10 raw + 118 strings) y los 3 stubs por int implementados.
- **Lectura directa del pseudo-C (2026-09-22, `out_ghidra.c` lín. 368150-369483):**
  `nativeGetJNIEnv` solo guarda mEnv; `nativeResize/SetPhone/Accelerometer` solo
  guardan globales (llamadas actuales correctas); `nativeRender→appUpdate`,
  `KeyDown/Up→appKeyPressed/Released`, `OnSize→SetScreenSize`, `OnTouch→appOnTouch`;
  `nativeInit(1)` = `m_bOGLLostContext` tras `appInit()` (igual que Java);
  `GTRacing_nativeInit(demo)` guarda `mIsDemo` + resuelve ~15 IDs (todos en tabla);
  `GLMediaPlayer_nativeInit` resuelve ~30 IDs (todos en tabla) y `GetTotalSounds=180`;
  `GLResLoader_nativeInit` resuelve SOLO las 3 variantes String (las file-backed);
  `nativeGetInfo` exige 4 jstrings válidos (se omite: solo telemetría);
  `appInit` fija `g_savePath`/`g_appPath` (coinciden con redirect io.c) y encadena
  `InitAppData→InitGL→Init3D→Init3DShaders→StringManager→InitGame` sin hazards;
  el .so NO importa EGL (presenta vía GLSurfaceView = nuestro `gl_swap`).
- **Auditoría de archivos + spec de stubs (2026-09-22, ver port_progress.md Fase 9):**
  196 nombres con extensión en el .so, 13 ausentes y todos inocuos (claves de texto
  en memoria, logs de debug sin callers, `gtracing.mp4` cuyo retorno se ignora,
  `igp_textinfo.bar`/`s_image.mp3` de billing/ads). `data.sav` ausente = defaults.
  Stubs corregidos a spec jadx: `detectPhoneLang=1`, `VGetChar=-1`,
  `isDemo=TRUE`, `Get_PhoneManufacturer=1`.

## 5. Herramientas

Este port se gestiona con **psvita-port-toolkit** (standalone, fuera de este repo). Desde el
toolkit: `Continuar con un port existente` → elegí esta carpeta (ya tiene `.psvita-toolkit.json`).

## Auto-detected lifecycle methods (psvita-toolkit)

Native methods whose name matches a well-known Android/GL app lifecycle hook --
these are the ones `main.c`/the loader most likely needs to call directly to
drive the game (there's no real Android `Activity`/`GLSurfaceView` calling them
for you).

- `com.gameloft.android.TBFV.GloftGTHP.ML.GLMediaPlayer.nativeInit(int)`
- `com.gameloft.android.TBFV.GloftGTHP.ML.GLResLoader.nativeInit(int)`
- `com.gameloft.android.TBFV.GloftGTHP.ML.GTRacing.nativeInit(int)`
- `com.gameloft.android.TBFV.GloftGTHP.ML.GameRenderer.nativeInit(int)`
- `com.gameloft.android.TBFV.GloftGTHP.ML.GameRenderer.nativeRender(void)`
- `com.gameloft.android.TBFV.GloftGTHP.ML.GameRenderer.nativeResize(int, int)`

## Auto-patch findings (psvita-toolkit)

Telemetry/IAP SDKs and hardcoded Android-only paths/URLs detected by a static
scan of the game's `.so` and the jadx-decompiled Java -- see
`docs/dev-notes/so_patcher.md` for why this stays a static, source-level scan
(no live in-memory binary patching), and why the generated stubs in
`telemetry_stubs.c` are a checklist to adapt, not a drop-in replacement.

### Hardcoded Android-only paths / network calls

- `http://ingameads.gameloft.com/redir/hdloading.php?game=#GAME#&country=#COUNTRY#&lg=#LANG#&ver=#IGP_VERSION#&device=#DEVICE#&f=#FIRMWARE#&udid=#ID#&g_ver=#VERSION#` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLMediaPlayer.java:35`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `/sdcard/gameloft/games/GTRacing//` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLMediaPlayer.java:206`) -- suggested Vita-side replacement: `/ux0:/data/gtracingmotoracademyhd` (this project's configured vita_game_data_dir)
- `/sdcard/gameloft/games/GTRacing/` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLMediaPlayer.java:226`) -- suggested Vita-side replacement: `/ux0:/data/gtracingmotoracademyhd` (this project's configured vita_game_data_dir)
- `/sdcard/gameloft/games/GTRacing/` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLMediaPlayer.java:239`) -- suggested Vita-side replacement: `/ux0:/data/gtracingmotoracademyhd` (this project's configured vita_game_data_dir)
- `/sdcard/gameloft/games/GTRacing//` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLResLoader.java:129`) -- suggested Vita-side replacement: `/ux0:/data/gtracingmotoracademyhd` (this project's configured vita_game_data_dir)
- `/sdcard/gameloft/games/GTRacing//` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLResLoader.java:179`) -- suggested Vita-side replacement: `/ux0:/data/gtracingmotoracademyhd` (this project's configured vita_game_data_dir)
- `https://secure.gameloft.com/tryandbuy/notifications/` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLUtils/f.java:14`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://confirmation.gameloft.com/android/3g_carrier.php` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLUtils/f.java:15`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `https://secure.gameloft.com/wap/paybox/` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLUtils/f.java:101`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://confirmation.gameloft.com/sms/unlock_profiles.php` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLUtils/f.java:109`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `https://secure.gameloft.com/tryandbuy/notifications/` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLUtils/f.java:182`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `https://secure.gameloft.com/tryandbuy/notifications/` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLUtils/f.java:204`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `https://secure.gameloft.com/tryandbuy/notifications/` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLUtils/f.java:267`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://confirmation.gameloft.com/android/3g_carrier.php` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLUtils/f.java:309`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://livewebapp.gameloft.com/glive/friends/evaluate/fid/FRIEND_ID/value/` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLiveMain.java:97`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://livewebapp.gameloft.com/glive/account` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLiveMain.java:98`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://livewebapp.gameloft.com/glive/index/add-trophy` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLiveMain.java:99`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://livewebapp.gameloft.com/glive/games/recommend-via-twitter/id/` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLiveMain.java:101`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://livewebapp.gameloft.com/glive/games/show-game/gid/GAMEID/fb_send/yes` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLiveMain.java:102`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `/sdcard/gameloft/games/GTRacing//androidTrophy.dat` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLiveMain.java:263`) -- suggested Vita-side replacement: `/ux0:/data/gtracingmotoracademyhd` (this project's configured vita_game_data_dir)
- `/sdcard/gameloft/games/GTRacing//androidTrophy.dat` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLiveMain.java:264`) -- suggested Vita-side replacement: `/ux0:/data/gtracingmotoracademyhd` (this project's configured vita_game_data_dir)
- `http://livewebapp.gameloft.com/glive/?lg=LANG&country=COUNTRY_DETECTED&d=DEVICE_ANDROID&f=FIRMWARE_ANDROID&udid=UDIDPHONE&GGI=GGIGAME&device_token=DEV_TOKEN&username=USER_NAME&pass=PASSWORD&remember_me=AUTOLOGIN&type=ANDROID&height=SCREEN_HEIGHT` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLiveMain.java:341`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://livewebapp.gameloft.com/glive/?lg=LANG&country=COUNTRY_DETECTED&d=DEVICE_ANDROID&f=FIRMWARE_ANDROID&udid=UDIDPHONE&GGI=GGIGAME&device_token=DEV_TOKEN&username=USER_NAME&pass=PASSWORD&remember_me=AUTOLOGIN&type=ANDROID&height=SCREEN_HEIGHT&fb=1` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/GLiveMain.java:343`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://ingameads.gameloft.com/redir/?from=GTHP&op=SAMS&lg=EN` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/IGPMain.java:20`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://signal-back.com` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/IGPMain.java:22`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://livewebapp.gameloft.com/glive/friends` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/a.java:28`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://livewebapp.gameloft.com/glive/games/recommend-via-mail/id/` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/af.java:28`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://livewebapp.gameloft.com/glive/games/recommend-via-mail/id/` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/ag.java:28`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://ingameads.gameloft.com/redir/?from` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/al.java:50`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- `http://livewebapp.gameloft.com/glive/challenges/create-challenge?challenged_user_id=` (java, `decompiled/apk_jadx/sources/com/gameloft/android/TBFV/GloftGTHP/ML/am.java:20`) -- network call -- likely needs FalsoJNI-side interception or removal, not a path rewrite (case-by-case, read the call site)
- ... and 152 more (re-run the scan to see the full list)
