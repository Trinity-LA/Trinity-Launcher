# 🎮 Trinity Launcher

> **Entorno gráfico para Minecraft Bedrock en Linux con soporte Flatpak**

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](./LICENSE)
[![Language](https://img.shields.io/badge/language-C%2B%2B-blue.svg)](src/)
[![Qt5](https://img.shields.io/badge/Qt-5-green.svg)](https://www.qt.io/)

Trinity Launcher es un entorno gráfico para ejecutar y gestionar **Minecraft: Bedrock Edition** en Linux, diseñado para funcionar dentro de Flatpak. Incluye dos aplicaciones complementarias escritas en **C++ con Qt5**:


- **Trinchete** 🚀 — Launcher principal que gestiona versiones del juego, permite extraer desde APK y lanza la partida.
- **Trinito** 📦 — Gestor de contenido que instala mods, texturas, packs de desarrollo y mundos.
# 📋 Índice

- [Tecnologías](#-tecnologías)
- [Trinchete — Launcher Multiversión](#trinchete--launcher-multiversión)
- [Trinito — Gestor de Contenido](#trinito--gestor-de-contenido)
- [Compilación](#compilación)
- [Empaquetado en Flatpak](#empaquetado-en-flatpak)
- [Pruebas](#pruebas)
- [Licencia](#licencia)

---

## 🛠️ Tecnologías

### Stack de desarrollo

| Componente | Descripción | Versión |
|:-----------|:-----------|:--------|
| **Lenguaje** | C++ estándar | C++11+ |
| **Framework UI** | Qt Framework para interfaz gráfica | Qt 5.15.11+ |
| **Build System** | Herramienta de compilación | qmake |
| **Compilador** | GCC (g++) para Linux | GCC 14.2+ |
| **Empaquetado** | Contenedor de aplicaciones | Flatpak |
| **Plataforma** | Sistema operativo destino | Linux x86_64 |

### Librerías Qt5 utilizadas

- **QtWidgets** — Componentes de UI (ventanas, botones, cuadros combinados, diálogos)
- **QtGui** — Renderizado gráfico y manejo de eventos
- **QtCore** — Funcionalidades fundamentales (strings, archivos, procesos)

### Herramientas externas

- **mcpelauncher-client** — Cliente para ejecutar Minecraft Bedrock
- **mcpelauncher-extract** — Utilidad para extraer versiones desde APK
- **libevdev** / **libzip** — Dependencias para Flatpak

---

## Trinchete — Launcher Multiversión

### Funcionalidades principales

- **Listado de versiones**: escanea `.../mcpelauncher/versions/` y muestra carpetas en un `QComboBox`.
- **Extracción de APK**: abre un diálogo para seleccionar un `.apk` y darle un nombre (ej. `1.21.0`). Luego ejecuta:
  ```
  mcpelauncher-extract <archivo.apk> <destino>
  ```
- **Validación de integridad**: comprueba que exista `lib/x86_64/libminecraftpe.so` antes de lanzar.
- **Lanzamiento del juego**: ejecuta `mcpelauncher-client -dg <ruta>` y cierra la interfaz.
- **Acceso a herramientas**: botón "Tools" que ejecuta el binario `trinito` desde el mismo directorio (`applicationDirPath()`).

## Trinito — Gestor de Contenido

### Estructura por pestañas (QTabWidget)



| Pestaña      | Tipo de selección | Destino                                      |
|:-------------|:-----------------|:---------------------------------------------|
| Mods         | Archivo           | `behavior_packs/`                            |
| Texturas     | Archivo           | `resource_packs/`                            |
| Desarrollo   | Archivo           | `development_behavior_packs/` y `development_resource_packs/` |
| Mundos       | Carpeta           | `minecraftWorlds/`                           |

### Funcionalidades clave

- **Copia segura**: si ya existe un elemento con el mismo nombre, pregunta antes de reemplazar.
- **Copia recursiva**: para carpetas de mundos, usa una función `copyDirectory()` recursiva.
- **Validación mínima**: asume que el usuario proporciona contenido válido.
- **Rutas portables**: todo basado en `QStandardPaths::GenericDataLocation + "/mcpelauncher/games/com.mojang"`.

## Compilación

Ambas aplicaciones se compilan con el flujo estándar de **Qt + qmake**.

### Compilar Trinchete

```bash
qmake -project -o trinchete.pro
echo "QT += widgets" >> trinchete.pro
qmake trinchete.pro
make
```

### Compilar Trinito

```bash
qmake -project -o trinito.pro
echo "QT += widgets" >> trinito.pro
qmake trinito.pro
make
```

## Estructura esperada del proyecto

```
Trinity/
├── CMakeLists.txt                 # Build system moderno
├── src/
│   ├── core/                      # Lógica de negocio (sin Qt)
│   │   ├── CMakeLists.txt
│   │   ├── version_manager.h
│   │   ├── version_manager.cpp
│   │   ├── pack_installer.h
│   │   ├── pack_installer.cpp
│   │   ├── game_launcher.h
│   │   └── game_launcher.cpp
│   │
│   ├── ui/                        # Interfaz gráfica (con Qt)
│   │   ├── CMakeLists.txt
│   │   ├── windows/
│   │   │   ├── launcher_window.h
│   │   │   ├── launcher_window.cpp
│   │   │   ├── trinito_window.h
│   │   │   └── trinito_window.cpp
│   │   ├── dialogs/
│   │   │   ├── extract_dialog.h
│   │   │   └── extract_dialog.cpp
│   │   └── widgets/
│   │       ├── version_selector.h
│   │       └── version_selector.cpp
│   │
│   └── main.cpp
│
├── tests/
│   ├── CMakeLists.txt
│   ├── test_version_manager.cpp
│   └── test_pack_installer.cpp
│
├── resources/
│   └── resources.qrc
│
└── build/
```

# 📦 Empaquetado en Flatpak

## Requisitos previos

```bash
flatpak install flathub io.qt.qtwebengine.BaseApp//5.15-23.08
flatpak install flathub org.kde.Platform//5.15-23.08 org.kde.Sdk//5.15-23.08
```

## Construcción

```bash
# Generar build y repo
flatpak-builder --user --force-clean build-dir com.trench.trinity.launcher.json
flatpak-builder --repo=repo --force-clean build-dir com.trench.trinity.launcher.json

# Crear paquete
flatpak build-bundle repo trinity.flatpak com.trench.trinity.launcher

# Instalar
flatpak install ./trinity.flatpak
```

> **Nota:** El manifest `com.trench.trinity.launcher.json` debe incluir los módulos de `libevdev`, `libzip` y copiar el directorio `files/` a `/app`.

# 🧪 Pruebas

## Desarrollo local (sin Flatpak)

```bash
make && ./trinchete
make && ./trinito
```

## Dentro de Flatpak

**Launcher principal:**
```bash
flatpak run com.trench.trinity.launcher
```

**Gestor de contenido (desde el botón "Tools" o directamente):**
```bash
flatpak run --command=trinito com.trench.trinity.launcher
```

## Rutas de datos

**En Flatpak:**
```
~/.var/app/com.trench.trinity.launcher/data/mcpelauncher/
```

**En local:**
```
~/.local/share/mcpelauncher/
```
# INSTALACION
``` 
flatpak install flathub io.qt.qtwebengine.BaseApp//5.15-23.08
flatpak install flathub org.kde.Platform//5.15-23.08 org.kde.Sdk//5.15-23.08
wget https://github.com/Trinity-LA/Trinity-Launcher/releases/download/1.0/trinity.flatpak
flatpak install ./trinity.flatpak
``` 
> Ambas apps usan `QStandardPaths`, por lo que **no hay diferencias en el código** entre ambos entornos.


# 📄 Licencia

Este proyecto está bajo licencia BSD. Consulte el archivo LICENSE para los términos completos de uso, modificación y redistribución.
