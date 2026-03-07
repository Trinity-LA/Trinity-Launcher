# 🛠️ Build Guide - Trinity Launcher

This document details the necessary steps to set up the development environment, install dependencies, and compile **Trinity Launcher** from the source code.

---

## 📋 Index
1. [Prerequisite: Game Engine (MCPE Launcher)](#1-prerequisite-game-engine-mcpe-launcher)
2. [Architecture and Dependencies](#2-architecture-and-dependencies)
3. [Dependency Installation by Distribution](#3-dependency-installation-by-distribution)
4. [Method 1: Automated Compilation (Recommended)](#4-method-1-automated-compilation-recommended)
5. [Method 2: Manual Compilation with CMake/Ninja](#5-method-2-manual-compilation-with-cmake-ninja)
6. [Method 3: Isolated Environment with Docker](#6-method-3-isolated-environment-with-docker)
7. [Method 4: Flatpak Packaging](#7-method-4-flatpak-packaging)

## 1. Prerequisite: Game Engine (MCPE Launcher)

Trinity Launcher functions as a *frontend* user interface. For the game to be executable, the system requires the binaries from the **[mcpelauncher](https://github.com/minecraft-linux/mcpelauncher-manifest)** project.

Trinity depends on these binaries to perform data extraction (`mcpelauncher-extract`) and native call translation (`mcpelauncher-client`).

**Engine Installation (Fork compatible with 1.21.131+):**
```bash
# Clone and prepare
git clone https://github.com/franckey02/mcpelauncher-patch.git
cd mcpelauncher-patch
git checkout qt6
git submodule update --init --recursive

# Compile
mkdir -p build && cd build
CC=clang CXX=clang++ cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_POLICY_DEFAULT_CMP0074=NEW \
  -DCMAKE_C_FLAGS="-march=x86-64 -mtune=generic -msse4.1 -msse4.2 -mpopcnt" \
  -DCMAKE_CXX_FLAGS="-march=x86-64 -mtune=generic -msse4.1 -msse4.2 -mpopcnt" \
  -Wno-dev

# Build and install
make -j$(nproc)
sudo make install
```
*After this step, the executables `mcpelauncher-client` and `mcpelauncher-extract` will be available on the system.*

---

## 2. Understanding the Dependencies

It is crucial to understand that this project requires two sets of dependencies:
1. **Trinity Launcher (The Interface):** Requires C++ build tools (Clang/GCC, CMake, Ninja) and the **Qt6** framework libraries (`Core`, `Widgets`, `Concurrent`, `Svg`).
2. **MCPE Launcher (The Game Engine):** Requires low-level system dependencies (X11, Wayland, Vulkan, EGL, ALSA, PulseAudio, OpenSSL, etc.) to translate Android calls to Linux.

---

## 3. Dependency Installation by Distribution

### 🟢 Ubuntu / Debian / Pop!_OS / Mint
```bash
sudo apt update
sudo apt install build-essential git curl cmake clang ninja-build \
    qt6-base-dev qt6-base-dev-tools qt6-declarative-dev qt6-webengine-dev qt6-svg-dev qt6-tools-dev \
    libcurl4-openssl-dev libssl-dev libasound2-dev libpulse-dev libjack-jackd2-dev libpipewire-0.3-dev \
    libx11-dev libxi-dev libxext-dev libxfixes-dev libxcursor-dev libxrandr-dev libxss-dev libxtst-dev \
    libgl1-mesa-dev libegl1-mesa-dev libgles2-mesa-dev libvulkan-dev vulkan-validationlayers \
    libdrm-dev libgbm-dev libudev-dev libevdev-dev libusb-1.0-0-dev libdbus-1-dev bluez \
    libibus-1.0-dev libxkbcommon-dev libpng-dev libzip-dev libcups2-dev libwayland-dev \
    libunwind-dev libdecor-0-dev
```

### 🔵 Arch Linux / Manjaro / EndeavourOS
```bash
sudo pacman -S --needed base-devel git curl cmake clang ninja \
    qt6-base qt6-declarative qt6-webengine qt6-svg qt6-tools qt6-translations \
    libzip libpng libpulse alsa-lib pipewire jack2 sndio \
    libx11 libxi libxext libxfixes libxcursor libxrandr libxss libxtst \
    mesa vulkan-devel vulkan-validation-layers libdrm libgbm \
    libevdev libusb dbus bluez ibus libxkbcommon libunwind libdecor wayland
```

### 🔴 Fedora / Rocky Linux
```bash
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y git curl cmake clang ninja-build \
    qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtwebengine-devel qt6-qtsvg-devel qt6-qttools-devel \
    libcurl-devel openssl-devel alsa-lib-devel pulseaudio-libs-devel pipewire-devel \
    libX11-devel libXi-devel libXext-devel libXfixes-devel libXcursor-devel libXrandr-devel libXtst-devel \
    mesa-libGL-devel vulkan-loader-devel libdrm-devel libgbm-devel systemd-devel libevdev-devel \
    libusb1-devel dbus-devel bluez-libs-devel libxkbcommon-devel libpng-devel libzip-devel \
    wayland-devel libunwind-devel libdecor-devel
```

---

## 4. Method 1: Automated Compilation (Recommended)

The project includes a `Makefile` that acts as a wrapper for the `build.sh` script, providing clearer visibility of the parameters that can be used.

From the project root, run:

```bash
# 1. Install dependencies, compile, and run (Ideal for the first time)
make start

# 2. Recompile quickly after making code changes and test
make run

# 3. Compile and install the binaries on your system (/usr/local/bin)
make install
```

**Other useful commands:**
* `make clean` - Cleans compilation files and repairs permissions if CMake was accidentally run as root.
* `make translations` - Updates `.ts` files by analyzing the source code for new `tr()` strings.

---

## 5. Method 2: Manual Compilation with CMake/Ninja

Using Ninja:

```bash
mkdir build && cd build

# Configure the project
export CC=clang
export CXX=clang++
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..

# Compile
ninja

# Run binary
./app/trinity
```

Using CMake:

```bash
# Configure specifying compiler and build mode
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

# Build
cmake --build build --parallel $(nproc)

# Run binary
./app/trinity
```
---

## 6. Method 3: Isolated Environment with Docker

**Using Docker Compose (Recommended):**
```bash
# Build the image with your user's permissions
UID=$(id -u) GID=$(id -g) docker compose build

# Enter the container
UID=$(id -u) GID=$(id -g) docker compose run --rm trinity

# Once inside, compile:
./build.sh --clean --release
```

**Using pure Docker:**
```bash
docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t trinity-launcher .

docker run --rm -it \
  --user $(id -u):$(id -g) \
  -e DISPLAY=$DISPLAY \
  -v "$(pwd)":/project \
  -v cargo-cache:/home/trinity/.cargo/registry \
  trinity-launcher /bin/bash
```

---

## 7. Method 4: Flatpak Packaging

To compile and package Trinity as a Flatpak application (ideal for distribution):

```bash
# 1. Install the base tools and the KDE/Qt6 SDK
flatpak install flathub io.qt.qtwebengine.BaseApp//6.9
flatpak install flathub org.kde.Platform//6.9 org.kde.Sdk//6.9

# 2. Build the directory
flatpak-builder --user --force-clean build-dir com.trench.trinity.launcher.json

# 3. Export to a local repository
flatpak-builder --repo=repo --force-clean build-dir com.trench.trinity.launcher.json

# 4. Create the .flatpak file
flatpak build-bundle repo trinity.flatpak com.trench.trinity.launcher

# 5. Install on the system
flatpak install ./trinity.flatpak

