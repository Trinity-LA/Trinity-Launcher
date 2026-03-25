#!/bin/bash

# ==========================================
# 🛠️ Trinity Launcher Build Script
# ==========================================

# Security setting: stop on errors
set -e

# Terminal colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Default variables
BUILD_TYPE="Release"
CLEAN_BUILD=false
ONLY_CLEAN=false
ONLY_DEPS=false
UPDATE_TRANSLATIONS=false
INSTALL_DEPS=false
INSTALL_SYSTEM=false
RUN_APP=false
DETACHED=false
UNINSTALL=false
BUILD_DIR="build"

show_banner() {
    echo -e "${CYAN}"
    echo "  _______   _       _ _            "
    echo " |__   __| (_)     (_) |           "
    echo "    | |_ __ _ _ __  _| |_ _   _    "
    echo "    | | '__| | '_ \| | __| | | |   "
    echo "    | | |  | | | | | | |_| |_| |   "
    echo "    |_|_|  |_|_| |_|_|\__|\__, |   "
    echo "                           __/ |   "
    echo "                          |___/    "
    echo -e "${NC}"
    echo -e "${BLUE}=== Trinity Launcher Build System ===${NC}"
    echo ""
}

ensure_sudo() {
 if [ -n "$CI" ]; then
        return 0
    fi
    echo -e "${YELLOW}🔐 Administrator permissions are required for this action...${NC}"
    if ! sudo -v; then
        echo -e "${RED}continue...${NC}"
    fi
}

detect_distro() {
    DISTRO_FAMILY=""
    OS_ID=""

    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS_ID=$ID
        case "$ID" in
            debian|ubuntu|linuxmint|pop|raspbian|kali|neon) DISTRO_FAMILY="debian_based" ;;
            fedora|rhel|centos|rocky|almalinux) DISTRO_FAMILY="fedora_based" ;;
            opensuse-leap|opensuse-tumbleweed) DISTRO_FAMILY="opensuse_based" ;;
            arch|manjaro|endeavouros|garuda) DISTRO_FAMILY="arch_based" ;;
            *)
                case "$ID_LIKE" in
                    *debian*) DISTRO_FAMILY="debian_based" ;;
                    *fedora*|*rhel*) DISTRO_FAMILY="fedora_based" ;;
                    *suse*) DISTRO_FAMILY="opensuse_based" ;;
                    *arch*) DISTRO_FAMILY="arch_based" ;;
                    *) DISTRO_FAMILY="unknown" ;;
                esac
                ;;
        esac
    elif type lsb_release >/dev/null 2>&1; then
        local lsb_id=$(lsb_release -si | tr '[:upper:]' '[:lower:]')
        case "$lsb_id" in
            ubuntu|debian|linuxmint|pop|sparky|raspbian) DISTRO_FAMILY="debian_based" ;;
            fedora|redhat|centos) DISTRO_FAMILY="fedora_based" ;;
            opensuse|suse) DISTRO_FAMILY="opensuse_based" ;;
            arch|manjaro) DISTRO_FAMILY="arch_based" ;;
            *) DISTRO_FAMILY="unknown" ;;
        esac
        OS_ID=$lsb_id
    elif [ -f /etc/arch-release ]; then DISTRO_FAMILY="arch_based"; OS_ID="arch";
    else DISTRO_FAMILY="unknown"; OS_ID=$(uname -s | tr '[:upper:]' '[:lower:]');
    fi
}

install_dependencies() {
    detect_distro
    echo -e "${CYAN}🔍 Detected system: $OS_ID ($DISTRO_FAMILY)${NC}"
    echo -e "${YELLOW}📦 Installing dependencies... (sudo will be required)${NC}"

    ensure_sudo

    case "$DISTRO_FAMILY" in
        "debian_based")
            sudo apt-get update
            sudo apt-get install -y build-essential git curl cmake clang ninja-build \
                qt6-base-dev qt6-base-dev-tools qt6-declarative-dev qt6-webengine-dev qt6-svg-dev qt6-tools-dev \
                libcurl4-openssl-dev libssl-dev libasound2-dev libpulse-dev libjack-jackd2-dev libpipewire-0.3-dev \
                libx11-dev libxi-dev libxext-dev libxfixes-dev libxcursor-dev libxrandr-dev libxss-dev libxtst-dev \
                libgl1-mesa-dev libegl1-mesa-dev libgles2-mesa-dev libvulkan-dev vulkan-validationlayers \
                libdrm-dev libgbm-dev libudev-dev libevdev-dev libusb-1.0-0-dev libdbus-1-dev bluez \
                libibus-1.0-dev libxkbcommon-dev libpng-dev libzip-dev libcups2-dev libwayland-dev libunwind-dev libdecor-0-dev
            ;;
        "fedora_based")
            sudo dnf groupinstall -y "Development Tools"
            sudo dnf install -y git curl cmake clang ninja-build \
                qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtwebengine-devel qt6-qtsvg-devel qt6-qttools-devel \
                libcurl-devel openssl-devel alsa-lib-devel pulseaudio-libs-devel pipewire-devel \
                libX11-devel libXi-devel libXext-devel libXfixes-devel libXcursor-devel libXrandr-devel libXtst-devel \
                mesa-libGL-devel vulkan-loader-devel libdrm-devel libgbm-devel systemd-devel libevdev-devel \
                libusb1-devel dbus-devel bluez-libs-devel libxkbcommon-devel libpng-devel libzip-devel \
                wayland-devel libunwind-devel libdecor-devel
            ;;
        "arch_based")
            sudo pacman -S --needed base-devel git curl cmake clang ninja \
                qt6-base qt6-declarative qt6-webengine qt6-svg qt6-tools qt6-translations \
                libzip libpng libpulse alsa-lib pipewire jack2 sndio \
                libx11 libxi libxext libxfixes libxcursor libxrandr libxss libxtst \
                mesa vulkan-devel vulkan-validation-layers libdrm libgbm \
                libevdev libusb dbus bluez ibus libxkbcommon libunwind libdecor wayland
            ;;
        "opensuse_based")
            sudo zypper install -y -t pattern devel_basis
            sudo zypper install -y git curl cmake clang ninja libqt6-qtbase-devel \
                libqt6-qtdeclarative-devel libqt6-qtwebengine-devel libqt6-qtsvg-devel libqt6-qttools-devel \
                libzip-devel libpng16-devel libopenssl-devel libevdev-devel libdecor-0-devel
            ;;
        *)
            echo -e "${RED}Distro not automatically supported. Check the README.${NC}"
            exit 1
            ;;
    esac
    echo -e "${GREEN}Dependencies installed.${NC}"
}

uninstall_app() {
    echo -e "${YELLOW}🗑️  Starting uninstallation process...${NC}"
    echo -e "${YELLOW}🔐 Permissions are required to remove system files (/usr/local/bin, etc)${NC}"
    
    ensure_sudo

    # Kill the process if running to avoid errors when deleting
    if pgrep -x "trinity" > /dev/null; then
        echo -e "   Stopping Trinity Launcher execution..."
        killall trinity || true
    fi

    echo -e "   Removing binaries..."
    sudo rm -f /usr/local/bin/trinity

    echo -e "   Removing resources (icons and shortcuts)..."
    sudo rm -f /usr/share/icons/com.trench.trinity.launcher.svg
    sudo rm -f /usr/share/applications/com.trench.trinity.launcher.desktop

    echo -e "${GREEN}✅ Trinity Launcher has been removed from the system.${NC}"
    
    # We don't automatically delete the data folder (~/.local/share/mcpelauncher)
    # because user's worlds and saved games are there.
    echo -e "${BLUE}ℹ️  Note: Game data (worlds, skins) is kept in:${NC}"
    echo -e "   ~/.local/share/mcpelauncher/"
    echo -e "   If you wish to delete them too, run: rm -rf ~/.local/share/mcpelauncher/"
}

# Help function
show_help() {
    echo -e "${BLUE}Usage: ./build.sh [OPTIONS]${NC}"
    echo ""
    echo "Options:"
    echo "  --debug      Compile in Debug mode (with symbols for debugging)"
    echo "  --release    Compile in Release mode (optimized, default)"
    echo "  --clean      Delete build/ and RECOMPILE"
    echo "  --clean-only Delete build/ and EXIT (Without compiling)"
    echo "  --update-ts  Scan code and update translation .ts files"
    echo "  --deps       Install system dependencies (detects distro automatically) and COMPILE"
    echo "  --deps-only  Install system dependencies (detects distro automatically) and EXIT"
    echo "  --install    Install to system (/usr/local/bin)"
    echo "  --uninstall  Uninstall from system (/usr/local/bin)"
    echo "  --run        Run Trinity upon completion"
    echo "  --detached   Used next to --run flag to hide information during launcher execution"
    echo "  --help       Show this help"
    echo ""
}

# 1. Process arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift ;;
        --release)
            BUILD_TYPE="Release"
            shift ;;
        --clean)
            CLEAN_BUILD=true
            shift ;;
        --clean-only) CLEAN_BUILD=true; ONLY_CLEAN=true; shift ;;
        --deps-only) INSTALL_DEPS=true; ONLY_DEPS=true; shift ;;
        --update-ts)
            UPDATE_TRANSLATIONS=true
            shift ;;
        --deps)
            INSTALL_DEPS=true
            shift ;;
        --run)
            RUN_APP=true
            shift ;;
        --install)
            INSTALL_SYSTEM=true;
            shift ;;
        --uninstall)
            UNINSTALL=true;
            shift ;;
        --detached) DETACHED=true; shift ;;
        --help)
            show_help
            exit 0 ;;
        *)
            echo -e "${RED}Error: Unknown option $1${NC}"
            show_help
            exit 1 ;;
    esac
done

show_banner;

if [ "$(id -u)" -eq 0 ]; then
   echo -e "${RED}❌ CRITICAL ERROR:${NC} Do not run this script with 'sudo'."
   echo -e "   The script will ask for administrator permissions automatically"
   echo -e "   only when needed to install dependencies and configure."
   echo -e "   ${YELLOW}Run: make start or make run (./build.sh)${NC}"
   exit 1
fi

# This is vital: If you are going to clean or compile, we first ensure the folder is yours.
# we ask for sudo ONLY to fix permissions and give them back to you.
# We check if the folder exists and if there are root files inside.
if [ -d "$BUILD_DIR" ]; then
    # Look for any file owned by root (user ID 0)
    # -print -quit makes it stop at the first one found (faster)
    ROOT_FILES=$(find "$BUILD_DIR" -user 0 -print -quit 2>/dev/null)

    if [ ! -w "$BUILD_DIR" ] || [ -n "$ROOT_FILES" ]; then
        echo -e "${YELLOW}⚠️  Files created by root detected in '$BUILD_DIR'.${NC}"
        echo -e "${YELLOW}🔓 Requesting permissions to regain ownership...${NC}"
        
        ensure_sudo
        
        if sudo chown -R $USER:$USER "$BUILD_DIR"; then
            echo -e "${GREEN}✅ Permissions corrected.${NC}"
        else
            echo -e "${RED}❌ Permission correction failed.${NC}"; exit 1
        fi
    fi
fi

# Run priority tasks
if [ "$INSTALL_DEPS" = true ]; then 
    install_dependencies
    if [ "$ONLY_DEPS" = true ]; then
        echo -e "${GREEN}✅ Dependencies ready.${NC}"
        exit 0
    fi
fi
if [ "$UNINSTALL" = true ]; then uninstall_app; exit 0; fi

# 2. Update Translations (If requested)
if [ "$UPDATE_TRANSLATIONS" = true ]; then
    echo -e "${YELLOW}🌍 Updating translation files (.ts)...${NC}"
    
    # 1. Try to find lupdate in normal PATH
    if command -v lupdate &> /dev/null; then
        LUPDATE_CMD="lupdate"
    # 2. If it fails, try to find it in specific Arch Linux / Qt6 path
    elif [ -f "/usr/lib/qt6/bin/lupdate" ]; then
        LUPDATE_CMD="/usr/lib/qt6/bin/lupdate"
    else
        echo -e "${RED}Error: 'lupdate' not found. Install 'qt6-tools' (Arch) or 'qt6-tools-dev' (Debian).${NC}"
        exit 1
    fi

    echo -e "${BLUE}   Using: $LUPDATE_CMD${NC}"

    # Run lupdate using the variable we found
    $LUPDATE_CMD src/ include/ -recursive -ts resources/i18n/*.ts
    
    echo -e "${GREEN}✅ .ts files updated.${NC}"
  # It ends here because it will only update .ts files
  exit 0
fi

# 3. Verify environment
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: CMakeLists.txt not found. Run this script from the project root.${NC}"
    exit 1
fi

echo -e "${BLUE}=== Starting build process ($BUILD_TYPE) ===${NC}"

# 4. Cleaning (if requested)
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}🧹 Cleaning previous builds (--clean)...${NC}"
    rm -rf "$BUILD_DIR"
    if [ "$ONLY_CLEAN" = true ]; then
        echo -e "${GREEN}✅ Cleaning completed. Exiting.${NC}"
        exit 0
    fi
fi

# 5. Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

# 6. Configure CMake
echo -e "${BLUE}🔧 Configuring project...${NC}"
# Force Clang++ and use Ninja if available
CMAKE_EXTRA_ARGS="-DCMAKE_CXX_COMPILER=clang++"
if command -v ninja &> /dev/null; then
    CMAKE_EXTRA_ARGS="$CMAKE_EXTRA_ARGS -G Ninja"
fi
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE $CMAKE_EXTRA_ARGS -Wno-dev

# 7. Compile
echo -e "${BLUE}🔨 Compiling...${NC}"
if cmake --build "$BUILD_DIR" --parallel $(nproc); then
    echo -e "${GREEN}✅ Build successful.${NC}"
else
    echo -e "${RED}❌ Error during compilation.${NC}"
    exit 1
fi

if [ "$INSTALL_SYSTEM" = true ]; then
    echo -e "${BLUE}📦 Starting system installation...${NC}"

    ensure_sudo

    # Install Trinity
    if [ -f "$BUILD_DIR/app/trinity" ]; then
        sudo cp -rf "$BUILD_DIR/app/trinity" /usr/local/bin
        echo -e "   -> trinity installed in /usr/local/bin"
    else
        echo -e "${RED}❌ Error: Binary 'trinity' not found. Compile first.${NC}"
        exit 1
    fi

    # Install Icons
    if [ -f "resources/branding/com.trench.trinity.launcher.svg" ]; then
        sudo cp -rf resources/branding/com.trench.trinity.launcher.svg /usr/share/icons/
    else
        echo -e "${YELLOW}⚠️  Icon (.svg) not found, copy skipped.${NC}"
    fi

    # Install Shortcut
    if [ -f "resources/shortcuts/com.trench.trinity.launcher.desktop" ]; then
        sudo cp -rf resources/shortcuts/com.trench.trinity.launcher.desktop /usr/share/applications/
    else
        echo -e "${YELLOW}⚠️  Shortcut (.desktop) not found, copy skipped.${NC}"
    fi

    echo -e "${GREEN}✅ Installation completed.${NC}"

    if [ "$RUN_APP" = false ]; then
        echo ""
        echo -e "${CYAN}❓ Do you want to start Trinity Launcher now? (y/n)${NC}"
        read -p "" -n 1 -r REPLY
        echo ""
        if [[ $REPLY =~ ^[SsYy]$ ]]; then
            echo -e "${GREEN}🚀 Launching in background...${NC}"
            RUN_APP=true
            DETACHED=true
        fi
    fi
fi

if [ "$RUN_APP" = true ]; then
    
    APP_PATH=""
    # Binary selection
    if [ -f "$BUILD_DIR/app/trinity" ]; then
        APP_PATH="$BUILD_DIR/app/trinity"
    elif command -v trinity &> /dev/null; then
        APP_PATH=$(command -v trinity)
    else
        echo -e "${RED}❌ Could not find the executable.${NC}"
        exit 1
    fi

    # --- CASE A: USER MODE (No logs, releases terminal) ---
    if [ "$DETACHED" = true ]; then
        echo -e "${CYAN}🎮 Launching Trinity...${NC}"
        "$APP_PATH" & > /dev/null 2>&1
        echo -e "${GREEN}✅ Application started in background.${NC}"
    
    # --- CASE B: DEV MODE (Logs, Ctrl+C, Waits) ---
    else
        echo -e "${CYAN}🎮 Launching Trinity (Development Mode)...${NC}"
        echo -e "${YELLOW}⚡ Executing: $APP_PATH${NC}"
        echo -e "${YELLOW}ℹ️  Live logs. Press Ctrl+C to stop.${NC}"
        echo ""

        cleanup() {
            echo ""
            echo -e "${RED}🛑 Stopping application...${NC}"
            if [ -n "$APP_PID" ]; then kill "$APP_PID" 2>/dev/null; fi
            exit 0
        }
        trap cleanup SIGINT

        "$APP_PATH" &
        APP_PID=$!
        wait "$APP_PID"
        
        EXIT_CODE=$?
        echo ""
        if [ $EXIT_CODE -eq 0 ]; then
            echo -e "${GREEN}✅ Application closed correctly.${NC}"
        else
            echo -e "${RED}⚠️  Exit with code: $EXIT_CODE${NC}"
        fi
    fi
fi

echo ""
if [ "$DETACHED" = true ]; then
    echo -e "${GREEN}🎉 All set!${NC}"
else
    echo -e "${GREEN}🎉 Session finished.${NC}"
fi

echo ""
echo -e "${GREEN}🎉 All set!${NC}"
