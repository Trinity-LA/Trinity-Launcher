#!/usr/bin/env bash

# ==========================================
# 🛠️ Trinity Launcher Build Script
# ==========================================

# Configuración de seguridad: detener si hay errores
set  

# Colores para la terminal
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Variables por defecto
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
    echo -e  "${CYAN}"
    echo "  _______   _       _ _            "
    echo " |__   __| (_)     (_) |           "
    echo "    | |_ __ _ _ __  _| |_ _   _    "
    echo "    | | '__| | '_ \| | __| | | |   "
    echo "    | | |  | | | | | | |_| |_| |   "
    echo "    |_|_|  |_|_| |_|_|\__|\__, |   "
    echo "                           __/ |   "
    echo "                          |___/    "
    echo -e  "${NC}"
    echo -e  "${BLUE}=== Trinity Launcher Build System ===${NC}"
    echo ""
}

ensure_sudo() {
 if [ -n "$CI" ]; then
        return 0
    fi
    echo   -e  "${YELLOW}🔐 Se requieren permisos de administrador para esta acción...${NC}"
    if ! sudo -v; then
        echo  -e  "${RED}❌ Error: Permiso denegado.${NC}"
        exit 1
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
            void) DISTRO_FAMILY="void_based";;
            *)
                case "$ID_LIKE" in
                    *debian*) DISTRO_FAMILY="debian_based" ;;
                    *fedora*|*rhel*) DISTRO_FAMILY="fedora_based" ;;
                    *suse*) DISTRO_FAMILY="opensuse_based" ;;
                    *arch*) DISTRO_FAMILY="arch_based" ;;
                    *void*) DISTRO_FAMILY="void_based" ;;
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
            void) DISTRO_FAMILY="void_based";;
            *) DISTRO_FAMILY="unknown" ;;
        esac
        OS_ID=$lsb_id
    elif [ -f /etc/arch-release ]; then DISTRO_FAMILY="arch_based"; OS_ID="arch";
    else DISTRO_FAMILY="unknown"; OS_ID=$(uname -s | tr '[:upper:]' '[:lower:]');
    fi
}

install_dependencies() {
    detect_distro
    echo    -e  "${CYAN}🔍 Sistema detectado: $OS_ID ($DISTRO_FAMILY)${NC}"
    echo    -e  "${YELLOW}📦 Instalando dependencias... (se requerirá sudo)${NC}"

    ensure_sudo

    case "$DISTRO_FAMILY" in
        "debian_based")
            sudo apt-get update
            sudo apt-get install -y build ssential git curl cmake clang ninja-build \
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
          "void_based")
            sudo xbps-install -Sy qt6-designer qt6-declarative-devel qt6-location-devel cmake ninja make  gcc qt6-tools-devel
           ;;
        *)
            echo    -e  "${RED}Distro no soportada automáticamente. Revisa el README.${NC}"
            exit 1
            ;;
    esac
    echo    -e  "${GREEN}Dependencias instaladas.${NC}"
}

uninstall_app() {
    echo    -e  "${YELLOW}🗑️  Iniciando proceso de desinstalación...${NC}"
    echo    -e  "${YELLOW}🔐 Se requieren permisos para eliminar archivos del sistema (/usr/local/bin, etc)${NC}"
    
    ensure_sudo

    echo "   Eliminando binarios..."
    sudo rm -f /usr/local/bin/trinity

    echo "   Eliminando recursos (iconos y accesos directos)..."
    sudo rm -f /usr/share/icons/com.trench.trinity.launcher.svg
    sudo rm -f /usr/share/applications/com.trench.trinity.launcher.desktop

    echo  -e  "${GREEN}✅ Trinity Launcher ha sido eliminado del sistema.${NC}"
    
    # No borramos la carpeta de datos (~/.local/share/mcpelauncher) automáticamente
    # porque ahí están los mundos y partidas guardadas del usuario.
    echo  -e  "${BLUE}ℹ️  Nota: Los datos del juego (mundos, skins) se mantienen en:${NC}"
    echo "   ~/.local/share/mcpelauncher/"
    echo "   Si deseas borrarlos también, ejecuta: rm -rf ~/.local/share/mcpelauncher/"
}

# Función de ayuda
show_help() {
    echo  -e  "${BLUE}Uso: ./build.sh [OPCIONES]${NC}"
    echo ""
    echo "Opciones:"
    echo "  --debug      Compila en modo Debug (con símbolos para depurar)"
    echo "  --release    Compila en modo Release (optimizado, por defecto)"
    echo "  --clean      Borra build/ y RECOMPILA"
    echo "  --clean-only Borra build/ y SALE (Sin compilar)"
    echo "  --update-ts  Escanea el código y actualiza los archivos .ts de traducción"
    echo "  --deps       Instala las dependencias del sistema (detecta distro automáticamente)"
    echo "  --install    Instala en el sistema (/usr/local/bin)"
    echo "  --run        Ejecuta Trinity al terminar"
    echo "  --help       Muestra esta ayuda"
    echo ""
}

# 1. Procesar argumentos
while [ "$#" -gt 0 ]; do
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
            echo    -e  "${RED}Error: Opción desconocida $1${NC}"
            show_help
            exit 1 ;;
    esac
done

show_banner;



# Esto es vital: Si vas a limpiar o compilar, primero aseguramos que la carpeta sea tuya.
# pedimos sudo SOLO para arreglar los permisos y devolvértelos.
# Verificamos si la carpeta existe y si hay archivos de root dentro.
if [ -d "$BUILD_DIR" ]; then
    # Buscamos si hay algún archivo propiedad de root (user ID 0)
    # -print -quit hace que se detenga al encontrar el primero (más rápido)
    ROOT_FILES=$(find "$BUILD_DIR" -user 0 -print -quit 2>/dev/null)

    if [ ! -w "$BUILD_DIR" ] || [ -n "$ROOT_FILES" ]; then
        echo    -e  "${YELLOW}⚠️  Se detectaron archivos creados por root en '$BUILD_DIR'.${NC}"
        echo    -e  "${YELLOW}🔓 Solicitando permisos para recuperar la propiedad...${NC}"
        
        ensure_sudo
        
        if sudo chown -R $USER:$USER "$BUILD_DIR"; then
            echo    -e  "${GREEN}✅ Permisos corregidos.${NC}"
        else
            echo    -e  "${RED}❌ Falló la corrección de permisos.${NC}"; exit 1
        fi
    fi
fi

# Ejecución de tareas prioritarias
if [ "$INSTALL_DEPS" = true ]; then 
    install_dependencies
    if [ "$ONLY_DEPS" = true ]; then
        echo    -e  "${GREEN}✅ Dependencias listas.${NC}"
        exit 0
    fi
fi
if [ "$UNINSTALL" = true ]; then uninstall_app; exit 0; fi

# 2. Actualizar Traducciones (Si se solicita)
if [ "$UPDATE_TRANSLATIONS" = true ]; then
    echo    -e  "${YELLOW}🌍 Actualizando archivos de traducción (.ts)...${NC}"
    
    # 1. Intentar encontrar lupdate en el PATH normal
    if command -v lupdate &> /dev/null; then
        LUPDATE_CMD="lupdate"
    # 2. Si falla, intentar encontrarlo en la ruta específica de Arch Linux / Qt6
    elif [ -f "/usr/lib/qt6/bin/lupdate" ]; then
        LUPDATE_CMD="/usr/lib/qt6/bin/lupdate"
    else
        echo    -e  "${RED}Error: 'lupdate' no encontrado. Instala 'qt6-tools' (Arch) o 'qt6-tools-dev' (Debian).${NC}"
        exit 1
    fi

    echo    -e  "${BLUE}   Usando: $LUPDATE_CMD${NC}"

    # Ejecutar lupdate usando la variable que encontramos
    $LUPDATE_CMD src/ include/ -recursive -ts resources/i18n/*.ts
    
    echo    -e  "${GREEN}✅ Archivos .ts actualizados.${NC}"
  # Se termina acá porque solo actualizará los archivos .ts
  exit 0
fi

# 3. Verificar entorno
if [ ! -f "CMakeLists.txt" ]; then
    echo    -e  "${RED}Error: No se encuentra CMakeLists.txt. Ejecuta este script desde la raíz del proyecto.${NC}"
    exit 1
fi

echo    -e  "${BLUE}=== Iniciando proceso de construcción ($BUILD_TYPE) ===${NC}"

# 4. Limpieza (si se solicita)
if [ "$CLEAN_BUILD" = true ]; then
    echo    -e  "${YELLOW}🧹 Limpiando compilaciones anteriores (--clean)...${NC}"
    rm -rf "$BUILD_DIR"
    rm -f trinity
    if [ "$ONLY_CLEAN" = true ]; then
        echo    -e  "${GREEN}✅ Limpieza completada. Saliendo.${NC}"
        exit 0
    fi
fi

# 5. Crear directorio build si no existe
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

# 5.5 Compilación de traducciones
echo    -e  "${BLUE}🌍 Genero archivos de traducción...${NC}"

if [ -f "/usr/lib/qt6/bin/lrelease" ]; then
    LRELEASE_CMD="/usr/lib/qt6/bin/lrelease"
elif command -v lrelease &> /dev/null; then
    LRELEASE_CMD="lrelease"
else
    echo    -e  "${YELLOW}⚠️ lrelease no encontrado, omito la generación de .qm${NC}"
    LRELEASE_CMD="true"
fi

$LRELEASE_CMD resources/i18n/*.ts

# 6. Configurar CMake
echo    -e  "${BLUE}🔧 Configurando proyecto...${NC}"
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -Wno-dev

# 7. Compilar
echo    -e  "${BLUE}🔨 Compilando...${NC}"
if cmake --build "$BUILD_DIR" --parallel; then
    echo    -e  "${GREEN}✅ Compilación exitosa.${NC}"
else
    echo    -e  "${RED}❌ Error durante la compilación.${NC}"
    exit 1
fi

if [ "$INSTALL_SYSTEM" = true ]; then
    echo    -e  "${BLUE}📦 Iniciando instalación en el sistema...${NC}"

    ensure_sudo

    # Instalar Trinity
    if [ -f "$BUILD_DIR/app/trinity" ]; then
        sudo cp -rf "$BUILD_DIR/app/trinity" /usr/local/bin
        echo   "   -> trinity instalado en /usr/local/bin"
    else
        echo    -e  "${RED}❌ Error: No se encontró el binario 'trinity'. Compila primero.${NC}"
        exit 1
    fi



    # Instalar Iconos (Verificando existencia)
    if [ -f "resources/branding/com.trench.trinity.launcher.svg" ]; then
        sudo cp -rf resources/branding/com.trench.trinity.launcher.svg /usr/share/icons/
    else
        echo    -e  "${YELLOW}⚠️  No se encontró el icono (.svg), se omitió su copia.${NC}"
    fi

    # Instalar Acceso Directo (Verificando existencia)
    if [ -f "resources/shortcuts/com.trench.trinity.launcher.desktop" ]; then
        sudo cp -rf resources/shortcuts/com.trench.trinity.launcher.desktop /usr/share/applications/
    else
        echo    -e  "${YELLOW}⚠️  No se encontró el acceso directo (.desktop), se omitió su copia.${NC}"
    fi

    echo    -e  "${GREEN}✅ Instalación completada.${NC}"

    if [ "$RUN_APP" = false ]; then
        echo ""
        echo    -e  "${CYAN}❓ ¿Deseas iniciar Trinity Launcher ahora? (s/n)${NC}"
        read -p "" -n 1 -r REPLY
        echo ""
        if [[ $REPLY =~ ^[SsYy]$ ]]; then
            echo    -e  "${GREEN}🚀 Lanzando en segundo plano...${NC}"
            RUN_APP=true
            DETACHED=true
        fi
    fi
fi

if [ "$RUN_APP" = true ]; then
    
    APP_PATH=""
    # Selección del binario
    if [ -f "$BUILD_DIR/app/trinity" ]; then
        APP_PATH="$BUILD_DIR/app/trinity"
    elif command -v trinity &> /dev/null; then
        APP_PATH=$(command -v trinity)
    else
        echo    -e  "${RED}❌ No se pudo encontrar el ejecutable.${NC}"
        exit 1
    fi

    # --- CASO A: MODO USUARIO (Sin logs, libera terminal) ---
    if [ "$DETACHED" = true ]; then
        echo    -e  "${CYAN}🎮 Lanzando Trinity...${NC}"
        "$APP_PATH" & > /dev/null 2>&1
        echo    -e  "${GREEN}✅ Aplicación iniciada en segundo plano.${NC}"
    
    # --- CASO B: MODO DEV (Logs, Ctrl+C, Espera) ---
    else
        echo    -e  "${CYAN}🎮 Lanzando Trinity (Modo Desarrollo)...${NC}"
        echo    -e  "${YELLOW}⚡ Ejecutando: $APP_PATH${NC}"
        echo    -e  "${YELLOW}ℹ️  Logs en vivo. Presiona Ctrl+C para detener.${NC}"
        echo ""

        cleanup() {
            echo ""
            echo    -e  "${RED}🛑 Deteniendo aplicación...${NC}"
            if [ -n "$APP_PID" ]; then kill "$APP_PID" 2>/dev/null; fi
            exit 0
        }
        trap cleanup SIGINT

        "$APP_PATH" &
        APP_PID=$!
        wait "$APP_PID"
        
        EXIT_CODE=$?
        echo ""
        if [ $EXIT_CODE  q 0 ]; then
            echo    -e  "${GREEN}✅ Aplicación cerrada correctamente.${NC}"
        else
            echo    -e  "${RED}⚠️  Cierre con código: $EXIT_CODE${NC}"
        fi
    fi
fi

echo ""
if [ "$DETACHED" = true ]; then
    # Si fue detached, ya terminamos
    echo    -e  "${GREEN}🎉 ¡Todo listo!${NC}"
else
    # Si fue modo dev, acabamos de cerrar la app
    echo    -e  "${GREEN}🎉 Sesión finalizada.${NC}"
fi

echo ""
echo    -e  "${GREEN}🎉 ¡Todo listo!${NC}"
