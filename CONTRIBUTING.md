# 🤝 Contributing to Trinity Launcher

First of all, **thank you for your interest in contributing!** This is an actively developing project and all contributions are welcome.

> **Important note:** Trinity Launcher is in an organizational phase. The priority right now is to **document, structure, and report**, not to optimize code.

---

## 📋 Before starting

Please read this completely. We have some important considerations to keep the project organized:

### 1️⃣ **Explicit and well-documented commits**

Even if we don't work professionally, documentation is key to finding bugs.

**Rule:** Commits must be **clear and specific**.

```bash
✅ GOOD:
git commit -m "feat: add lib validation in trinity"
git commit -m "fix: correction in data path in Flatpak"
git commit -m "docs: update build instructions"

❌ BAD:
git commit -m "changes"
git commit -m "updates"
git commit -m "fixed"
```

**Recommended format (Conventional Commits):**
```
<type>: <short description>

<optional long description>
```

**Valid types:**
- `feat:` — New functionality
- `fix:` — Bug fix
- `docs:` — Documentation
- `refactor:` — Code change without new functionality
- `test:` — Tests
- `chore:` — Maintenance tasks

---

### 2️⃣ **Found a bug?**

**First:** Verify that the software runs (even with the bug).

**Rule:** If the software runs, **DO NOT FIX THE BUG**, just report it.

**Steps:**

1. Open an [Issue](../../issues) with:
   - **Descriptive title:** E.g. "Bug: trinchete does not detect versions in Flatpak"
   - **What you expected:** Clear description of what should happen
   - **What happens:** What actually happens
   - **Steps to reproduce:** Step-by-step how to reach the bug
   - **System:** Distro, Qt version, environment (local/Flatpak)

2. **Label:** Mark as `bug` (if it exists)

3. **Wait for organization:** Maintainers will decide when and how to fix it

**Example of a well-made issue:**
```
**Title:** Bug: trinito does not copy mod folders in Flatpak

**What I expected:** That when selecting a mod folder, it copies to `behavior_packs/`

**What happens:** The folder is not copied, with no error messages

**Steps:**
1. Run: `flatpak run com.trench.trinity.launcher`
2. Click on "Tools"
3. Go to "Mods" tab
4. Select a folder
5. See that nothing happens

**System:** Ubuntu 22.04, Qt 5.15.11, Flatpak
```

---

### 3️⃣ **Have an improvement idea?**

**Rule:** Document the idea, **DO NOT touch the code** until the project is organized.

**Steps:**

1. Open a [Discussion](../../discussions) or [Issue](../../issues) with the `enhancement` label
2. Describe:
   - What improvement you propose
   - Why it would be useful
   - Expected impact
3. **Wait for feedback** from maintainers before making changes

**Current priority:** Organize project > Optimize code

---

### 4️⃣ **Are you going to use AI for support?**

**Important rule:** Avoid letting AI modify code unnecessarily.

**Allow:**
✅ Using AI to understand code  
✅ Using AI to write documentation  
✅ Using AI to design tests  
✅ Using AI to propose solutions in issues  

**Do NOT allow:**
❌ AI optimizing code on its own  
❌ AI refactoring without prior documentation  
❌ AI making changes "improving" the code  

**Reason:** Undocumented changes can break things we haven't documented yet. Stability is more important than optimization right now.

---

## 🏗️ Code Standards

### Identifier Naming

| Element | Style | Example |
|:---------|:-------|:--------|
| **Functions/Methods** | `camelCase` | `loadInstalledVersions()`, `launchGame()` |
| **Variables** | `camelCase` | `selectedVersion`, `libPath` |
| **Constants** | `UPPER_SNAKE_CASE` | `MAX_RETRIES`, `DEFAULT_TIMEOUT` |
| **Classes** | `PascalCase` | `LauncherWindow`, `VersionManager` |
| **Folders** | `lowercase` | `src/`, `ui/`, `core/` |
| **Files** | `snake_case` | `launcher_window.cpp`, `version_manager.h` |

### Structure Example

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

### Code Documentation

- Always document **public functions**
- Use clear comments in complex logic
- Follow the existing style in files

**Example:**
```cpp
/**
 * Loads installed versions from the mcpelauncher directory
 * 
 * @return QStringList with the names of found versions
 */
QStringList loadInstalledVersions();
```

---

## 🔄 Contribution Flow

### 1. Prepare your environment

```bash
# Clone the repo
git clone https://github.com/Trinity-LA/Trinity-Launcher.git
cd Trinity_Launcher

# Create a branch
git switch -c feature/your-change
# or
git switch -c fix/your-bug
```

### 2. Make changes

- Follow code standards
- Write clear and explicit commits
- If using AI, avoid unnecessary code changes

### 3. Compile and test locally

```bash
# Compile trinity
qmake -project -o trinity.pro
echo "QT += widgets" >> trinity.pro
qmake trinity.pro
make

# Execute
./trinity
```

### 4. Open a Pull Request

**Before PR:**
- ✅ Compiles without errors
- ✅ Tested locally
- ✅ Commits are clear
- ✅ Updated documentation if necessary

**In the PR:**
- **Title:** Briefly describe the change
- **Description:** Explain what changed and why
- **References:** Link related issues with `Fixes #123`

**Example:**
```markdown
# Added: Integrity validation in trinchete

## Description
Added validation that `libminecraftpe.so` exists before launching the game.
This prevents crashes when the version is incomplete.

## Changes
- Added `validateGameVersion()` method in `launcher_window.cpp`
- Updated launch flow
- Added clear error message to user

## Testing
- Tested locally on Ubuntu 22.04
- Validated with complete and incomplete versions

Fixes #42
```

---

## 📞 Questions or doubts?

- **About code:** Open a [Discussion](../../discussions)
- **Found a bug:** Report in [Issues](../../issues)
- **Improvement idea:** Discussion or Issue with `enhancement` label

---

## ✨ Thank you for contributing

Trinity Launcher grows thanks to people like you. Your documentation, reports, and proposals are invaluable to take the project to the next level.

**Welcome to the team!** 🚀
