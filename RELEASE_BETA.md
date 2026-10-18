# GT Racing: Motor Academy HD — PS Vita Port (Beta v1.1) 🏎️🏁

## 🆕 What's new in Beta v1.1

* **Much better performance:** Races and menus now run at a stable 25-30 FPS, up from ~16 FPS.
  * The game's soft-float math is now executed on the Vita's VFP unit.
  * The loader is now built with optimizations.
  * Log writes moved off the game thread.
  * The game thread is pinned to its own CPU core.
* **Smooth intro video:** The intro now plays smoothly; before it ran at about 5 FPS.
* **Audio starts immediately:** Race music and engine sounds no longer start ~20 seconds late. OGG files are now decoded progressively.
* **No more red cars in the distance:** Opponents at medium distance no longer appear with the default red texture before switching to their real livery.
* **UI fixes at 960x544:** The track minimap, flags and Garage 3D car are now positioned correctly.
* **Controls:** Accelerator and brake were swapped and are now fixed: R/Cross/Up accelerate, L/Square/Down brake.
* **Menu stutter:** Fixed the stutter when navigating menus.

**Known issue:** for the first few seconds after entering a menu or starting a race, the framerate can drop to ~20 FPS before settling at 30 FPS.

---

### ✨ Key Features in this Beta:

* **Native ARM Execution:** Runs `libGTRacing.so` directly on the Vita's Cortex-A9 CPU with native performance.
* **Full Physical Controls Integration:** Complete console-style input mapping:
  * **Analog Stick / D-Pad Left & Right:** Smooth steering.
  * **R Trigger / Cross (❌) / D-Pad Up:** Throttle / Accelerate.
  * **L Trigger / Square (🟥) / D-Pad Down:** Brake / Reverse.
  * **Triangle (▲):** Toggle on-screen touch HUD buttons on/off anytime.
  * **Start:** Pause / in-race menu.
  * **Circle (⭕):** Back button in menus with automatic profile save.
* **Widescreen Graphics via vitaGL:** Clean, hardware-accelerated presentation across the entire 960x544 PlayStation Vita OLED/LCD panel.
* **Native Audio Engine:** Multi-channel 48kHz audio mixer (`sceAudioOut`) decoding 164 OGG/WAV engine sounds, ambient effects, and background music tracks.
* **Fullscreen Intro Video Playback:** Multi-threaded software video decoding via FFmpeg / ISO-BMFF, decoded at half resolution and upscaled with synchronized audio, skippable with **Cross**, **Circle**, or **Start**.

---

### ⚠️ Important Notes & Known Issues:

* **Control Scheme Selection:** You **MUST select the second control option: Touch Controls (Controles Táctiles)** in the in-game options menu (**Options → Controls**). Physical buttons map directly to this scheme.
* **Framerate:** Stable 25-30 FPS. The first seconds after entering a menu or starting a race can dip to ~20 FPS.
* **Audio:** Audio is fully working, but minor pitch quirks or looping inconsistencies might occasionally occur on certain engine sounds.

---

### 📋 Requirements:

1. PS Vita / PS TV on CFW (**HENkaku** or **Enso**).
2. [**kubridge**](https://github.com/TheOfficialFloW/kubridge/releases) and [**FdFix**](https://github.com/TheOfficialFloW/FdFix/releases) installed in `ur0:tai/config.txt`.
3. [**libshacccg.suprx**](https://github.com/Rinnegatamante/ShaRKBR33D/releases/latest) in `ur0:data/`.
4. Original **GT Racing: Motor Academy HD v3.2.8** Android APK (`com.gameloft.android.TBFV.GloftGTHP.ML`) and its data cache.

---

### 📦 Installation Guide:

1. Install `gtracingmotoracademyhd.vpk` on your PS Vita using **VitaShell**.
2. Extract `lib/armeabi/libGTRacing.so` from your original `.apk` file.
3. Copy `libGTRacing.so` to `ux0:data/gtracingmotoracademyhd/`.
4. Copy the entire `GTRacing/` data folder (with audio, `.ba4`/`.ba5` archives, `gtracing.mp4`) into `ux0:data/gtracingmotoracademyhd/`.
5. Verify your file structure matches the layout below:

```text
ux0:data/gtracingmotoracademyhd/
├── libGTRacing.so             <- Native library from APK
├── GTRacing/                  <- Game data folder
│   ├── gtracing.mp4           <- Intro video
│   ├── *.ogg / *.wav          <- Audio files
│   ├── *.ba4, *.ba5, etc.     <- Game data archives
│   └── ...
└── logs/                      <- Incremental logs (generated at runtime)
```

6. Launch the game from LiveArea and enjoy realistic racing on your PS Vita!

---

### 👥 Credits & Special Thanks:
* **Gameloft** for creating GT Racing: Motor Academy HD.
* **TheFloW** for `so_util`, `kubridge`, and `FdFix`.
* **Rinnegatamante** for `vitaGL` and ongoing Vita support.
* **v-atamanenko** for `FalsoJNI`.
* The entire **PS Vita homebrew community**!
