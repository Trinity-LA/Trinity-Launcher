# Trinity Launcher 

[Official Website](https://trinitylauncher.vercel.app)

[![Version](https://img.shields.io/badge/version-1.0.0-blue)]()
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)]()
[![License](https://img.shields.io/badge/license-BSD--3--Clause-green)]()
<img width="964" height="594" alt="image" src="https://huggingface.co/datasets/JaviercPLUS/servers/resolve/main/Trinity.png" />

**Trinity Launcher** is a modular graphical environment designed to manage and run Minecraft: Bedrock Edition natively on Linux environments.

---

### Key Features
* **Multi-version Management:** Extracts and organizes different versions of the game (APKs).
* **Content Manager (Trinito):** Centralized interface for Mods, Textures, Shaders, and Worlds.
* **Native Integration:** Support for Flatpak and native execution.
* **Discord Integration:** Rich Presence implemented natively.

---

### How does it work?
**Trinity Launcher** is a *frontend* designed to improve the management and usability of **Minecraft: Bedrock Edition** on Linux systems.

> **Special Acknowledgments:** Trinity is built upon the technical foundation of the [mcpelauncher-manifest](https://github.com/minecraft-linux) project.

---

## Installation

Download the latest version from our [Releases](https://github.com/Trinity-LA/Trinity-Launcher/releases) and run the installer.

### Method from source code
If you wish to compile the latest version from the repository:

1. **Clone the project:**
   ```bash
   git clone https://github.com/Trinity-LA/Trinity-Launcher.git
   cd Trinity-Launcher
   ```
2. **Install dependencies and compile:**
   ```bash
   chmod +x build.sh && ./build.sh --deps 
   ```

*(For a detailed guide, refer to [docs/BUILD.md](docs/BUILD.md))*

if you wanna use nix run:
``` 
nix --extra-experimental-features "nix-command flakes" develop
```

---

## Technical Architecture

The project is divided into two main libraries:
- **`TrinityCore`**: File management logic, configuration, and communication with the Bedrock runtime.
- **`TrinityUI`**: User interface based on Qt6.

---

## Contributions
Contributions are welcome! Please read our [Contribution Guide](CONTRIBUTING.md) before opening a *Pull Request*.

---
