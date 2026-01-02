
# 🎮 Trinity Launcher

[Español](README.md) 

To install and learn the detailed steps, visit our [official website](https://trinitylauncher.vercel.app)  

> **Modular graphic environment For Minecraft Bedrock Linux**

[![License](https://img.shields.io/badge/license-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Qt](https://img.shields.io/badge/Qt-6-41CD52?logo=qt)](https://www.qt.io/)
[![GitHub](https://img.shields.io/badge/GitHub-Source-181717?logo=github)](https)
[![Flatpak](https://img.shields.io/badge/Flatpak-ready-6666FF?logo=flatpak)](https://flatpak.org/)

**Trinity Launcher** is a modern graphic environment, modular and functional for running and managing **Minecraft: Bedrock Edition** on Linux. Designed to work both on the system and within **Flatpak**, it uses **Qt6** and follows a clean architecture based on separate libraries (`core` and `ui`).

Includes two complementary applications:

- **`Trinchete`** 🚀 → **Principal Launcher**: Advanced version management, export/import, shortcuts.
- **`Trinito`** 📦 → **Content manager**: Instalation, activation/deactivation and removal of mods, textures, development packs, and worlds.

---

## 📋 Table of Contents

- [Technologies](#-technologies)
- [Trinchete — Launcher UI](#-trinchete--launcher-ui)
- [Trinchete — Multiversion Launcher](#-trinito--multiversion-launcher)
- [Trinito — Content manager](#-trinito--content-manager)
- [Compilation and Installation](#-compilation-and-installation)
- [MCPElauncher required](#-mcpelauncher-required)
- [Packaging in Flatpak](#-packaging-in-flatpak)
- [Testing](#-testing)
- [License](#-license)

---

## 🛠️ Technologies

### Development stack

| Component         | Description                              | Version        |
|-------------------|------------------------------------------|----------------|
| **Language**      | C++ Standard                             | C++17          |
| **Framework UI**  | Qt Framework                             | Qt 6.6+        |
| **Build System**  | CMake                                    | 3.17+          |
| **Compiler**      | Clang                                    | 16+            |
| **Packaging**     | Flatpak                                  |                |
| **Platform**      | Linux (x86_64, ARM64)                    | glibc          |

### Modular architecture

- **`TrinityCore`**: Business logic (version management, packs, release, export).
- **`TrinityUI`**: Graphical interfaces (windows, dialogs, widgets).
- **Clear separation**: Facilitates maintenance, testing, and reuse.

---

## 🎮 Trinchete — Launcher UI

### Top bar
- **+ Extract APK**: Select an `.apk` file, give it a name, and extract it with `mcpelauncher-extract`.
- **Import**: Restores a saved version in `.tar.gz` (includes game and data from `com.mojang`).
- **Tools**: Open the `trinito` application.

### Right panel (selected version)
- **PLAY**: Run `mcpelauncher-client -dg <ruta>` and close the launcher.
- **Create Shortcut**: Generate a `.desktop` file in `~/Downloads/` to launch this version via Flatpak.
- **Edit Configuration**: Allows you to add environment variables or arguments (e.g., `DRI_PRIME=1`) stored in `trinity-config.txt`.
- **Export**: Save the version + your data in a compressed file (`.tar.gz`).
- **Remove**: Remove the version permanently.

> ✅ **Modern interface with dark theme and support for icons**.

---

## 🧰 Trinchete — Multiversion Launcher

### Tabs

| Tabs      | Selection type | Destiny                                              |
|-----------------|-------------------|---------------------------------------------|
| **Mods**        | File              | `behavior_packs/`                           |
| **Textures**    | File              | `resource_packs/`                           |
| **Worlds**      | **Folder**        | `minecraftWorlds/`                          |
| **Development** | File           | `development_behavior_packs/` and `development_resource_packs/` |

### Key features
- **Installation**: Button to select file or folder.
- **Enable/Disable**:
- ✅ **Enabled**: Normal name.
- ⬜ **Disabled**: Renamed to `.disabled` and compressed with `tar`.
- **Management**: Interactive checkboxes, reload, and delete.
- **Secure validation**: Prompts before replacing or deleting.

---

## ⚙️ Compilation and Installation
You can follow our manual at:
https://trinity-la.github.io/
### Requeriments:
- CMake 3.17+
- Clang
- Qt6 (Core, Widgets)

### Process
```sh
# Grant execution permissions
chmod +x build.sh

# Compile and install on the system
sudo ./build.sh
```

> 📦 Install `trinchete` and `trinito` in `/usr/local/bin`, and register the `.desktop` file and icon in the system.

Once installed, run from you preferred terminal:
```sh
trinchete
trinito
```

---

## 🔧 MCPElauncher Required

Trinity Launcher requires the binaries `mcpelauncher-client`, `mcpelauncher-extract`, and `mcpelauncher-webview`.

### Recomendation
Usa the maintained:  
[https://github.com/franckey02/mcpelauncher-patch](https://github.com/franckey02/mcpelauncher-patch)  
(Compatible with recent versions of Minecraft, including **1.21.131+** and betas).

### Instruccions
**Dependences:**
- Compiler C/C++ (gcc, g++)
- Make
- Autotools (autoconf, automake, libtool)
- Git
- cURL
- CMake
- Clang/LLVM
- Ninja
- Qt Core
- Qt GUI
- Qt Widgets
- Qt QML/Quick
- Qt WebEngine
- Qt SVG
- Qt Tools
- OpenSSL
- TLS/SSL
- ALSA
- PulseAudio
- JACK Audio
- PipeWire
- Sndio
- X11 (Xlib, Xext, Xi, Xfixes, Xcursor, Xrandr, XSS, XTest)
- OpenGL
- EGL
- GLES
- Vulkan
- DRM
- GBM
- Wayland
- udev
- evdev
- USB (libusb)
- D-Bus
- Bluetooth
- CUPS
- IBus
- xkbcommon
- libpng
- libzip
- libdecor
- libunwind

Note: If you use the dockerfile, you save yourself from having to install so many dependencies.

**Commands for compiling**
```sh
git clone https://github.com/franckey02/mcpelauncher-patch.git
cd mcpelauncher-patch
git checkout qt6
git submodule update --init --recursive
mkdir -p build
cd build

CC=clang CXX=clang++ cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_POLICY_DEFAULT_CMP0074=NEW \
  -DCMAKE_C_FLAGS="-march=x86-64 -mtune=generic -msse4.1 -msse4.2 -mpopcnt" \
  -DCMAKE_CXX_FLAGS="-march=x86-64 -mtune=generic -msse4.1 -msse4.2 -mpopcnt" \
  -Wno-dev

make -j$(getconf _NPROCESSORS_ONLN)
sudo make install
```

---

## 📦 Packaging in Flatpak

### Requisites
```sh
flatpak install flathub io.qt.qtwebengine.BaseApp//6.9
flatpak install flathub org.kde.Platform//6.9 org.kde.Sdk//6.9
```

### Bulding
```sh
flatpak-builder --user --force-clean build-dir com.trench.trinity.launcher.json
flatpak-builder --repo=repo --force-clean build-dir com.trench.trinity.launcher.json
flatpak build-bundle repo trinity.flatpak com.trench.trinity.launcher
flatpak install ./trinity.flatpak
```

> ✅ The manifest must include `libevdev`, `libzip`, and copy `files/` to `/app`.

---

## 🧪 Testing

### On system
```sh
trinchete
trinito
```

### On Flatpak
```sh
flatpak run com.trench.trinity.launcher
flatpak run --command=trinito com.trench.trinity.launcher
```

### Data paths (automatic)
- **Flatpak**: `~/.var/app/com.trench.trinity.launcher/data/mcpelauncher/`
- **Local**: `~/.local/share/mcpelauncher/`

> Both use `QStandardPaths::GenericDataLocation` → **full compatibility**.

---

## 📄 License

Trinity Launcher is distributed under the **BSD-3-Clause license**.
