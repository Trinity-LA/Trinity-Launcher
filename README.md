
# 🎮 Trinity Launcher

[English](README_eng.md)

Para instalar y conocer los pasos detallados, visita nuestra [página web oficial](https://trinitylauncher.vercel.app)

> **Entorno gráfico modular para Minecraft Bedrock en Linux**

[![License](https://img.shields.io/badge/license-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Qt](https://img.shields.io/badge/Qt-6-41CD52?logo=qt)](https://www.qt.io/)
[![GitHub](https://img.shields.io/badge/GitHub-Source-181717?logo=github)](https)
[![Flatpak](https://img.shields.io/badge/Flatpak-ready-6666FF?logo=flatpak)](https://flatpak.org/)

**Trinity Launcher** es un entorno gráfico moderno, modular y funcional para ejecutar y gestionar **Minecraft: Bedrock Edition** en Linux. Diseñado para funcionar tanto en sistema como dentro de **Flatpak**, utiliza **Qt6** y sigue una arquitectura limpia basada en librerías separadas (`core` y `ui`).

Incluye dos aplicaciones complementarias:

- **`Trinchete`** 🚀 → **Launcher principal**: gestión avanzada de versiones, exportación/importación, accesos directos.
- **`Trinito`** 📦 → **Gestor de contenido**: instalación, activación/desactivación y eliminación de mods, texturas, packs de desarrollo y mundos.

---

## 📋 Índice

- [Tecnologías](#-tecnologías)
- [Trinchete — Launcher Multiversión](#trinchete--launcher-multiversión)
- [Trinito — Gestor de Contenido](#trinito--gestor-de-contenido)
- [Compilación e Instalación](#compilación-e-instalación)
- [MCPelauncher Requerido](#mcpelauncher-requerido)
- [Empaquetado en Flatpak](#empaquetado-en-flatpak)
- [Pruebas](#pruebas)
- [Licencia](#licencia)

---

## 🛠️ Tecnologías

### Stack de desarrollo

| Componente        | Descripción                              | Versión        |
|-------------------|------------------------------------------|----------------|
| **Lenguaje**      | C++ estándar                             | C++17          |
| **Framework UI**  | Qt Framework                             | Qt 6.6+        |
| **Build System**  | CMake                                    | 3.17+          |
| **Compilador**    | Clang                                    | 16+            |
| **Empaquetado**   | Flatpak                                  |                |
| **Plataforma**    | Linux (x86_64, ARM64)                    | glibc          |

### Arquitectura modular

- **`TrinityCore`**: lógica de negocio (gestión de versiones, packs, lanzamiento, exportación).
- **`TrinityUI`**: interfaces gráficas (ventanas, diálogos, widgets).
- **Separación clara**: facilita mantenimiento, pruebas y reutilización.

---

## 🎮 Trinchete — Interfaz del launcher

### Barra superior
- **+ Extraer APK**: selecciona un `.apk`, le da un nombre y lo extrae con `mcpelauncher-extract`.
- **Importar**: restaura una versión guardada en `.tar.gz` (incluye juego y datos de `com.mojang`).
- **Herramientas**: abre la aplicación `trinito`.

### Panel derecho (versión seleccionada)
- **JUGAR**: ejecuta `mcpelauncher-client -dg <ruta>` y cierra el launcher.
- **Crear Acceso Directo**: genera un archivo `.desktop` en `~/Descargas/` para lanzar esta versión vía Flatpak.
- **Editar Configuración**: permite añadir variables de entorno o argumentos (ej: `DRI_PRIME=1`) guardados en `trinity-config.txt`.
- **Exportar**: guarda la versión + sus datos en un archivo comprimido (`.tar.gz`).
- **Eliminar**: borra permanentemente la versión.

> ✅ **Interfaz moderna con tema oscuro y soporte para íconos**.

---

## 🧰 Trinito — Gestor de Contenido

### Pestañas

| Pestaña      | Tipo de selección | Destino                                      |
|--------------|-------------------|---------------------------------------------|
| **Mods**     | Archivo           | `behavior_packs/`                           |
| **Texturas** | Archivo           | `resource_packs/`                           |
| **Mundos**   | **Carpeta**       | `minecraftWorlds/`                          |
| **Desarrollo**| Archivo           | `development_behavior_packs/` y `development_resource_packs/` |

### Funcionalidades clave
- **Instalación**: botón para seleccionar archivo o carpeta.
- **Activación/Deshabilitación**:  
  - ✅ **Habilitado**: nombre normal.  
  - ⬜ **Deshabilitado**: renombrado a `.disabled` y comprimido con `tar`.
- **Gestión**: checkboxes interactivos, recarga y eliminación.
- **Validación segura**: pregunta antes de reemplazar o eliminar.

---

## ⚙️ Compilación e Instalación

Puedes seguir nuestro manual detallado en: [https://trinity-la.github.io/](https://trinity-la.github.io/)

### Requisitos Previos

El sistema instalará automáticamente las dependencias, pero necesitarás tener `make` y `git` instalados inicialmente.
Depndencias usadas:
- CMake 3.17+
- Clang
- Qt6 (Core, Widgets)

### 🚀 Método Rápido (Recomendado)

```sh
# 1. Primera vez (Instala dependencias, compila y ejecuta)
make start

# 2. Uso diario (Solo compila y ejecuta localmente, para devs)
make run

# 3. Instalar en el sistema (Opcional, requiere sudo)
make install

```

### 🛠️ Método Manual (Script)

Si prefieres usar el script directamente para mayor control:

```sh
# Dar permisos
chmod +x build.sh

# Instalar dependencias y compilar
./build.sh --deps --release

# Compilar y ejecutar
./build.sh --release --run

```

> 📦 Instala `trinchete` y `trinito` en `/usr/local/bin`, y registra el `.desktop` y el icono en el sistema.

Una vez instalado, ejecuta desde cualquier terminal:
```sh
trinchete
trinito
```

### 📦 Comandos disponibles

| Comando | Descripción |
| --- | --- |
| `make start` | Setup completo: Dependencias + Build + Run. |
| `make run` | Compila y ejecuta la versión local (dev). |
| `make install` | Instala Trinity en `/usr/local/bin` para todo el sistema. |
| `make clean` | Limpia la carpeta de compilación y arregla permisos. |
| `make uninstall` | Elimina Trinity completamente del sistema. |
---

## 🔧 Mcpelauncher Requerido

Trinity Launcher **requiere** los binarios de `mcpelauncher-client`, `mcpelauncher-extract` y `mcpelauncher-webview`.

### Recomendación
Usa el fork mantenido en:  
[https://github.com/franckey02/mcpelauncher-patch](https://github.com/franckey02/mcpelauncher-patch)  
(Compatible con versiones recientes de Minecraft, incluyendo **1.21.131+** y betas).

### Instrucciones 
**Dependencias:**
- Compilador C/C++ (gcc, g++)
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

Nota: si usas el dockerfile te ahorras el instalar tantas dependencias

**Comandos para compilar**
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

## 📦 Empaquetado en Flatpak

### Requisitos
```sh
flatpak install flathub io.qt.qtwebengine.BaseApp//6.9
flatpak install flathub org.kde.Platform//6.9 org.kde.Sdk//6.9
```

### Construcción
```sh
flatpak-builder --user --force-clean build-dir com.trench.trinity.launcher.json
flatpak-builder --repo=repo --force-clean build-dir com.trench.trinity.launcher.json
flatpak build-bundle repo trinity.flatpak com.trench.trinity.launcher
flatpak install ./trinity.flatpak
```

> ✅ El manifest debe incluir `libevdev`, `libzip` y copiar `files/` a `/app`.

---

## 🧪 Pruebas

### En sistema
```sh
trinchete
trinito
```

### En Flatpak
```sh
flatpak run com.trench.trinity.launcher
flatpak run --command=trinito com.trench.trinity.launcher
```

### Rutas de datos (automáticas)
- **Flatpak**: `~/.var/app/com.trench.trinity.launcher/data/mcpelauncher/`
- **Local**: `~/.local/share/mcpelauncher/`

> Ambas usan `QStandardPaths::GenericDataLocation` → **total compatibilidad**.

---

## 📄 Licencia

Trinity Launcher se distribuye bajo la **Licencia BSD de 3 cláusulas**.
