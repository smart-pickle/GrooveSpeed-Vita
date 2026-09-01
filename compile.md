# 🛠️ How to Compile GrooveSpeed Vita (`.vpk`)

This guide provides step-by-step instructions to compile and package **GrooveSpeed Vita** into an installable `.vpk` file for the PlayStation Vita and Vita3K emulator.

---

## 🚀 Method 1: Using Docker (Recommended)

Docker provides a reproducible, pre-configured VitaSDK toolchain without needing to install VitaSDK locally on your host OS. It works on **macOS (Apple Silicon & Intel)**, **Linux**, and **Windows (WSL2 / PowerShell)**.

### Prerequisites
- Install [Docker Desktop](https://www.docker.com/products/docker-desktop/).

### One-Command Build
Open a terminal in the root directory of the project and run:

```bash
docker run --platform linux/amd64 --rm -v "$(pwd):/src" -w /src vitasdk/vitasdk bash -c "
  export PATH=/usr/local/vitasdk/bin:\$PATH && \
  export VITASDK=/usr/local/vitasdk && \
  rm -rf build && mkdir build && cd build && \
  cmake -DCMAKE_C_COMPILER=/usr/local/vitasdk/bin/arm-vita-eabi-gcc \
        -DCMAKE_CXX_COMPILER=/usr/local/vitasdk/bin/arm-vita-eabi-g++ \
        -DCMAKE_TOOLCHAIN_FILE=/usr/local/vitasdk/share/vita.cmake .. && \
  make GrooveSpeedVita && \
  vita-elf-create GrooveSpeedVita GrooveSpeedVita.velf && \
  vita-make-fself GrooveSpeedVita.velf eboot.bin && \
  vita-mksfoex -s TITLE_ID=\"GROOVE001\" -s APP_VER=\"01.00\" \"GrooveSpeed\" param.sfo && \
  cd /src && \
  vita-pack-vpk -s build/param.sfo -b build/eboot.bin \
                -a vpk/icon0.png=sce_sys/icon0.png \
                -a vpk/bg.png=sce_sys/livearea/contents/bg.png \
                -a vpk/startup.png=sce_sys/livearea/contents/startup.png \
                -a vpk/template.xml=sce_sys/livearea/contents/template.xml \
                -a vpk/web_button.png=sce_sys/livearea/contents/web_button.png \
                -a vpk/bgm.at9=sce_sys/livearea/contents/bgm.at9 \
                GrooveSpeedVita.vpk && \
  ls -lh GrooveSpeedVita.vpk
"
```

The output file **`GrooveSpeedVita.vpk`** will be generated in your project root directory.

---

## 💻 Method 2: Native VitaSDK Build (Local Toolchain)

If you have [VitaSDK](https://vitasdk.org/) installed natively on your system (`/usr/local/vitasdk`):

### 1. Set Environment Variables
```bash
export VITASDK=/usr/local/vitasdk
export PATH=$VITASDK/bin:$PATH
```

### 2. Configure with CMake
```bash
rm -rf build
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.cmake ..
```

### 3. Compile ELF Binary
```bash
make GrooveSpeedVita
```

### 4. Create Executable & Metadata
```bash
# Convert ELF to VELF (Vita ELF)
vita-elf-create GrooveSpeedVita GrooveSpeedVita.velf

# Create signed self binary
vita-make-fself GrooveSpeedVita.velf eboot.bin

# Generate param.sfo metadata
vita-mksfoex -s TITLE_ID="GROOVE001" -s APP_VER="01.00" "GrooveSpeed" param.sfo
```

### 5. Package VPK
```bash
cd ..
vita-pack-vpk -s build/param.sfo -b build/eboot.bin \
              -a vpk/icon0.png=sce_sys/icon0.png \
              -a vpk/bg.png=sce_sys/livearea/contents/bg.png \
              -a vpk/startup.png=sce_sys/livearea/contents/startup.png \
              -a vpk/template.xml=sce_sys/livearea/contents/template.xml \
              -a vpk/web_button.png=sce_sys/livearea/contents/web_button.png \
              -a vpk/bgm.at9=sce_sys/livearea/contents/bgm.at9 \
              GrooveSpeedVita.vpk
```

---

## 🕹️ Testing in Vita3K Emulator

To run the freshly compiled app directly in Vita3K:

### macOS
```bash
# 1. Create app directory in Vita3K virtual filesystem
mkdir -p "$HOME/Library/Application Support/Vita3K/Vita3K/fs/ux0/app/GROOVE001/sce_sys/livearea/contents"

# 2. Copy binaries and assets
cp -f build/eboot.bin "$HOME/Library/Application Support/Vita3K/Vita3K/fs/ux0/app/GROOVE001/eboot.bin"
cp -f build/param.sfo "$HOME/Library/Application Support/Vita3K/Vita3K/fs/ux0/app/GROOVE001/sce_sys/param.sfo"
cp -f vpk/icon0.png "$HOME/Library/Application Support/Vita3K/Vita3K/fs/ux0/app/GROOVE001/sce_sys/icon0.png"
cp -f vpk/bg.png "$HOME/Library/Application Support/Vita3K/Vita3K/fs/ux0/app/GROOVE001/sce_sys/livearea/contents/bg.png"

# 3. Launch cleanly (detached from terminal pipe)
open /Applications/Vita3K.app --args -w -r GROOVE001
```

### Linux
```bash
mkdir -p "$HOME/.local/share/Vita3K/Vita3K/fs/ux0/app/GROOVE001/sce_sys/livearea/contents"
cp -f build/eboot.bin "$HOME/.local/share/Vita3K/Vita3K/fs/ux0/app/GROOVE001/eboot.bin"
cp -f build/param.sfo "$HOME/.local/share/Vita3K/Vita3K/fs/ux0/app/GROOVE001/sce_sys/param.sfo"
cp -f vpk/icon0.png "$HOME/.local/share/Vita3K/Vita3K/fs/ux0/app/GROOVE001/sce_sys/icon0.png"
cp -f vpk/bg.png "$HOME/.local/share/Vita3K/Vita3K/fs/ux0/app/GROOVE001/sce_sys/livearea/contents/bg.png"

vita3k -w -r GROOVE001
```

---

## 📱 Installing on a Real PS Vita

1. Launch **VitaShell** on your PS Vita.
2. Press **Select** to start FTP or USB mode.
3. Transfer `GrooveSpeedVita.vpk` to `ux0:data/` (or any folder).
4. In VitaShell, navigate to `ux0:data/` and highlight `GrooveSpeedVita.vpk`.
5. Press **Cross (✕)** to install the package.
6. Return to the LiveArea home screen to launch **GrooveSpeed**.

---

## ⚙️ Configuration & Versioning

- **Application Title**: Defined in `vita-mksfoex` (`"GrooveSpeed"`).
- **Title ID**: `GROOVE001` (Folder in `ux0:app/GROOVE001/`).
- **Version Number**: Set via `-s APP_VER="01.00"` in `vita-mksfoex` and `set(VITA_VERSION "01.00")` in `CMakeLists.txt`.
- **LiveArea Assets**:
  - `vpk/icon0.png`: App icon (128×128, 8-bit or 24-bit PNG, max 100KB).
  - `vpk/bg.png`: LiveArea background (840×500, 24-bit PNG, max 1MB).
