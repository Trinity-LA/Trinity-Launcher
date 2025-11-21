# 🤝 Contribuir a Trinity Launcher

Primero que nada, **¡gracias por tu interés en contribuir!** Este es un proyecto en desarrollo activo y todas las contribuciones son bienvenidas.

> **Nota importante:** Trinity Launcher es un proyecto en fase de organización. La prioridad ahora es **documentar, estructurar y reportar**, no optimizar código.

---

## 📋 Antes de empezar

Por favor, lee esto completamente. Tenemos algunas consideraciones importantes para mantener el proyecto en orden:

### 1️⃣ **Commits explícitos y bien documentados**

Aunque no trabajemos de manera profesional, la documentación es clave para encontrar bugs.

**Regla:** Los commits deben ser **claros y específicos**.

```bash
✅ BIEN:
git commit -m "feat: agregar validación de lib en trinchete"
git commit -m "fix: corrección en ruta de datos en Flatpak"
git commit -m "docs: actualizar instrucciones de compilación"

❌ MAL:
git commit -m "cambios"
git commit -m "updates"
git commit -m "arreglado"
```

**Formato recomendado (Conventional Commits):**
```
<tipo>: <descripción corta>

<descripción larga opcional>
```

**Tipos válidos:**
- `feat:` — Nueva funcionalidad
- `fix:` — Corrección de bug
- `docs:` — Documentación
- `refactor:` — Cambio de código sin funcionalidad nueva
- `test:` — Tests
- `chore:` — Tareas de mantenimiento

---

### 2️⃣ **Encontraste un bug?**

**Primero:** Verifica que el software funcione (aunque tenga el bug).

**Regla:** Si el software funciona, **NO ARREGLES EL BUG**, solo reporta.

**Pasos:**

1. Abre un [Issue](../../issues) con:
   - **Título descriptivo:** Ej. "Bug: trinchete no detecta versiones en Flatpak"
   - **Qué esperabas:** Descripción clara de lo que debería pasar
   - **Qué sucede:** Lo que realmente sucede
   - **Pasos para reproducir:** Paso a paso cómo llegar al bug
   - **Sistema:** Distro, versión de Qt, entorno (local/Flatpak)

2. **Etiqueta:** Marca como `bug` (si existe)

3. **Espera a que se organice:** Los maintainers decidirán cuándo y cómo arreglarlo

**Ejemplo de issue bien hecho:**
```
**Título:** Bug: trinito no copia carpetas de mods en Flatpak

**Qué esperaba:** Que al seleccionar una carpeta de mods, se copie a `behavior_packs/`

**Qué sucede:** La carpeta no se copia, sin mensajes de error

**Pasos:**
1. Ejecutar: `flatpak run com.trench.trinity.launcher`
2. Click en "Tools"
3. Ir a pestaña "Mods"
4. Seleccionar una carpeta
5. Ver que nada sucede

**Sistema:** Ubuntu 22.04, Qt 5.15.11, Flatpak
```

---

### 3️⃣ **Tienes una idea de mejora?**

**Regla:** Documenta la idea, **NO toques el código** hasta que se organice el proyecto.

**Pasos:**

1. Abre un [Discussion](../../discussions) o [Issue](../../issues) con etiqueta `enhancement`
2. Describe:
   - Qué mejora propones
   - Por qué sería útil
   - Impacto esperado
3. **Espera feedback** de los maintainers antes de hacer cambios

**Prioridad actual:** Organizar proyecto > Optimizar código

---

### 4️⃣ **Vas a usar IA para apoyarte?**

**Regla importante:** Evita que la IA modifique código innecesariamente.

**Permite:**
✅ Usar IA para entender el código  
✅ Usar IA para escribir documentación  
✅ Usar IA para diseñar tests  
✅ Usar IA para proponer soluciones en issues  

**NO permitas:**
❌ Que IA optimice código por su cuenta  
❌ Que IA refactorice sin documentación previa  
❌ Que IA haga cambios "mejorando" el código  

**Razón:** Los cambios sin documentación pueden romper cosas que no hemos documentado todavía. La estabilidad es más importante que la optimización ahora.

---

## 🏗️ Estándares de código

### Nombrado de identificadores

| Elemento | Estilo | Ejemplo |
|:---------|:-------|:--------|
| **Funciones/Métodos** | `camelCase` | `loadInstalledVersions()`, `launchGame()` |
| **Variables** | `camelCase` | `selectedVersion`, `libPath` |
| **Constantes** | `UPPER_SNAKE_CASE` | `MAX_RETRIES`, `DEFAULT_TIMEOUT` |
| **Clases** | `PascalCase` | `LauncherWindow`, `VersionManager` |
| **Carpetas** | `lowercase` | `src/`, `ui/`, `core/` |
| **Archivos** | `snake_case` | `launcher_window.cpp`, `version_manager.h` |

### Ejemplo de estructura

```
src/
├── core/
│   ├── version_manager.h
│   ├── version_manager.cpp
│   ├── pack_installer.h
│   └── pack_installer.cpp
│
├── ui/
│   ├── windows/
│   │   ├── launcher_window.h
│   │   └── launcher_window.cpp
│   ├── dialogs/
│   │   ├── extract_dialog.h
│   │   └── extract_dialog.cpp
```

### Documentación de código

- Documenta **funciones públicas** siempre
- Usa comentarios claros en lógica compleja
- Sigue el estilo existente en los archivos

**Ejemplo:**
```cpp
/**
 * Carga las versiones instaladas desde el directorio mcpelauncher
 * 
 * @return QStringList con los nombres de las versiones encontradas
 */
QStringList loadInstalledVersions();
```

---

## 🔄 Flujo de contribución

### 1. Preparar tu entorno

```bash
# Clonar el repo
git clone https://github.com/0rt4/Trinity_Launcher.git
cd Trinity_Launcher

# Crear una rama
git checkout -b feature/tu-cambio
# o
git checkout -b fix/tu-bug
```

### 2. Hacer cambios

- Sigue los estándares de código
- Escribe commits claros y explícitos
- Si usas IA, evita cambios innecesarios en el código

### 3. Compilar y probar localmente

```bash
# Compilar trinchete
qmake -project -o trinchete.pro
echo "QT += widgets" >> trinchete.pro
qmake trinchete.pro
make

# Compilar trinito
qmake -project -o trinito.pro
echo "QT += widgets" >> trinito.pro
qmake trinito.pro
make

# Ejecutar
./trinchete
./trinito
```

### 4. Abrir un Pull Request

**Antes de PR:**
- ✅ Compila sin errores
- ✅ Probaste localmente
- ✅ Commits son claros
- ✅ Actualizaste documentación si es necesario

**En la PR:**
- **Título:** Describe brevemente el cambio
- **Descripción:** Explica qué cambió y por qué
- **Referencias:** Vincula issues relacionados con `Fixes #123`

**Ejemplo:**
```markdown
# Agregado: Validación de integridad en trinchete

## Descripción
Se agregó validación de que `libminecraftpe.so` existe antes de lanzar el juego.
Esto previene crashes cuando la versión está incompleta.

## Cambios
- Agregado método `validateGameVersion()` en `launcher_window.cpp`
- Actualizado flujo de lanzamiento
- Agregado mensaje de error clara al usuario

## Testing
- Probado localmente en Ubuntu 22.04
- Validado con versiones completas e incompletas

Fixes #42
```

---

## 📞 Preguntas o dudas?

- **Sobre código:** Abre una [Discussion](../../discussions)
- **Encontraste un bug:** Reporta en [Issues](../../issues)
- **Idea de mejora:** Discussion o Issue con etiqueta `enhancement`

---

## ✨ Gracias por contribuir

Trinity Launcher crece gracias a gente como tú. Tu documentación, reportes y propuestas son invaluables para llevar el proyecto al siguiente nivel.

**¡Bienvenido al equipo!** 🚀
