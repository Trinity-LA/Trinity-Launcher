
# 🌐 Trinity Launcher — Entorno modular para Minecraft Bedrock en Linux

[![C++](https://img.shields.io/badge/language-C%2B%2B-00599C?logo=c%2B%2B)](https://isocpp.org/)
[![Qt6](https://img.shields.io/badge/Qt-6-41CD52?logo=qt)](https://www.qt.io/)
[![Flatpak](https://img.shields.io/badge/Flatpak-ready-6666FF?logo=flatpak)](https://flatpak.org/)
[![Codeberg](https://img.shields.io/badge/Codeberg-Source-212121?logo=codeberg)](https://codeberg.org)
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

**Trinity Launcher** es un entorno gráfico moderno y modular para ejecutar y gestionar **Minecraft: Bedrock Edition** en Linux. Diseñado para funcionar tanto en sistema como dentro de **Flatpak**, utiliza **Qt6** y sigue una arquitectura limpia basada en librerías separadas (`core` y `ui`).

Incluye dos aplicaciones complementarias:

- `trinchete` → **Launcher principal**: gestión avanzada de versiones, exportación/importación, accesos directos.
- `trinito` → **Gestor de contenido**: instalación, activación/desactivación y eliminación de mods, texturas, packs de desarrollo y mundos.

---

## 🎮 Funcionalidad por Botón

### En `trinchete` (Launcher Principal)

#### Barra superior
- **+ Extraer APK**: selecciona un `.apk`, le da un nombre y lo extrae con `mcpelauncher-extract`.
- **Importar**: restaura una versión guardada en `.tar.gz` (incluye juego y datos de `com.mojang`).
- **Herramientas**: abre la aplicación `trinito`.

#### Panel derecho (al seleccionar una versión)
- **JUGAR**: ejecuta `mcpelauncher-client -dg <ruta>` y cierra el launcher.
- **Crear Acceso Directo**: genera un archivo `.desktop` en `~/Descargas/` para lanzar esta versión vía Flatpak.
- **Editar Configuración**: permite añadir variables de entorno o argumentos (ej: `DRI_PRIME=1`) guardados en `trinity-config.txt`.
- **Exportar**: guarda la versión + sus datos en un archivo comprimido (`.tar.gz`).
- **Eliminar**: borra permanentemente la versión.

### En `trinito` (Gestor de Contenido)

- **Pestañas**: Mods, Texturas, Mundos, Desarrollo.
- **Instalación**: botón para seleccionar archivo (`.zip`, `.mcpack`) o carpeta (solo mundos).
- **Gestión**: lista con checkboxes para **habilitar/deshabilitar** packs (renombrándolos a `.disabled` y comprimiéndolos).
- **Recargar**: actualiza la lista si hay cambios externos.
- **Eliminar**: borra el contenido seleccionado.

---

## ⚙️ Compilación e Instalación de Trinity Launcher

### Requisitos
- CMake 3.17+
- C++17 compatible compiler (GCC o Clang)
- Qt6 (Core, Widgets)

### Pasos
```sh
# Dar permisos de ejecución (solo la primera vez)
chmod +x build.sh

# Compilar e instalar en el sistema
sudo ./build.sh
```

Este comando:
- Compila Trinity Launcher (`trinchete` y `trinito`).
- Instala los binarios en `/usr/local/bin`.
- Copia el icono y el archivo `.desktop` a sus ubicaciones correspondientes.

Una vez instalado, ejecuta desde cualquier terminal:
```sh
trinchete
trinito
```

---

## 🔧 Compilación e Instalación de MCPelauncher

Trinity Launcher **requiere** los binarios de `mcpelauncher-client` , `mcpelauncher-extract` y `mcpelauncher-webview`.

### Recomendación
Usa el fork mantenido en:  
[https://github.com/franckey02/mcpelauncher-patch](https://github.com/franckey02/mcpelauncher-patch)  
(Compatible con versiones recientes de Minecraft, incluyendo 1.21.131+ y betas).

### Instrucciones oficiales
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
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_FLAGS="-march=x86-64 -mtune=generic -msse4.1 -msse4.2 -mpopcnt" \
  -DCMAKE_CXX_FLAGS="-march=x86-64 -mtune=generic -msse4.1 -msse4.2 -mpopcnt" \
  -Wno-dev

make -j$(getconf _NPROCESSORS_ONLN)
sudo make install
```

Esto instala los binarios necesarios en `/usr/local/bin`, donde Trinity Launcher los detecta automáticamente.

> ✅ Este fork incluye parches críticos para versiones **1.21.131+** y soporte para **OpenGL ES 3.0+**.

---

## 📦 Dependencias

### Trinity Launcher:
- CMake
- C++ compiler (GCC o Clang)
- Qt6 (Core, Widgets)

### mcpelauncher-patch:
- pkg-config
- libevdev
- libzip
- Mesa (OpenGL ES 3.0+)
- PulseAudio

---

## 📄 Licencia

Trinity Launcher se distribuye bajo la **Licencia BSD de 3 cláusulas**.

```
Copyright (c) 2024, Trinity Launcher Authors
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
``` 
