# GT Racing: Motor Academy HD — PS Vita Port (Beta Release) 🏎️🏁

We are proud to present the first public **Beta Release** of **GT Racing: Motor Academy HD** for the PlayStation Vita and PlayStation TV!

Powered by a custom dynamic loader (*soloader*), FalsoJNI, and hardware-accelerated GLES 1.1 rendering through **vitaGL**, this port runs the original Android ARM native code directly on the Vita's processor.

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
* **Fullscreen Intro Video Playback:** Multi-threaded software video decoding via FFmpeg / ISO-BMFF in native 800x480 resolution with synchronized audio and no border flickering, skippable with **Cross**, **Circle**, or **Start**.

---

### ⚠️ Important Notes & Known Issues:

* **Control Scheme Selection:** You **MUST select the second control option: Touch Controls (Controles Táctiles)** in the in-game options menu (**Options → Controls**). Physical buttons map directly to this scheme.
* **Game Speed / Framerate:** On complex circuits or races with many competitor vehicles, framerate may dip slightly under heavy load.
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
