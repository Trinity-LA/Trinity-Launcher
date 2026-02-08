# 🌍 Contributing Translations

First of all, **thank you for your interest in translating Trinity Launcher!**
We want to make Minecraft Bedrock accessible to everyone on Linux, and your contribution helps us reach more users.

> **Note:** We prioritize **quality over quantity**. Please ensure your translations are natural and grammatically correct. **Do not use raw AI outputs** without manual verification.

---

## 🛠️ Tools You Need

- **Text Editor** (VS Code, Notepad++, etc.)
- **Qt Linguist** (Recommended): A specialized tool from Qt to handle translation files easily.
  - On Debian/Ubuntu: `sudo apt install qt6-tools-dev-tools`
  - On Arch: `sudo pacman -S qt6-tools`

---

## 🚀 How to Add a New Language

### Step 1: Create the Translation File

1. Navigate to the `resources/i18n/` folder.
2. Duplicate the file `trinity_en.ts`.
3. Rename the copy using your language code (ISO 639-1).
   - Example for Polish: `trinity_pl.ts`
   - Example for Brazilian Portuguese: `trinity_pt_BR.ts`

### Step 2: Translate

Open your new `.ts` file (e.g., `trinity_pl.ts`) in **Qt Linguist** or a text editor.

- **Using Qt Linguist (Recommended):**
  - Open the file.
  - You will see a list of strings.
  - Type the translation in the target field and mark the item as "Done" (Green checkmark).

- **Using a Text Editor:**
  - Find the `<source>` tags (English text).
  - Add your translation inside the `<translation>` tags.

  **Example:**
  ```xml
  <message>
      <source>Launch Game</source>
      <translation>Lancer le jeu</translation> <!-- French example -->
  </message>
  ```

### Step 3: Register the Language

To make sure the launcher packs your translation, you need to register it in the resource system.

1. Open `resources/resources.qrc`.
2. Locate the `<qresource prefix="/i18n">` section.
3. Add a line for your language pointing to the `.qm` file (the build system generates this automatically from your `.ts` file).

   **Example for adding Polish:**
   ```xml
   <qresource prefix="/i18n">
       <file alias="trinity_en.qm">i18n/trinity_en.qm</file>
       <file alias="trinity_uk.qm">i18n/trinity_uk.qm</file>
       <!-- Add your line here: -->
       <file alias="trinity_pl.qm">i18n/trinity_pl.qm</file> 
   </qresource>
   ```

### Step 4: Build and Test

Run the build script. It will automatically compile your `.ts` file into a `.qm` binary and launch the app.

```bash
./build.sh --release --run
```

Go to **Settings** (or the language dropdown in the top bar) and check if your language appears.

---

## 🔄 Updating Existing Translations

If developers added new features or changed text in the source code, the translation files need to be updated.

1. **Update the .ts files:**
   Run the following command to scan the source code and add new strings to the `.ts` files:
   ```bash
   make translations
   # or
   ./build.sh --update-ts
   ```

2. **Translate new strings:**
   Open your language file in **Qt Linguist**. New strings will be marked with a question mark `?` or listed as "unfinished". Translate them and save.

3. **Submit:**
   Create a Pull Request with your changes.

---

## 💡 Important Translation Rules

1. **Variables (`%1`, `%2`):**
   Do not translate or remove these placeholders. They are replaced by code (e.g., file paths, version numbers).
   - ✅ Correct: `Delete version %1?` -> `¿Eliminar la versión %1?`
   - ❌ Incorrect: `Delete version %1?` -> `¿Eliminar la versión uno?`

2. **Punctuation:**
   Respect the original punctuation (dots, question marks, exclamation marks).

3. **Context:**
   If a word is ambiguous (e.g., "Play"), check where it is used. In this launcher, it usually refers to starting the game.

4. **Shortcuts:**
   If you see an ampersand `&` inside a string (e.g., `&File`), it indicates a keyboard shortcut (Alt+F). You can place the `&` before the letter you want to use as a shortcut in your language, or omit it if not applicable.

---

6. ### Mission completed!

To see the result, follow the building instructions.
  
Remember that every time the language is changed, it is necessary to close and open the launcher again.
