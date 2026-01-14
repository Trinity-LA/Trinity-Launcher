# ==========================================
# 🛠️ Trinity Launcher Build Script
# ==========================================

# Configuración de seguridad: detener si hay errores
set -e

# Colores para la terminal
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Variables por defecto
BUILD_TYPE="Release"
CLEAN_BUILD=false
UPDATE_TRANSLATIONS=false
BUILD_DIR="build"

# Función de ayuda
show_help() {
    echo -e "${BLUE}Uso: ./build.sh [OPCIONES]${NC}"
    echo ""
    echo "Opciones:"
    echo "  --debug      Compila en modo Debug (con símbolos para depurar)"
    echo "  --release    Compila en modo Release (optimizado, por defecto)"
    echo "  --clean      Borra la carpeta build/ y compila desde cero"
    echo "  --update-ts  Escanea el código y actualiza los archivos .ts de traducción"
    echo "  --help       Muestra esta ayuda"
    echo ""
}

# 1. Procesar argumentos
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
        --update-ts)
            UPDATE_TRANSLATIONS=true
            shift ;;
        --help)
            show_help
            exit 0 ;;
        *)
            echo -e "${RED}Error: Opción desconocida $1${NC}"
            show_help
            exit 1 ;;
    esac
done

# 2. Actualizar Traducciones (Si se solicita)
if [ "$UPDATE_TRANSLATIONS" = true ]; then
    echo -e "${YELLOW}🌍 Actualizando archivos de traducción (.ts)...${NC}"
    
    # 1. Intentar encontrar lupdate en el PATH normal
    if command -v lupdate &> /dev/null; then
        LUPDATE_CMD="lupdate"
    # 2. Si falla, intentar encontrarlo en la ruta específica de Arch Linux / Qt6
    elif [ -f "/usr/lib/qt6/bin/lupdate" ]; then
        LUPDATE_CMD="/usr/lib/qt6/bin/lupdate"
    else
        echo -e "${RED}Error: 'lupdate' no encontrado. Instala 'qt6-tools' (Arch) o 'qt6-tools-dev' (Debian).${NC}"
        exit 1
    fi

    echo -e "${BLUE}   Usando: $LUPDATE_CMD${NC}"

    # Ejecutar lupdate usando la variable que encontramos
    $LUPDATE_CMD src/ include/ -recursive -ts resources/i18n/trinity_en.ts resources/i18n/trinity_ca.ts
    
    echo -e "${GREEN}✅ Archivos .ts actualizados.${NC}"
  # Se termina acá porque solo actualizará los archivos .ts
  exit 0
fi

# 3. Verificar entorno
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: No se encuentra CMakeLists.txt. Ejecuta este script desde la raíz del proyecto.${NC}"
    exit 1
fi

echo -e "${BLUE}=== Iniciando proceso de construcción ($BUILD_TYPE) ===${NC}"

# 4. Limpieza (si se solicita)
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}🧹 Limpiando compilaciones anteriores (--clean)...${NC}"
    rm -rf "$BUILD_DIR"
    rm -f trinchete trinito
fi

# 5. Crear directorio build si no existe
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

# 6. Configurar CMake
echo -e "${BLUE}🔧 Configurando proyecto...${NC}"
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -Wno-dev

# 7. Compilar
echo -e "${BLUE}🔨 Compilando...${NC}"
if cmake --build "$BUILD_DIR" --parallel; then
    echo -e "${GREEN}✅ Compilación exitosa.${NC}"
else
    echo -e "${RED}❌ Error durante la compilación.${NC}"
    exit 1
fi

# 8. Mover binarios (Opcional, por comodidad)
echo -e "${BLUE}📦 Organizando ejecutables...${NC}"

# Verificar si los binarios existen antes de moverlos
if [ -f "$BUILD_DIR/app/trinchete" ]; then
    sudo cp -rf "$BUILD_DIR/app/trinchete" /usr/local/bin
    echo -e "   -> trinchete listo"
fi

if [ -f "$BUILD_DIR/app/trinito" ]; then
    sudo cp "$BUILD_DIR/app/trinito" /usr/local/bin
    echo -e "   -> trinito listo"
fi

# Copiamos los iconos del launcher
sudo cp -rf resources/branding/com.trench.trinity.launcher.svg /usr/share/icons/
sudo cp -rf resources/shortcuts/com.trench.trinity.launcher.desktop /usr/share/applications/

echo ""
echo -e "${GREEN}🎉 ¡Todo listo!${NC}"
