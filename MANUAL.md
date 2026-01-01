# 🌐 Trinity Launcher — Entorno modular para Minecraft Bedrock en Linux

[![C++](https://img.shields.io/badge/language-C%2B%2B-00599C?logo=c%2B%2B)](https://isocpp.org/)
[![Qt6](https://img.shields.io/badge/Qt-6-41CD52?logo=qt)](https://www.qt.io/)
[![Flatpak](https://img.shields.io/badge/Flatpak-ready-6666FF?logo=flatpak)](https://flatpak.org/)
[![Codeberg](https://img.shields.io/badge/Codeberg-Source-212121?logo=codeberg)](https://codeberg.org)
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

**Trinity Launcher** es un entorno gráfico moderno y modular para ejecutar y gestionar **Minecraft: Bedrock Edition** en Linux. Diseñado para funcionar dentro de **Flatpak**, utiliza **Qt6** y sigue una arquitectura limpia basada en librerías separadas (`core` y `ui`).

Incluye dos aplicaciones complementarias:

- `trinchete` → **Launcher principal**: gestión avanzada de versiones, exportación/importación, accesos directos.
- `trinito` → **Gestor de contenido**: instalación, activación/desactivación y eliminación de mods, texturas, packs de desarrollo y mundos.

---

## 🧱 Arquitectura del Código

El proyecto está estructurado en módulos con **CMake**:

- **`TrinityCore`**: lógica de negocio (gestión de versiones, packs, lanzamiento, exportación).
- **`TrinityUI`**: interfaces gráficas (ventanas, diálogos, widgets).
- **`app/`**: puntos de entrada (`main.cpp`, `trinito_main.cpp`).

Esta separación facilita mantenibilidad, reutilización y futuras extensiones.

---

## 🎮 `trinchete` — Funcionalidad por Botón

### 🔝 Barra Superior
| Botón             | Función |
|-------------------|--------|
| **+ Extraer APK** | Selecciona un `.apk`, le asigna un nombre y lo extrae con `mcpelauncher-extract` en segundo plano. |
| **Importar**      | Restaura una versión previamente exportada (`.tar.gz`), incluyendo tanto el juego como los datos de `com.mojang`. |
| **Herramientas**  | Lanza la aplicación `trinito` desde el mismo directorio del ejecutable. |

### 🖱️ Panel Derecho (versión seleccionada)
| Botón                     | Función |
|--------------------------|--------|
| **JUGAR**                | Ejecuta `mcpelauncher-client -dg <ruta>` y cierra el launcher. |
| **Crear Acceso Directo** | Genera un archivo `.desktop` en la carpeta de Descargas para lanzar esta versión vía Flatpak. |
| **Editar Configuración** | Permite añadir variables de entorno o argumentos personalizados (ej: `DRI_PRIME=1`) guardados en `trinity-config.txt`. |
| **Exportar**             | Empaqueta la versión seleccionada **+ sus datos de com.mojang** en un archivo `.tar.gz`. |
| **Eliminar**             | Borra permanentemente la versión del disco. |

> ✅ **Barra de estado**: muestra ruta y tamaño estimado de la versión seleccionada.

---

## 🧰 `trinito` — Funcionalidad por Pestaña

### 📦 Pestañas: **Mods**, **Texturas**, **Mundos**
| Componente               | Función |
|--------------------------|--------|
| **Seleccionar archivo**  | Instala un archivo (`.zip`, `.mcpack`) en la carpeta correspondiente. |
| **Lista con checkboxes** | Muestra packs instalados. ✅ = habilitado, ⬜ = deshabilitado (renombrado a `.disabled` y comprimido). |
| **Recargar Lista**       | Actualiza la vista si se modificaron archivos externamente. |
| **Eliminar Seleccionado**| Borra permanentemente el pack o mundo seleccionado. |

### ⚙️ Pestaña: **Desarrollo**
- Dos columnas independientes: **Development Behavior Packs** y **Development Resource Packs**.
- Mismas funciones: instalación, recarga y eliminación.
- Ideal para creadores que usan carpetas de desarrollo.

> 💡 **Activación/desactivación**:  
> - **Habilitar**: descomprime `.disabled` → nombre original.  
> - **Deshabilitar**: comprime el pack → añade extensión `.disabled`.

---

## ⚙️ Compilación

### Requisitos
- CMake 3.17+
- C++17 compatible compiler
- Qt6 (Core, Widgets, Concurrent)

### Proceso
```sh
chmod +x build.sh
./build.sh --release
```

El script:
- Genera binarios en `build/app/`.
- Soporta `--debug` y `--clean`.
- Instala opcionalmente en `/usr/local/bin` (requiere `sudo`).

---

## 📦 Dependencias

- **CMake**
- **GCC** o **Clang**
- **Qt6Base**
- **Qt6Declarative** (opcional, si se usa QML en el futuro)
- **pkg-config**
- **libevdev**
- **libzip**
- **mesa-libGL** / **OpenGL ES**
- **pulseaudio** (para audio en Linux)

> ✅ Para ejecutar Minecraft, también necesitas:
> - [`mcpelauncher-client`](https://github.com/franckey02/mcpelauncher-patch)
> - [`mcpelauncher-extract`](https://github.com/franckey02/mcpelauncher-patch)

---

## 🔗 MCPelauncher Recomendado

Se recomienda usar el fork mantenido en:  
👉 [https://github.com/franckey02/mcpelauncher-patch](https://github.com/franckey02/mcpelauncher-patch)

Este fork soporta **Minecraft 1.21.131+ y versiones beta**, y corrige problemas críticos en versiones recientes.

Compílalo y coloca los binarios (`mcpelauncher-client`, `mcpelauncher-extract`) en el mismo directorio que `trinchete` y `trinito`.

---

## 📦 Empaquetado en Flatpak

- Usa `io.qt.qtwebengine.BaseApp//6.6` y `org.kde.Platform//6.6`.
- Copia la carpeta `files/` (con `bin/` y `share/`) al interior del bundle.
- Incluye permisos para `xdg-data/mcpelauncher:rw`.

---

## 🧪 Pruebas

- **Local**: `./build/app/trinchete`, `./build/app/trinito`
- **Flatpak**:  
  ```sh
  flatpak run com.trench.trinity.launcher
  flatpak run --command=trinito com.trench.trinity.launcher
  ```

Las rutas de datos usan `QStandardPaths::GenericDataLocation`, por lo que son compatibles en ambos entornos.

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
