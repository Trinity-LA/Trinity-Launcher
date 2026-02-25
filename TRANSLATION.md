# Contributing Translations / Guía de Traducciones

First of all, **thank you for your interest in translating Trinity Launcher!**
¡Primero que nada, **gracias por tu interés en traducir Trinity Launcher!**

## Tools You Need / Herramientas Necesarias
- **Qt Linguist** (Recommended/Recomendado): The easiest tool for `.ts` files. / La herramienta más fácil para archivos `.ts`.
  - Ubuntu/Debian: `sudo apt install qt6-tools-dev-tools`
  - Arch Linux: `sudo pacman -S qt6-tools`
- Or any Text Editor / O cualquier editor de texto (GEANY, nvim, VS Code, Notepad++, etc).

---

## How to Add a New Language / Cómo Añadir un Idioma

### Step 1: Create the File / Paso 1: Crear el Archivo
1. Go to the folder / Ve a la carpeta `resources/i18n/`.
2. Duplicate the file / Duplica el archivo `trinity_es.ts` (Spanish is the base language of the launcher / El español es el idioma base).
3. Rename the copy using your language code / Renómbralo usando el código de tu idioma (ISO 639-1).
   - *Example (English):* `trinity_en.ts`
   - *Example (Portuguese):* `trinity_pt.ts`

### Step 2: Translate / Paso 2: Traducir

**Using Qt Linguist (Super Easy/Fácil):**
1. Open your new `.ts` file in Qt Linguist / Abre tu nuevo `.ts` en Qt Linguist.
2. Select text on the left, write translation on the right / Selecciona el texto a la izquierda, escribe tu traducción a la derecha.
3. Click the checkmark (Done) / Presiona el botón verde de "Hecho" (✓) por cada línea.
4. Save / Guarda el archivo.

**Using a Text Editor / Usando Editor de Texto:**
Find the `<source>` texts and write below them inside `<translation>`:
Busca los textos encerrados en `<source>` y escribe la traducción abajo en `<translation>`:
```xml
<message>
    <source>Launch Game</source>
    <translation>Lancer le jeu</translation>
</message>
```

### Step 3: Register your Language / Paso 3: Registrar tu Idioma
Open / Abre el archivo `resources/resources.qrc`.
Add your language line inside the `<qresource prefix="/i18n">` section:
Añade una línea para tu idioma debajo de la sección `<qresource prefix="/i18n">`:
```xml
<qresource prefix="/i18n">
    <file alias="trinity_es.qm">i18n/trinity_es.qm</file>
    <!-- Add your line here / Añade tu línea aquí: -->
    <file alias="trinity_pl.qm">i18n/trinity_pl.qm</file> 
</qresource>
```

### Step 4: Build / Paso 4: Compilar y Probar
Run the build script to compile the new language and launch the app:
Ejecuta el script para compilar el nuevo idioma y abrir la aplicación:
```bash
./build.sh --release --run
```
Done! You can select it in the top language dropdown. 
¡Listo! Podrás seleccionarlo en la barra de idiomas arriba. Ten en cuenta que debes reiniciar el launcher para ver los cambios completos.

---

## 🔄 Updating Translations / Actualizando Traducciones Existentes
If the source code was updated, you will need to update the translation files to show new texts:
Si el código ha cambiado y hay nuevos textos para traducir, actualiza los archivos así:

```bash
make translations
# or / o usa el script:
./build.sh --update-ts
```
Then open Qt Linguist again, translate the new pending text (marked with `?`), and save!
Luego abre Qt Linguist de nuevo, traduce los nuevos textos pendientes (marcados con `?`), ¡y guarda!

---

## Important Rules / Reglas Importantes
1. **Variables (`%1`, `%2`):** DO NOT translate these. They are replaced by code (like versions or paths).
   NO traduzcas estas variables. Son reemplazadas por el código:
   - Correct: `Delete version %1?` -> `¿Eliminar la versión %1?`
   - Incorrect: `Delete version %1?` -> `¿Eliminar la versión uno?`
2. **Shortcuts (`&`):** The `&` symbol means "Alt + key" shortcut. You can keep it.
   El símbolo `&` se usa para atajos de teclado (ej. `&Archivo` = Alt+A).
3. **Punctuation:** Respect the original punctuation (dots, colons, question marks).
   Respeta y mantén los signos de puntuación originales (puntos, dos puntos, símbolos de interrogación).
