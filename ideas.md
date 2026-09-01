# 🎵 GrooveSpeed Vita — Future Roadmap & Improvement Ideas

This document outlines feature ideas, DSP algorithms, hardware optimizations, and UI enhancements proposed for **GrooveSpeed Vita**.

---

## 1. 🏆 Audiophile Grading & Diagnostic Intelligence

### 1.1 Automated Hi-Fi Quality Rating (NAB & DIN 45507 Standards)
Evaluate measured turntable performance against established international studio and broadcasting standards, assigning an instant quality grade badge:

- 🟢 **Grade A+ (Broadcast Reference / Direct Drive Master)**:
  - Wow & Flutter: `< 0.05%`
  - Pitch Deviation: `< 0.10%`
  - *Target equipment*: Technics SL-1200G/SP-10, VPI Direct, Brinkmann, Denon DP-80.
- 🔵 **Grade A (Audiophile Grade / High-End Belt)**:
  - Wow & Flutter: `< 0.08%`
  - Pitch Deviation: `< 0.25%`
  - *Target equipment*: Rega Planar 8/10, Pro-Ject X8, Linn Sondek LP12, ClearAudio.
- 🟡 **Grade B (Hi-Fi Consumer Standard)**:
  - Wow & Flutter: `< 0.15%`
  - Pitch Deviation: `< 0.50%`
  - *Target equipment*: Vintage consumer turntables (Dual, Pioneer, Technics belt series, Debut Carbon).
- 🔴 **Grade C (Service & Maintenance Recommended)**:
  - Wow & Flutter: `> 0.15%` or Pitch Deviation `> 0.50%`
  - *Indicates*: Stretched drive belt, dry central spindle bearing, motor pulley slip, or idler wheel flat spots.

### 1.2 Live Pitch Adjustment Guider (Tuning Assistant)
An interactive real-time tuning helper for turntables equipped with pitch control potentiometers or motor trim pots:
- Computes exact offset needed to reach precise target speed (e.g. `+0.42%`).
- Shows directional cue: **"Rotate Pitch Pot +0.4% Clockwise (Faster)"** or **"Rotate -0.8% Counter-Clockwise (Slower)"**.
- Visual calibration lock indicator with animated checkmark when speed is within \(\pm 0.05\%\).

---

## 2. 📊 Advanced Real-Time DSP & Spectral Analysis

### 2.1 Live FFT Motor & Bearing Rumble Spectrum View
Add a dedicated **0–100 Hz Real-Time FFT Spectrum Analyzer** tab to pinpoint mechanical vibration sources:
- **0.55 Hz / 0.75 Hz Peaks**: Platter / sub-platter eccentricity or off-center pressing simulation.
- **1.8 Hz – 4.0 Hz Peaks**: Belt rotational cycle and pulley resonance.
- **50 Hz / 60 Hz Peaks**: AC synchronous motor pole magnetic hum / mains transformer mechanical interference.
- **100 Hz / 120 Hz Peaks**: Full-wave rectified power supply vibration.

### 2.2 Thermal Drift & Warm-Up Burn-In Test (Long-Duration Mode)
Turntable motors, belts, and bearing lubricants change viscosity and tension as they warm up.
- Continuous 10–30 minute logging session.
- Real-time thermal drift graph showing speed curve from cold-start to stable operating temperature.
- Identifies whether a turntable needs a 15-minute warm-up before critical listening.

### 2.3 Variable & Custom Target RPM Support
Support bespoke and vintage turntable speeds:
- **16 ⅔ RPM**: Spoken word and audiobooks.
- **72 RPM / 80 RPM**: Vintage early shellac pressings.
- **DJ Pitch Range**: Configurable \(\pm 8\%\), \(\pm 16\%\), and \(\pm 50\%\) target locks.

---

## 3. 🎮 PS Vita Native Hardware Features & Preservation

### 3.1 OLED Pure-Black Stealth Mode (Burn-In Prevention)
For PS Vita 1000 OLED consoles spinning on a turntable for extended periods:
- One-tap toggle that turns all UI background elements to true `#000000` (pixels turned completely off on OLED).
- Displays only an ultra-dim minimalist neon strobe ring or single numerical speed readout.
- Drastically reduces battery consumption and eliminates any burn-in risk.

### 3.2 Turntable Equipment Garage (Gear Profiles)
Save multiple named turntable profiles on memory card (`ux0:data/groovespeed/profiles.json`):
- Assign deck name (e.g., *"Technics SL-1200MK2"*, *"Rega Planar 3"*, *"Dual 1219"*).
- Track belt installation date, bearing oil type, and cartridge/stylus notes.
- Automatically stamp every saved session and CSV export with the active turntable profile.

### 3.3 Authentic Vinyl Certification Card Image Export
- Render a high-resolution PNG certificate to `ux0:picture/GrooveSpeed/`.
- Formatted like a vintage Japanese turntable factory test certificate with:
  - Turntable model name & date.
  - Average RPM, DIN Wow & Flutter curve, Pitch Deviation %, Platter Wobble, and Motor Rumble.
  - Polar deviation plot thumbnail.

---

## 4. 🎨 Audiophile Console Color Themes

Add a Theme Switcher modal with iconic Hi-Fi color palettes:
1. 🩵 **Groove Teal** *(Default)*: Modern cyberpunk cyan (`#00E5FF`) on dark slate (`#0B0E12`).
2. 🥇 **Technics Gold**: Classic SL-1200 champagne gold (`#FFD700`) & amber with brushed aluminum highlights.
3. 💚 **McIntosh Emerald**: Iconic glowing tube emerald green (`#00E676`) on deep obsidian blue.
4. ❤️ **Audiophile Crimson**: Low-light red (`#FF3D00`) designed for darkroom listening sessions.
5. 🍾 **Marantz Champagne**: Warm vintage silver-gold (`#E6D5AC`) with soft blue accents.

---

## 5. 🔊 Audio Calibration Tone Generator

- Synthesize accurate calibration reference audio tones through the PS Vita headphone jack:
  - **1,000 Hz / 3,150 Hz Sine Waves**: Standard test frequencies for DIN 45507 test records.
  - **Screen Strobe Light**: Flash the screen backlight at precise 50Hz / 60Hz intervals to serve as an optical strobe light for physical turntable strobe discs.

---

## 📋 Implementation Priority Matrix

| Feature | Complexity | Value | Target Area |
|---|---|---|---|
| **Hi-Fi Quality Rating (NAB/DIN Grades)** | Low | High | DSP / UI |
| **Live Pitch Adjustment Guider** | Low | High | UI / Diagnostics |
| **OLED Pure-Black Stealth Mode** | Low | High | Hardware Preservation |
| **Color Theme Switcher** | Medium | High | Aesthetics |
| **Turntable Equipment Garage** | Medium | High | Data & Storage |
| **Live FFT Rumble Spectrum** | Medium-High | Very High | Advanced DSP |
| **Thermal Drift Long-Run Mode** | Medium | Medium | Logging |
| **Audio Calibration Tone Generator** | Medium | Medium | Audio Engine |
