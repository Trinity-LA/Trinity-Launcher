#!/bin/bash

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
BUILD_DIR="build"

# Función de ayuda
show_help() {
    echo -e "${BLUE}Uso: ./build.sh [OPCIONES]${NC}"
    echo ""
    echo "Opciones:"
    echo "  --debug    Compila en modo Debug (con símbolos para depurar)"
    echo "  --release  Compila en modo Release (optimizado, por defecto)"
    echo "  --clean    Borra la carpeta build/ y compila desde cero"
    echo "  --help     Muestra esta ayuda"
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
        --help)
            show_help
            exit 0 ;;
        *)
            echo -e "${RED}Error: Opción desconocida $1${NC}"
            show_help
            exit 1 ;;
    esac
done

# 2. Verificar entorno
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: No se encuentra CMakeLists.txt. Ejecuta este script desde la raíz del proyecto.${NC}"
    exit 1
fi

echo -e "${BLUE}=== Iniciando proceso de construcción ($BUILD_TYPE) ===${NC}"

# 3. Limpieza (si se solicita)
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}🧹 Limpiando compilaciones anteriores (--clean)...${NC}"
    rm -rf "$BUILD_DIR"
    rm -f trinchete trinito
fi

# 4. Crear directorio build si no existe
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

# 5. Configurar CMake
# Usamos -S . (source) y -B build (build dir) que es la sintaxis moderna
echo -e "${BLUE}🔧 Configurando proyecto...${NC}"
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -Wno-dev

# 6. Compilar
# --parallel usa automáticamente todos los núcleos disponibles
echo -e "${BLUE}🔨 Compilando...${NC}"
if cmake --build "$BUILD_DIR" --parallel; then
    echo -e "${GREEN}✅ Compilación exitosa.${NC}"
else
    echo -e "${RED}❌ Error durante la compilación.${NC}"
    exit 1
fi

# 7. Mover binarios (Opcional, por comodidad)
echo -e "${BLUE}📦 Organizando ejecutables...${NC}"

# Verificar si los binarios existen antes de moverlos
if [ -f "$BUILD_DIR/app/trinchete" ]; then
    cp "$BUILD_DIR/app/trinchete" .
    echo -e "   -> trinchete listo"
fi

if [ -f "$BUILD_DIR/app/trinito" ]; then
    cp "$BUILD_DIR/app/trinito" .
    echo -e "   -> trinito listo"
fi

echo ""
echo -e "${GREEN}🎉 ¡Todo listo!${NC}"
echo -e "Ejecuta: ${YELLOW}./trinchete${NC} o ${YELLOW}./trinito${NC}"
