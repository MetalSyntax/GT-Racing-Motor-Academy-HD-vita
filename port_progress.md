# Registro de Progreso — GT Racing Motor Academy HD (PS Vita)

## Fase 1: Configuración y Preparación (Completada — 2026-09-08)
- Repo creado desde soloader-boilerplate, `.gitignore` anti-DMCA.
- APK `GTRacing_3.2.8.apk` copiado y extraído.
- ABI detectada: armeabi (elegida: armeabi).
- GLES detectado: AndroidManifest.xml no declara glEsVersion -- usar heurística (GLES1 (pipeline fijo: glVertexPointer/glClearColorx/glTexParameterx))

## Fase 2: Decompilación (Completada — 2026-09-08)
- jadx: corrido, resultados en decompiled/apk_jadx/.
- Ghidra (.so): corrido para cada .so.

## Fase 3: Análisis del Motor Real (Completada — 2026-09-22)
- Motor Gameloft propietario `TBFV/GloftGTHP.ML`, 1 .so (4.3 MB, armeabi).
- JNI 100% estática: 27 exports `Java_*`, 0 `JNI_OnLoad`, 0 `RegisterNatives`
  (strings del .so + disasm). Sin registro dinámico.
- GLES1 confirmado a nivel ELF (NEEDED `libGLESv1_CM.so`), no solo heurística.
- Ciclo de vida real extraído de jadx (`GTRacing.java` 533 lín, `GameRenderer.java`,
  `GameGLSurfaceView.java`, `GLMediaPlayer.java`, `GLResLoader.java`): ver
  PORTING_PLAN.md sección 6.
- Hermano `GT-Racing-Hyundai-Edition-vita` evaluado y DESCARTADO para reuso de código
  (paquete `ANMP.GloftGTHY`, v7a, sin `Java_*`, ciclo distinto). Reusado solo el patrón
  de bootstrap de su `main.c`/`CMakeLists` (Asphalt-6, misma familia Gameloft).
- Middleware (`jni-analyze` + strings): OpenAL nativo (`alSourcePlay` x4), libpng,
  libjpeg. Sin OpenSL/Unity. Sin `AAssetManager` (0 hits) → assets vía callbacks Java.
- Rutas hardcodeadas `/sdcard/gameloft/games/GTRacing/` (21 hits) → redirigir a
  `ux0:data/`. Pack de datos offline ya presente en `ux0_data/.../GTRacing/*.ba1`.

## Fase 4: Bootstrap del Loader (Completada en host — 2026-09-22)
- `source/main.c` reescrito con el ciclo real: `JNI_OnLoad` NULL-guarded (ausente),
  `nativeSetPhone(960)` → `nativeGetJNIEnv()` → `GLResLoader.nativeInit(0)` →
  `GLMediaPlayer.nativeInit(0)` → `GTRacing.nativeInit(1, demo)` →
  `GameRenderer.nativeInit(1)` → `nativeResize(960,544)` → loop `nativeRender()+gl_swap`
  con throttle 33 ms (~30 fps, como el `Thread.sleep` de Java).
- `source/java.c`: 102 candidatos de `generated_jni_table.h` cableados en las tablas
  reales (19 int + 6 boolean + 65 void + 12 object). Se mantienen `WINDOW_SERVICE`/`SDK_INT`.
- `CMakeLists.txt`: añadido `source/generated_jni_stubs.c` + `lib/falso_jni/converter.c`
  + `-Wl,--allow-multiple-definition` (reimpl/egl.c vs vitaGL actual).
- `generated_jni_stubs.c`: include `FalsoJNI_Logger.h` + redefinición `fjni_logv_dbg`
  con `##__VA_ARGS__` (los stubs la llaman con 1 solo arg).
- `lib/falso_jni` estaba vacío (submódulo sin inicializar, repo sin commits):
  clonado de upstream `v-atamanenko/FalsoJNI` por HTTPS.

## Fase 5: Primer Build (OK en host — 2026-09-22)
- `psvita-toolkit doctor`: VITASDK + toolchain + cmake/make/jadx OK
  (solo WARN menores: sin ninja, sin psp2cgc, sin PVRTexToolCLI).
- `psvita-toolkit build --preset debug`: **BUILD OK** →
  `build/eboot.bin` (583 KB) + `build/gtracingmotoracademyhd.vpk` (665 KB).
- `clean-junk`: 70 archivos `._*` eliminados (disco exFAT).
- 3 fallos de link diagnosticados y corregidos en la misma sesión (ver Fase 4).

## Fase 6: Estructura de datos Vita (Completada en host — 2026-09-22)
- `SO_PATH`: `main.so` → `libGTRacing.so` (así se llama en `ux0_data/`; verificado con
  strings en el .elf final).
- `source/reimpl/io.c`: `remap_android_path()` — `/sdcard/gameloft/games/GTRacing/`
  (21 hits en el .so: AI.bin, *.bar, *.swf, License.bin; existen sueltos en `GTRacing/`)
  → `DATA_PATH"GTRacing/"`; `/data/data/<paquete>` → `DATA_PATH"saves/"`.
  Aplicado en `fopen/open/stat/opendir`. Tolera `//` (Java concatena con doble slash).
  `open` conserva el `mode` de `O_CREAT` (saves).
- `generated_jni_stubs.c`: `getResourceBytes(String,off,len)`, `getResourceFull(String)`
  y `getResourceLength` ahora leen archivos reales (`DATA_PATH` tal cual → `assets/` →
  `res/`, más rama sdcard→`GTRacing/`), devuelven `NewByteArray` (patrón Asphalt-6).
  `getRawResource(int)`/`getResourceFull(int)`/`getString` quedan NULL (requieren
  resources.arsc) — confirmar con log si el .so los llama.
- Rebuild Debug: **BUILD OK** (`eboot.bin` 585 KB, VPK 668 KB). `clean-junk` (8 `._*`).

## Fase 7: Avance sin consola (Completada en host — 2026-09-22)
- **Imports 291/291:** auditoría `nm -D --undefined-only` vs `dynlib.c` → 25 faltantes,
  todos mapeados: 21 `__aeabi_*` float (existen en libgcc de Vita, patrón `extern void*`
  del scaffold), `modff` (newlib), `inet_addr` (`arpa/inet.h`), stub
  `getprotobyname`=NULL (no existe en Vita; solo telemetría), dummy `__dso_handle`.
  Tipos verificados con `readelf -r` (todo JUMP_SLOT salvo `__dso_handle`=GLOB_DAT).
  Sin esto, primer arranque = `fatal_error Unknown symbol` vía `plt0_stub`.
- **Sin OpenAL nativo:** 0 imports `al_*/alc_*` (los hits eran strings; audio = Java
  SoundPool/MediaPlayer por stubs JNI). Sin puente de audio pendiente a nivel import.
- **`align-check`:** limpio (sin ldrd/strd/vld1 ni struct packing).
- **`so-patch`:** sin SDKs de telemetría/IAP en el .so; 182 URLs/paths son Java
  (ads, billing SMS, GLive, IGP) que no se ejecutan en Vita. Volcado al final del PLAN.
- **LiveArea:** `livearea --validate` OK (4 PNG indexados + template).
- **Input (`source/input.c/h`, cableado en `main.c`+CMake):** touch frontal 1:1 a
  960x544 (`nativeOnTouch` 1/2/0 con slot=id, `nativeOnSize` una vez), dpad→19-22,
  CROSS→23, CIRCLE→BACK(4), START→MENU(82) por flanco, acelerómetro (`+1.0f` en Y
  como jadx) + `nativeSetOrientation(±1)` con umbral ±8.0. `sceTouch` sampling ON
  (viene apagado por default). Mapeo de botones inicial, calibrar con log real.
- **Stubs por int (`glres_data.inc` generado de R.java+public.xml+strings.xml):**
  tabla 10 raw (`res/raw/...`) + 118 strings EN; `getRawResource(int)`,
  `getResourceFull(int)`, `getString(int)` implementados con archivos reales.
  Quedan 0 stubs JNI devolviendo NULL/0 por defecto sin implementar.
- Rebuild Debug tras 2 errores (miembro `accelerometer` de SceMotionSensorState;
  `extern void*` + `arpa/inet.h` para nuevos símbolos): **BUILD OK**.
- 2 fixes de código en la misma sesión (ver arriba).

## Fase 8: Lectura del pseudo-C (Completada — 2026-09-22, sin consola)
- Fuente: `decompiled/libGTRacing_armeabi/ghidra/out_ghidra.c` lín. 368150-369483
  (los 27 `Java_*` + `appInit`), leído por ventanas acotadas.
- **Firmas/args verificados:** `nativeGetJNIEnv(env)`=guarda mEnv;
  `nativeResize(w,h)`, `nativeSetPhone(w)`, `nativeAccelerometer(x,y,z)`=stores;
  `nativeOnTouch(t,x,y,id)`→`appOnTouch`; `KeyDown/Up`→`appKeyPressed/Released`;
  `nativeRender()`→`appUpdate`; `nativeInit(1)`=`m_bOGLLostContext` (igual que Java).
- **`GTRacing_nativeInit(demo)`** guarda `mIsDemo` + resuelve ~15 IDs de billing/app
  (todos existen en `java.c`; `nativeIsDemo` descarta el retorno → el modo demo lo
  decide solo el arg, 1 en primer arranque).
- **`GLMediaPlayer_nativeInit`** resuelve ~30 IDs de audio (todos en tabla; audio
  mudo pero sin crash) y `nativeGetTotalSounds=180` / `OfSameInstance=2`.
  Usa `__aeabi_f2d` en el camino de play → valida el mapeo aeabi de Fase 7.
- **`GLResLoader_nativeInit` resuelve SOLO variantes String** → confirma que las 3
  file-backed son las load-bearing; las `(int)` casi no las pide el nativo.
- **BUG REAL encontrado y corregido sin consola:** FalsoJNI resuelve por nombre y
  devuelve el PRIMER match; `getResourceFull` duplicado devolvía el stub `(I)` ante
  el pedido `(String)[B` → assets rotos. Reordenado (String primero) con comentario.
  `init`×3/`loadSound`×2 verificados inocuos (el nativo nunca pide esas firmas).
- **`nativeGetInfo` exige 4 jstrings** (`GetStringUTFChars`+`strdup`+`get_info`):
  se mantiene omitido (telemetría); implementarlo = 4 `NewStringUTF`.
- **`appInit` confirma paths:** `g_savePath`=/data/data (→saves/),
  `g_appPath`=/sdcard/.../GTRacing/ (→GTRacing/), y `GetSaveFilePath_GLX` genera
  `GTRacing//file` (doble slash → el colapso en `io.c` era necesario).
  Secuencia sin hazards: `InitAppData→InitGL→Init3D→Init3DShaders→InitGame`.
- **Sin EGL importado** (0 hits en undefined): presenta vía Java → nuestro `gl_swap`
  es el único presenter, sin riesgo de doble-present.
- **Clase NULL inocua:** `GetStaticMethodID` ignora `clazz` (solo `<init>` la usa) →
  nuestras llamadas `(&jni, NULL)` son seguras.
- Rebuild Debug: **BUILD OK**. `clean-junk` (8 `._*`).

## Fase 9: Auditoría de archivos + spec de stubs (Completada — 2026-09-22)
- 196 literales con extensión extraídos del .so (`python3`, regex) vs `ux0_data`:
  13 ausentes, todos inocuos tras leer su contexto en pseudo-C:
  `Rankings*.Texts.txt` = claves de StringManager en memoria (no archivos);
  `DebugOutput.txt`/`asphalt3.dbg`/`log_recv-send.txt` = logs debug SIN callers;
  `gtracing.mp4` → `nativeLoadMovie` cuyo retorno `GS_TrailerMovie::Create` ignora
  (nuestro stub=0, sin crash); `igp_textinfo.bar`/`s_image.mp3` = billing/ads.
- `LoadSettingData`: `data.sav` ausente → defaults, sigue (vía `saves/` si existe).
- `SoundManager::allocBuffers` = solo memoria. `InitAppData` usa `__aeabi_i2f/fdiv`
  (valida Fase 7) y `GetPhoneLanguage` → `detectPhoneLang`.
- 4 stubs a spec jadx: `detectPhoneLang` 0→1 (rango real 1-7, índice de idioma),
  `VGetChar` 0→-1 (string vacío), `isDemo` FALSE→TRUE (k==0, coherente con
  `nativeInit(1)`), `Get_PhoneManufacturer` ×2 0→1 (genérico; retornos descartados).
  `isSoundLoaded=0`/`isWifiAlive=0`/`IsDemo=0`/`unlockDemo=0` ya eran spec.
- Rebuild Debug: **BUILD OK**. `clean-junk` (8 `._*`).

## Fase 10: Optimización de I/O y Deduplicación de Logs (Completada — 2026-09-23)
- **Diagnóstico del log (`logs/log.log`):** `Game::InitGame()` llama `GetResourceFileList()`
  para escanear 6 extensiones de archivos (`.ba4`, `.ba5`, `.ba0`, `.ba1`, `.ba2`, `.ba3`).
  Con 629 archivos en `GTRacing/`, cada extensión realiza 2 pasadas completas de `opendir()`/`readdir()`.
  Esto producía más de 7,500 llamadas consecutivas a `readdir()`. En el logger original,
  cada llamada abría (`fopen`), escribía (`fputs`) y cerraba (`fclose`) `log.log` en el
  almacenamiento de la Vita, congelando la ejecución durante minutos y saturando el log.
- **Sistema de Logs Deduplicado e Incremental (`source/utils/logger.c` / `logger.h`):**
  - Numeración incremental cíclica de `[001]` a `[999]` por mensaje impreso.
  - Detección y colapso de mensajes consecutivos idénticos en memoria.
  - Cuando un log se repite más de una vez, se emite una única línea con el contador al final
    (ej: `readdir(0x823e26f8): 0x823e2700 (x628)`).
  - Vaciado inmediato (`log_flush`) en errores críticos (`LT_ERROR`, `LT_FATAL`), pasos
    principales de inicio de `main.c` y al final de cada frame en `gl_swap()`.
  - Fix de vulnerabilidad de formato: `sceClibPrintf("%s", buffer_b)` en vez de `sceClibPrintf(buffer_b)`.
- **Optimización de Memoria en `readdir_soloader` (`source/reimpl/io.c` / `_struct_converters.c`):**
  - Implementada `dirent_newlib_to_bionic_buf()` sin asignación en el heap para evitar
    `malloc` y `free` en cada una de las miles de entradas de directorio escaneadas.
- Rebuild Debug: **BUILD OK** (`eboot.bin` 585 KB, VPK 668 KB).

## Fase 11: Nombres de Archivo de Log Incrementales y Entrada al Main Loop (Completada — 2026-09-23)
- **Confirmación de ejecución exitosa en hardware (`logs/log.log`):**
  - El motor superó completamente `InitGame()`, el escaneo de `.ba4` y `.ba5`, la carga de recursos Flash SWF (`LoadingMenu.swf`, `Main_Menu_REAL.swf`), fuentes TrueType (`MicroFLF-Bold.ttf`), perfiles (`profiles.sav`), settings (`data.sav`), y completó exitosamente los estados de transición `Switch = 0` hasta `Switch = 13` ("Loadiing Finished").
  - El juego entró formalmente en el loop de render principal a 960x544 (`GameRenderer_nativeResize`, `entrando en main loop de render`).
- **Nombres de archivo de log incrementales (`log_001.log` ... `log_999.log`):**
  - En [`source/utils/logger.c`](file:///Volumes/Seagate/PSVITA%20Develop/GT-Racing-Motor-Academy-HD-vita/source/utils/logger.c): Implementada la función `_init_log_filename()`, que busca el primer número disponible en `ux0:data/gtracingmotoracademyhd/logs/log_%03u.log` (desde `log_001.log` hasta `log_999.log`).
  - Cada arranque de la aplicación conserva su archivo de sesión independiente (`log_001.log`, `log_002.log`, etc.), además de mantener un espejo en `log.log` para compatibilidad con scripts.
- **Creación preventiva de directorios (`source/utils/init.c`):**
  - Asegurada la creación en arranque de `ux0:data/gtracingmotoracademyhd/saves/` y `logs/` mediante `sceIoMkdir` para evitar fallos de guardado.
- Rebuild Debug: **BUILD OK** (`eboot.bin` 585 KB, VPK 668 KB).

## Fase 12: Diagnóstico y Corrección de Pantalla Negra (Completada en host — 2026-09-23)
- **Diagnóstico cruzado con ports hermanos Gameloft (`Asphalt-5-Vita`, `Asphalt-6-Vita`, `Dungeon-Hunter-2-vita`, `Shadow-Guardian-vita`):**
  - **4x MSAA y Starvation de VRAM (`SCE_GXM_MULTISAMPLE_4X` con 6MB):** El boilerplate invocaba `vglInitExtended(0, 960, 544, 6 * 1024 * 1024, SCE_GXM_MULTISAMPLE_4X)`. En la arquitectura GXM de PS Vita, 4x MSAA a 960x544 requiere >16.7 MB de VRAM solo para los buffers de muestras de color y profundidad. Con 6 MB de pool, vitaGL falla silenciosamente en alocar superficies de resolve o corrompe la salida, provocando una pantalla negra permanente. Todos los ports operativos usan `SCE_GXM_MULTISAMPLE_NONE` y un pool de 24 MB (`24 * 1024 * 1024`).
  - **Multi-Scene Render Target (`vglSetupDisplayRenderTarget(8)`):** GT Racing (`libGTRacing.so`) usa OpenGL ES 1.1 + `OES_framebuffer_object` y GameSWF (Flash) para renderizar UI y escenas a texturas FBO. vitaGL por defecto asigna 1 escena por frame; con múltiples cambios de render target por frame, `sceGxmBeginScene` se bloquea esperando la presentación del frame previo, congelando la imagen en negro. Se agregó `vglSetupDisplayRenderTarget(8)` antes de `vglInitExtended`.
  - **Conflicto de EGL y Múltiples Definiciones:** `source/reimpl/egl.c` proveía stubs ficticios que retornaban strings (`strdup("surface")`) y secuestraba `eglInitialize()`, forzando el flag `-Wl,--allow-multiple-definition`. Se removió `source/reimpl/egl.c` y se eliminó `-Wl,--allow-multiple-definition` de `CMakeLists.txt`, enlazando limpiamente la implementación nativa y completa de EGL que provee `libvitaGL.a`.
  - **Restauración de FBO en Swap & Heartbeat (`source/utils/glutil.c`):** En `gl_swap()`, se verifica si quedó algún FBO activo (`GL_FRAMEBUFFER_BINDING`) y se restaura el framebuffer 0 antes del swap. Se implementó un latido de render cada 60 frames (`[gl] heartbeat: %u frames presented`) y el sink `vgl_log()` para capturar advertencias internas de vitaGL.
  - **Directorio de caché de shaders (`DATA_PATH "shader_cache"`):** Configurado con `vglSetShaderCachePath()` y memoria interna cacheada (`vglUseCachedMem(GL_TRUE)`).
- Rebuild Debug: **BUILD OK** (`eboot.bin` 585 KB, VPK 668 KB).

## Fase 13: Corrección de Memoria del Heap y Recuperación de `vglInitExtended` (Completada en host — 2026-09-23)
- **Diagnóstico del log (`logs/log_002.log`):**
  - En la línea `[030] ! fatal vglInitExtended failed!`, vitaGL retornó `GL_FALSE`.
  - Causa directa: `_newlib_heap_size_user = 256 * 1024 * 1024` ocupaba prácticamente toda la RAM de usuario (`USER_RW`) del sistema. En vitaGL, el parámetro `ram_threshold` en `vglInitExtended()` define el margen de RAM que debe quedar **libre** para el sistema (`ram_pool = free_ram - ram_threshold`). Al solicitar `ram_threshold = 24 MB` cuando la RAM libre era menor a 24 MB, `ram_pool` caía a 0 y la inicialización de vitaGL abortaba por completo, dejando el contexto GXM sin inicializar.
- **Ajuste de Heap y Umbral con Fallback (`source/main.c` / `source/utils/glutil.c`):**
  - Reducido `_newlib_heap_size_user` a `192 * 1024 * 1024` (192 MB), liberando **64 MB** de memoria física para vitaGL y buffers de GPU.
  - Implementada telemetría de memoria pre-inicialización vía `sceKernelGetFreeMemorySize()` (`USER_RW`, `CDRAM`, `PHYCONT`).
  - Implementada cascada de fallback en `gl_init()`: prueba umbral de 12 MB, luego 6 MB y finalmente 1 MB para garantizar que vitaGL siempre inicialice con éxito.
  - Añadido guard `static unsigned char vgl_initialized` para prevenir reinicializaciones redundantes.
- **Envolturas Diagnósticas de Dibujo (`source/utils/glutil.c` / `glutil.h` / `dynlib.c`):**
  - Interceptadas `glViewport`, `glClearColor`, `glClear`, `glDrawArrays` y `glDrawElements` hacia wrappers que loguean sus primeros llamados con parámetros reales para verificar las llamadas de dibujo emitidas por el motor.
- Rebuild Debug: **BUILD OK** (`eboot.bin` 586 KB, VPK 668 KB).

## Fase 14: Vendorización de VitaGL (de Asphalt 5), ABI softfp, DRAW_SPEEDHACK=2 y Reimplementación de EGL (Completada en host — 2026-09-23)
- **Diagnóstico del log (`logs/log_003.log`):**
  - El motor emitía llamadas de dibujo reales: `glViewport(0, 0, 800, 480)`, `glViewport(0, 0, 960, 544)`, `glClearColor(1.0, 1.0, 1.0, 1.0)`, `glClear(0x4100)`, `glDrawArrays(GL_TRIANGLES, first=0, count=6)`.
  - Sin embargo, la pantalla seguía viéndose negra.
  - Causa raíz identificada:
    1. **Discrepancia de ABI de punto flotante (`softfp` vs `hardfp`):** El SDK precompilado de vitasdk incluye un `libvitaGL.a` compilado para VFP (`hardfp`), donde los floats (`glClearColor`, `glViewport`, vértices, matrices) se pasan en registros `s0-s3`. El juego nativo Android (`libGTRacing.so`) y el cargador de Vita están compilados con `-mfloat-abi=softfp` (donde los floats se pasan en registros de propósito general `r0-r3`). Esto causaba que todas las coordenadas, colores y proyecciones se recibieran como 0.0f en vitaGL.
    2. **Falta de `DRAW_SPEEDHACK=2`:** VitaGL estándar copia todos los streams de atributos de vértices a un búfer circular temporal pequeño (256 KB) antes de cada draw. Los modelos de coches y mallas complejas desbordan o corrompen este pool, provocando fallos silenciosos de renderizado.
    3. **Semántica de retorno de `vglInitExtended()`:** Retorna `GL_FALSE` cuando inicializa exitosamente sin requerir resolución de fallback (`res_fallback`), por lo que `if (!res) l_fatal` era un falso positivo.
- **Acciones Realizadas:**
  - **Vendorización de VitaGL:** Se copió el árbol fuente completo de `lib/vitagl` probado en *Asphalt 5* a `lib/vitagl/`.
  - **Compilación desde código fuente en CMake:** Integrado custom target `vitaGL_build` en `CMakeLists.txt` con los flags exactos:
    `SOFTFP_ABI=1 NO_DEBUG=1 HAVE_SHADER_CACHE=1 NO_SPLASHSCREEN=1 DRAW_SPEEDHACK=2`.
  - **Reincorporación de `source/reimpl/egl.c`:** Añadidas las funciones nativas faltantes sincronizadas con Asphalt 5 (`eglBindAPI`, `eglGetDisplay`, `eglGetError`, `eglGetProcAddress`, `eglSwapBuffers`, `eglSwapInterval`, `eglQueryAPI`, `eglGetSystemTimeFrequencyNV`, `eglGetSystemTimeNV`), donde `eglSwapBuffers` delega a `gl_swap()`.
  - **Multi-Scene Render Target con `vglSetupRenderTargetScenesNum(8, 8)`:** Configurados tanto el display principal como los FBOs a 8 escenas por frame.
  - **Corrección de chequeo en `gl_init()`:** Ahora evalúa correctamente el resultado de `vglInitExtended(0, 960, 544, 12 * 1024 * 1024, SCE_GXM_MULTISAMPLE_NONE)`.
- Rebuild Debug: **BUILD OK** (`eboot.bin` 590 KB, VPK 673 KB).

## Fase 15: Audio Nativo, Mapeo Completo de Controles, Escalado a Pantalla Completa (960x544) y Overclock (Completada — 2026-09-23)
- **Confirmación Visual y Rendimiento en Hardware (`logs/log_004.log` + Screenshot):**
  - La captura de pantalla en hardware confirmó que el motor 3D, pistas, coches, minimapa, tacómetro y texturas renderizan a la perfección a 30 FPS mediante el VitaGL vendorizado en softfp.
- **Escalado a Pantalla Completa (960x544) en Viewport y Scissor (`source/utils/glutil.c` / `glutil.h`):**
  - El juego inicializaba la escena 3D / HUD a 800x480 con franjas vacías. Se interceptaron `glViewport` y `glScissor` para que cuando rendericen al framebuffer principal (`bound_fbo == 0`) y la resolución solicitada sea 800x480, se reescale automáticamente al tamaño nativo de PS Vita (960x544). Las texturas/FBO internas conservan sus dimensiones exactas.
- **Overclock y Optimizaciones de Rendimiento (`source/utils/init.c`):**
  - Ajustado el reloj de la CPU a 500 MHz (`scePowerSetArmClockFrequency(500)`) para asegurar una tasa de cuadros estable sin ralentizaciones.
  - Optimizado el logger en `source/main.c`: vaciado a disco (`log_flush()`) cada 60 frames en vez de cada frame individual.
- **Optimización de Carga e I/O (`source/reimpl/io.c`):**
  - Removido el logging repetitivo en llamadas de alta frecuencia (`fopen`, `fclose`, `opendir`, `readdir`), eliminando miles de escrituras innecesarias a disco que ralentizaban los tiempos de carga de assets.
- **Sistema de Audio Nativo (`source/audio.h` / `source/audio.cpp`):**
  - Portado e integrado el motor de audio de *Asphalt 5* utilizando `minimp3`, decodificación OGG con `stb_vorbis` y mezcla multicanal con salida directa vía `sceAudioOut`.
  - Mapeadas las 164 pistas y efectos de sonido extraídas de `ac.java` (`s_sound_names`) ubicadas en `ux0:data/gtracingmotoracademyhd/GTRacing/*.ogg`.
  - Conectados todos los métodos JNI de `GLMediaPlayer` en `source/java.c` (`loadSound`, `playSound`, `stopSound`, `pauseSound`, `resumeSound`, `setVolume`, `setPitch`, `isMediaPlaying`, `initSoundPoolArray`, etc.).
- **Mapeo Completo de Controles e Inhabilitación de Acelerómetro (`source/patch.c` / `source/input.c`):**
  - **Patch en `CGameSettings::ResetControls()` (`source/patch.c`):** Parcheado el offset `0xcdf62` de `01 22` (`movs r2, #1`) a `02 22` (`movs r2, #2`) para establecer como predeterminado el modo de controles táctiles en pantalla en lugar de acelerómetro.
  - **Inhabilitación del sensor físico de movimiento:** Se anuló el muestreo del acelerómetro físico para evitar giros indeseados por inclinación de la consola.
  - **Mapeo físico estilo consola:**
    - **Freno:** Gatillo L, botón Cuadrado, D-Pad Abajo (emite toque en slot 6: `x=95, y=440`).
    - **Acelerador:** Gatillo R, botón Cruz, D-Pad Arriba (emite toque en slot 7: `x=930, y=440`).
    - **Dirección (Izquierda / Derecha):** D-Pad Izquierda/Derecha y stick analógico izquierdo (emite toques en slot 5: `x=180` / `x=780, y=380` y aceleración sintética `s_accel`).
    - **Pausa:** Botón START (toque en slot 4: `x=35, y=35` + `KEY_MENU`).
    - Se conservó el soporte multitouch en pantalla frontal (slots 0 a 3) y navegación en menús con cruceta/botones.
- Rebuild: **BUILD OK** (`eboot.bin` 668 KB, VPK 751 KB).

## Fase 16: Corrección Integral de Controles, Guardado de Esquemas, Ocultación de Botones HUD y Giro Proporcional (Completada — 2026-09-23)
- **Diagnóstico del log y decompilado (`logs/log_005.log` / `out_ghidra.c`):**
  - **Rechazo de esquemas de control en `ControlsMenu::IsComboValid()`:** El menú de controles validaba combinaciones específicas de dirección, acelerador y marchas mediante `IsComboValid()`. Cuando el usuario seleccionaba botones laterales o configuraciones no previstas por Gameloft, `IsComboValid()` retornaba 0, forzando a `CGameSettings::ResetControls()` a revertir la configuración al valor por defecto y anulando el guardado del perfil en `Profile.0.gts`.
  - **Fallo del giro sintético en modo acelerómetro:** En Android landscape, el giro del vehículo se calcula mediante `atan2(sqrt(X^2 + Z^2), Y)` en `CarControlHuman::UpdateSteeringAccelerometer()`. El ángulo de inclinación lateral se mide en el eje **Y** (con rango +/- 8.5 m/s²), mientras que el eje X sostiene la gravedad (~9.8 m/s²). El código previo enviaba la señal de giro en el eje X y dejaba Y en 1.0f constante, resultando en un giro nulo permanente.
  - **Límite de 2 punteros multitouch:** `libGTRacing.so` solo soporta punteros táctiles 0 y 1 (`pointerId < 2`). Los toques sintéticos con slots > 1 eran descartados silenciosamente por el motor.
  - **Botón Atrás en menús:** `appKeyPressed` es un stub vacío en Android nativo; el motor solo detecta toques físicos en el botón Back de la interfaz Flash en `(x=35, y=35)`.
- **Acciones Realizadas:**
  - **Patch `ControlsMenu::IsComboValid()` (`source/patch.c`):** Parcheado el offset `0x138f2c` con `01 20 70 47` (`movs r0, #1; bx lr`). Cualquier combinación seleccionada por el usuario es considerada válida, se preserva y se guarda inmediatamente en `Profile.0.gts` con `ProfileManager::SaveActiveProfile()`.
  - **Patch de Valores por Defecto en `CGameSettings::ResetControls()` (`source/patch.c`):** Parcheado el offset `0xcdf68` para que la dirección por defecto sea `0` (SideButtons / Botones físicos) en vez de `2` (Acelerómetro), y acelerador en `1` (`0xcdf62`: PedalsFar).
  - **Giro Dual (GamePad Directo + Acelerómetro Proporcional) (`source/input.c`):**
    - En modo SideButtons/Teclado: Se resuelven los símbolos `GamePadManager::GetGamePad` (`_ZN14GamePadManager10GetGamePadEi`), `GamePad::KeyboardKeyPressed` (`_ZN7GamePad18KeyboardKeyPressedEi`) y `GamePad::KeyboardKeyReleased` (`_ZN7GamePad19KeyboardKeyReleasedEi`), inyectando directamente las teclas `1` (Izquierda), `2` (Derecha), `4` (Acelerador) y `8` (Freno) al motor de física del vehículo.
    - En modo Acelerómetro: Se envía el eje Y proporcional (`synth_steer * 8.5f`) y gravedad horizontal en X (`9.8f`), asegurando un giro suave y preciso tanto con el stick analógico izquierdo como con la cruceta D-Pad.
  - **Ocultación Dinámica de Botones Táctiles en Pantalla (HUD) con Botón Triángulo (`source/input.c`):**
    - Resuelto `RenderFX::SetVisible` (`_ZN8RenderFX10SetVisibleEPKcb`) sobre el HUD en `g_pMainGameClass + 0x1a70`.
    - Los botones táctiles en pantalla (`HUD.Direction`, `HUD.AccelBrake`, `HUD.RenderWheel`, `HUD.Gears`) ahora inician **ocultos por defecto** para una interfaz de consola limpia.
    - Al presionar el botón **Triángulo** (`SCE_CTRL_TRIANGLE`), se alterna la visibilidad de los botones en pantalla cuando el usuario lo desee.
  - **Navegación y Guardado Rápido con Botón Círculo (`source/input.c`):**
    - Al presionar **Círculo** en menús u opciones de control, se emite un toque sintético en el botón de retroceso (`x=35, y=35`), guardando la configuración y regresando de inmediato al menú anterior.
- Rebuild: **BUILD OK** (`eboot.bin` 678 KB, VPK 955 KB).

## Fase 17: Reproducción de Video Intro con Decodificación Software (FFmpeg / ISO-BMFF) (Completada — 2026-09-23)
- **Diagnóstico del motor y assets originales (`out_ghidra.c:48771` / `apk_jadx`):**
  - Al iniciar, `GS_TrailerMovie::Create()` invoca `nativeLoadMovie("gtracing.mp4")`, el cual llama vía JNI estática al método `GLMediaPlayer.loadMovie(String)` (Method ID 19).
  - Los archivos de video originales de Gameloft (`gtracing.mp4` / `gtracing.m4v` en `ux0:data/gtracingmotoracademyhd/GTRacing/`) están codificados en formato MPEG-4 Part 2 / AAC. El hardware de PlayStation Vita (`SceAvPlayer`) solo decodifica H.264, por lo que requerían decodificación por software, igual que en *Asphalt 5*.
  - En Android original, `GLMediaPlayer.loadMovie` abría una Activity (`MyVideoView`), reproducía el video fullscreen y retornaba 1.
- **Implementación del reproductor de video (`source/video.h`, `source/video.cpp`, `source/java.c`):**
  - Se adaptó el reproductor de video multihilo de *Asphalt 5* con demuxer ISO-BMFF nativo, decodificación por software vía `avcodec`/`avutil`/`swresample`, conversión YUV420P a RGB565 optimizada con instrucciones ARM NEON, y salida directa de audio AAC con `sceAudioOut`.
  - **Resolución dinámica de archivos (`resolve_video_path`):** Busca de forma inteligente tanto `gtracing.mp4` como `Gtracing.mp4`, `gtracing.m4v` y variaciones en mayúsculas/minúsculas sin necesidad de convertir ni alterar los archivos originales.
  - **Renderizado directo en VitaGL:** El fotograma de video se proyecta a pantalla completa (960x544) preservando el aspect ratio original y el estado de la máquina de estados OpenGL.
  - **Control e interrupción:** Se puede omitir el video en cualquier momento presionando **Cruz**, **Círculo** o **START**.
  - **Conexión JNI (`source/java.c`):** Se reemplazó el stub 19 por `GLMediaPlayer_loadMovie()`, que extrae el nombre del video del objeto `jstring`, ejecuta `video_play(name)` de forma síncrona y retorna `1` para continuar suavemente hacia `GS_Copyright` y los menús.
  - **Inicialización del subsistema (`source/main.c` / `CMakeLists.txt`):** Se enlaza `libavformat.a`, `libavcodec.a`, `libavutil.a`, `libswresample.a`, `libmp3lame.a`, `SceNet_stub`, `SceNetCtl_stub`, `SceSysmodule_stub` y se llama a `video_init()` tras `gl_init()`.
- Rebuild: **BUILD OK** (`eboot.bin` y `gtracingmotoracademyhd.vpk` generados exitosamente sin advertencias ni errores).

## Fase 18 en adelante: Pruebas y Ajustes Finales
- [ ] Probar el nuevo VPK en consola física y verificar:
  - Reproducción fluida del video de introducción `gtracing.mp4` / `gtracing.m4v` con audio y posibilidad de saltarlo con Cruz/Círculo/Start.
  - Giro del coche con el stick analógico y cruceta en carrera.
  - Ocultación/muestra de botones HUD al presionar Triángulo.
  - Guardado y retroceso con botón Círculo en menús.
  - Respuesta de frenos (L / Cuadrado / Abajo) y acelerador (R / Cruz / Arriba).
