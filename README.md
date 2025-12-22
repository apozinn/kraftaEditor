
---

# ⚡ **Krafta Editor**

<p align="center">
  <img width="180" src="https://raw.githubusercontent.com/apozinn/kraftaEditor/refs/heads/main/assets/images/kraftaEditor.png" alt="Krafta Editor Logo">
</p>

<p align="center">
  <b>A fast, lightweight, and cross-platform code editor built with C++ and wxWidgets.</b><br>
  Designed for developers who value <b>speed, simplicity, and native performance</b>.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C++-20-00599C?style=for-the-badge&logo=cplusplus" alt="C++20">
  <img src="https://img.shields.io/badge/wxWidgets-3.3+-0078D6?style=for-the-badge" alt="wxWidgets 3.3+">
  <img src="https://img.shields.io/badge/Version-0.8.0-orange?style=for-the-badge" alt="Version 0.8.0">
  <img src="https://img.shields.io/badge/License-LGPLv3-blue?style=for-the-badge" alt="LGPLv3 License">
</p>

---

## ✨ **Key Features**

* ⚡ **Blazing fast performance** powered by native C++
* 🧩 **Cross-platform support:** Windows, Linux, macOS
* 🎨 **Modern, customizable UI** built with wxWidgets
* 🧠 **Smart syntax highlighting** for multiple languages
* 🪶 **Lightweight** — no Electron or heavy dependencies
* 🧰 **Productivity-first design** with intuitive workflows

![Krafta Editor Screenshot](https://i.postimg.cc/C122qcM4/image.png)

<p align="center"><em>Clean and fast interface designed for developers.</em></p>

---

## 🚀 **Getting Started**

### Prerequisites

| Platform    | Requirements                 |
| ----------- | ---------------------------- |
| **All**     | CMake ≥ 3.27, C++20 compiler |
| **Linux**   | GTK3, wxWidgets 3.3+         |
| **macOS**   | Xcode Command Line Tools     |
| **Windows** | Visual Studio 2022+          |

> 💡 *Tip:* You can use CMake’s `FetchContent` to automatically download wxWidgets.
> This simplifies setup but increases build time and directory size.

---

## 🧩 **Dependencies Installation**

### Linux (Debian / Ubuntu)

```bash
sudo apt install build-essential cmake libgtk-3-dev libwxgtk3.2-dev
```

### macOS (Homebrew)

```bash
brew install cmake wxwidgets
```

### Windows

1. Install [CMake](https://cmake.org/download/)
2. Install [wxWidgets](https://www.wxwidgets.org/downloads/)
3. Install **Visual Studio 2022+** with C++ workload enabled

---

## ⚙️ **Building from Source**

```bash
# Clone the repository
git clone https://github.com/apozinn/kraftaEditor.git
cd kraftaEditor

# Configure
cmake -S . -B build

# Build (Release)
cmake --build build --config Release

# Run
cmake --build build --target run --config Release
```

> ℹ️ **Build Notes**
>
> * On **Windows (Visual Studio)**, `--config Release` is mandatory.
> * On **Linux and macOS (Make/Ninja)**, `--config Release` is ignored but kept for
>   cross-platform consistency.
> * The executable will be generated at:
>
>   ```
>   build/bin/RELEASE/kraftaEditor
>   ```

---

### Optional CMake Flags

```bash
-DUSE_SYSTEM_WXWIDGETS=ON    # Use system-installed wxWidgets
-DENABLE_TESTS=ON           # Build test suite
-DENABLE_CLANG_TIDY=ON      # Enable clang-tidy static analysis
```

> ℹ️ `-DCMAKE_BUILD_TYPE=Release` is only required when using **single-config generators**
> (Linux/macOS with Make or Ninja).

---

## 🐧 **Linux Installation & Uninstallation**

Krafta Editor provides helper scripts for **system-wide installation** on Linux.

These scripts handle:

* Binary installation
* Assets, languages, config, and i18n files
* Desktop entry creation/removal

> ⚠️ **Root permissions are required**
> Files are installed under `/usr/local` and `/usr/share`.

---

### 📦 Install

Before installing, make sure the project is built and the executable exists at:

```
build/bin/RELEASE/kraftaEditor
```

#### Steps

```bash
chmod +x scripts/install.sh
sudo ./scripts/install.sh
```

#### What the install script does

* Installs the executable to:

  ```
  /usr/local/bin/kraftaEditor
  ```
* Installs shared resources to:

  ```
  /usr/share/kraftaEditor/
  ├── assets/
  ├── languages/
  ├── config/
  └── i18n/
  ```
* Creates a desktop entry:

  ```
  /usr/share/applications/kraftaEditor.desktop
  ```
* Updates the desktop database

After installation, you can:

* Launch **Krafta Editor** from your applications menu
* Or run from the terminal:

  ```bash
  kraftaEditor
  ```

---

### 🗑️ Uninstall

```bash
chmod +x scripts/uninstall.sh
sudo ./scripts/uninstall.sh
```

#### What the uninstall script does

* Removes the executable from:

  ```
  /usr/local/bin/kraftaEditor
  ```
* Deletes all shared application data:

  ```
  /usr/share/kraftaEditor
  ```
* Removes the desktop entry:

  ```
  /usr/share/applications/kraftaEditor.desktop
  ```

After uninstalling, no system files related to Krafta Editor remain.

---

### ℹ️ Notes

* These scripts are intended for **local/system-wide installs**
* They are **not a replacement for package managers**
* For distribution packaging (`.deb`, `.rpm`, Arch PKGBUILD), these scripts can be used as a reference

---

## 🛠️ **Development**

Contributions are welcome!

1. Fork the repository
2. Create a branch:

   ```bash
   git checkout -b feature/new-feature
   ```
3. Commit your changes:

   ```bash
   git commit -am "Add new feature"
   ```
4. Push to your fork:

   ```bash
   git push origin feature/new-feature
   ```
5. Open a Pull Request

### Code Style

* Follow the existing style (4-space indentation)
* Keep commits **clear and focused**
* Update documentation when applicable

---

## 🧾 **Release Highlights — v0.8.0**

### *“Boosting Editor Productivity”*

Version 0.8.0 introduces major improvements to editing fluidity, stability, and UX.

### 🧠 Code Editing Workflow

| Feature                     | Description                                  |
| --------------------------- | -------------------------------------------- |
| Smart HTML Auto-Completion  | Automatically completes and closes HTML tags |
| Select Next Occurrence      | Multi-cursor selection for fast refactoring  |
| Line Duplication Commands   | Duplicate lines or blocks above/below        |
| Optimized Cursor Navigation | Faster and smoother text interaction         |

### 🎨 Customization & UI

| Feature              | Description                                        |
| -------------------- | -------------------------------------------------- |
| Themeable Indicators | Selection and highlight colors are now theme-aware |
| New Language Icons   | JavaScript, XML, and TypeScript icons added        |

### 🧩 Stability & Quality

| Area         | Improvements                                          |
| ------------ | ----------------------------------------------------- |
| Core Editor  | Improved backspace handling and unsaved state display |
| Event System | Fixed `CharAdded` auto-completion trigger             |
| UX           | Fixed misaligned icons and removed unused assets      |

> 🔧 Updating to **v0.8.0** is strongly recommended.

---

## 📜 **License**

Krafta Editor is licensed under the **GNU LGPLv3**.

© 2023–2025 **Okarin Softwares**

---

## 💬 **Support**

* 🐞 [Open an Issue](https://github.com/apozinn/kraftaEditor/issues)
* 💡 Use Discussions for feature ideas

---

## 🗺️ **Roadmap (Preview)**

* [ ] Plugin system
* [ ] Split-view editing
* [ ] Improved search & replace
* [ ] Portable Windows build
* [ ] Theme & syntax editor UI

---

⭐ **If you like Krafta Editor, consider giving it a star — it helps a lot!**

---