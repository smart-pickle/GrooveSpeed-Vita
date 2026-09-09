# 🎵 GrooveSpeed Vita

[![Platform](https://img.shields.io/badge/Platform-PS%20Vita%20%7C%20PSTV-blue.svg)](https://vitasdk.org/)
[![Version](https://img.shields.io/badge/Version-01.00-teal.svg)](https://github.com/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Website](https://img.shields.io/badge/Website-smartpickle.me%2Fgroovespeed-00E5FF.svg)](https://smartpickle.me/groovespeed)

**GrooveSpeed Vita** is a standalone, precision turntable speed and flutter diagnostic tool for vinyl enthusiasts and audiophiles, built natively for the PlayStation Vita (PCH-1000 OLED, PCH-2000 Slim, and PlayStation TV (please don't use it on Playstation TV).

Using the PS Vita's built-in 6-axis hardware gyroscope and accelerometer (`SceMotion`), GrooveSpeed delivers laboratory-grade turntable diagnostics without requiring dedicated tachometers, optical strobe discs, or external sensors.

🌐 **Official Website**: [smartpickle.me/groovespeed](https://smartpickle.me/groovespeed)

---

## 🚀 Key Features

* **Precision RPM Measurement**: Real-time angular speed tracking for **33 ⅓**, **45**, and **78 RPM** modes.
* **Wow & Flutter Analysis**:
  * **DIN 45507**: 2nd-order IIR bandpass weighting focused on human-perceptible 4.0 Hz cyclic variation.
  * **Unweighted Mode**: Raw peak-to-peak speed deviation analysis.
* **Pitch Deviation Percentage**: High-resolution percentage offset (\(\pm \%\)) to calibrate variable-pitch faders or motor trim pots.
* **Platter Wobble & Motor Rumble**:
  * **Wobble**: Vertical acceleration variance detecting warped platters or uneven mats.
  * **Rumble**: Radix-2 Cooley-Tukey FFT spectral analysis pinpointing motor pole vibration and bearing friction.
* **Live Strobe Disc**: Dynamic 60 FPS vector strobe ring with real-time speed lock tinting:
  * 🟢 **Emerald Green**: Reference studio lock (\(< \pm 0.15\%\))
  * 🩵 **Neon Cyan**: In-spec standard (\(< \pm 0.50\%\))
  * 🟡 **Amber Gold**: Minor speed drift
  * 🔴 **Coral Red**: Significant drift / slip
* **Studio Waveform Graph**: Real-time scrolling speed graph with translucent tolerance band (\(\pm 0.1\%\)) and leading-edge beacon dot.
* **360° Polar Deviation Plot**: Rotational spatial plot revealing once-per-revolution platter warp and eccentric pressings.
* **Guided 2-Step Calibration Wizard**: In-app calibration for zero-gyro bias and platter reference ratios (saved to `ux0:data/groovespeed/config.txt`).
* **Session Storage & Export**:
  * Saves diagnostic logs to `ux0:data/groovespeed/history.csv`.
  * Exports formatted PNG inspection report cards to `ux0:picture/GrooveSpeed/`.
* **Audiophile Modern Hi-Fi Interface**: Zero-scroll left telemetry deck, segmented LED VU level meters, and anti-aliased TrueType typography.
* **Full Dual Controls**: Seamless control via front touchscreen or physical gamepad buttons.

---

## 🎛️ How to Measure Your Turntable

1. **Mounting**:
   * Place a standard **45 RPM adapter** over your turntable's center spindle pin.
   * Place a small **non-slip rubber pad or silicone disc** on top of the 45 adapter to protect the Vita's rear touchpad.
   * Rest your PS Vita **face up** centered on top of the rubber pad over the spindle.
2. **Setup**:
   * Check the top navigation bar: verify the flatness badge shows **`[ LEVEL ]`** in green (if tilted, gently re-center the Vita).
   * Select your target speed: **`[ 33 ⅓ ]`**, **`[ 45 RPM ]`**, or **`[ 78 RPM ]`**.
   * Choose your test duration: **`[ 5s ]`**, **`[ 10s ]`**, **`[ 15s ]`**, or **`[ 30s ]`**.
3. **Run Diagnostic**:
   * Start your turntable motor.
   * Tap **`[ START MEASUREMENT ]`** (or let **Auto-Start** detect rotational velocity automatically).
   * After the run completes, inspect the live Wow/Flutter, Pitch Offset, Wobble, and Rumble metrics, or export your report card.

---

## 📥 Installation

### Via VitaHomebrew Browser / VitaDB
Search for **GrooveSpeed** in **VitaDB Downloader** or **Vita Homebrew Browser (VHBB)** and tap Install.

### Manual Installation (.vpk)
1. Download `GrooveSpeedVita.vpk` from the latest [GitHub Release](https://github.com/smart-pickle/GrooveSpeed-Vita/releases/).
2. Open **VitaShell** on your PS Vita.
3. Connect your Vita to your computer via USB or FTP (press `Select` in VitaShell).
4. Copy `GrooveSpeedVita.vpk` to `ux0:data/` (or any directory).
5. In VitaShell, navigate to `GrooveSpeedVita.vpk` and press **Cross (✕)** to install.
6. Launch **GrooveSpeed** from the LiveArea home screen.

---

## 🛠️ Building from Source

### Native VitaSDK Toolchain

With [VitaSDK](https://vitasdk.org/) installed:

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake
cmake --build build
```

This single command automatically compiles the executable, generates `eboot.bin` and `param.sfo`, packages `GrooveSpeedVita.vpk` with all LiveArea assets, and mirrors it directly to the repository root.


---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## 🌐 Community & Links

* **Official Website**: [smartpickle.me/groovespeed](https://smartpickle.me/groovespeed)
* **VitaDB**: [vitadb.rinnegatamante.it](https://vitadb.rinnegatamante.it/)
