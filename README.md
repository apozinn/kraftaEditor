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

![Krafta Editor Dark theme Screenshot](https://i.imgur.com/M0sKSg9.png)
![Krafta Editor Light theme Screenshot](https://i.imgur.com/mtKpEO3.png)

<p align="center"><em>Clean and fast interface designed for developers.</em></p>

---

## ⚡ Performance Comparison

### 🚀 Startup Time

| Editor             | Startup time           |
| ------------------ | ---------------------- |
| **Krafta Editor**  | **Instant (< 200 ms)** |
| Visual Studio Code | ~2–4 seconds           |

---

### 🧠 Memory Usage (Idle)

| Editor             | RAM usage     |
| ------------------ | ------------- |
| **Krafta Editor**  | **~60–70 MB** |
| Visual Studio Code | ~300–500 MB   |

---

### 🖥️ Tech Stack

|          | Krafta Editor | VS Code                 |
| -------- | ------------- | ----------------------- |
| Language | C++           | JavaScript / TypeScript |
| UI       | wxWidgets     | Electron                |
| Type     | Native        | Web-based               |

---

### 🔎 Summary

* **Krafta Editor** is lightweight and starts instantly
* **VS Code** is powerful but significantly heavier due to Electron

---

## 🚀 **Getting Started**

### Prerequisites

| Platform    | Requirements                 |
| ----------- | ---------------------------- |
| **All**     | CMake ≥ 3.27, C++20 compiler |
| **Linux**   | GTK3                         |
| **macOS**   | Xcode Command Line Tools     |
| **Windows** | Visual Studio 2022+          |

---

## 🧩 **wxWidgets — Recommended Setup**

Krafta Editor depends on **wxWidgets 3.3+**. There are two ways to provide it:

### ✅ Option 1 — FetchContent (Recommended)

By default, CMake automatically downloads and compiles wxWidgets 3.3 during the build.
**No manual installation is needed.** This is the recommended approach because it guarantees
the correct version on every platform and requires zero extra setup.

> ⚠️ The first build will take longer since wxWidgets is compiled from source.
> Subsequent builds are fast due to CMake's FetchContent cache.

```bash
# Nothing to do — just build normally:
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

---

### Option 2 — System-installed wxWidgets

If you prefer to use a version already installed on your system, pass `-DUSE_SYSTEM_WXWIDGETS=ON`.

> ⚠️ **Version requirement: wxWidgets 3.3 or newer.**
> If version 3.2 or older is detected, the build will automatically fall back to
> FetchContent and download 3.3 — no error, no manual intervention needed.

#### Linux (Debian / Ubuntu)

```bash
# wxWidgets 3.2 is available in most distro repos — the build will still work,
# but it will fall back to FetchContent to fetch 3.3 automatically.
sudo apt install build-essential cmake libgtk-3-dev libwxgtk3.2-dev

# To use it:
cmake -S . -B build -DUSE_SYSTEM_WXWIDGETS=ON -DCMAKE_BUILD_TYPE=Release
```

#### macOS (Homebrew)

```bash
brew install cmake wxwidgets   # Homebrew ships wxWidgets 3.3+

cmake -S . -B build -DUSE_SYSTEM_WXWIDGETS=ON -DCMAKE_BUILD_TYPE=Release
```

#### Windows

1. Download wxWidgets 3.3+ from [wxwidgets.org/downloads](https://www.wxwidgets.org/downloads/)
2. Build and install it following the official guide
3. Set the `wxWidgets_ROOT_DIR` environment variable to your install path
4. Configure with:

```bat
cmake -S . -B build -DUSE_SYSTEM_WXWIDGETS=ON -DCMAKE_BUILD_TYPE=Release
```

---

## ⚙️ **Building from Source**

```bash
# Clone the repository
git clone https://github.com/apozinn/kraftaEditor.git
cd kraftaEditor

# Configure (wxWidgets downloaded automatically)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
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
>   build/bin/Release/kraftaEditor
>   ```

---

### All CMake Options

| Flag | Default | Description |
| ---- | ------- | ----------- |
| `-DUSE_SYSTEM_WXWIDGETS=ON` | OFF | Use system wxWidgets instead of FetchContent |
| `-DENABLE_TESTS=ON` | OFF | Build the test suite |
| `-DENABLE_CLANG_TIDY=ON` | OFF | Enable clang-tidy static analysis |
| `-DENABLE_WERROR=ON` | OFF | Treat warnings as errors |
| `-DENABLE_UNITY_BUILD=OFF` | ON | Disable unity build (faster incremental builds) |

> ℹ️ `-DCMAKE_BUILD_TYPE=Release` is only required when using **single-config generators**
> (Linux/macOS with Make or Ninja).

---

## 🐧 **Linux Installation & Uninstallation**

There are two ways to install Krafta Editor system-wide on Linux:

| Method | Best for |
| ------ | -------- |
| **Helper scripts** (`scripts/install.sh`) | Quick local install from a source build |
| **CMake install** (`cmake --target install`) | CI, packaging, custom install prefixes |

Both methods install the binary, assets, desktop entry, and icon, and refresh the system caches automatically so the app appears in your applications menu.

> ⚠️ **Root permissions are required.**
> Files are written to `/usr/local/bin` and `/usr/share`.

---

### 📦 Method 1 — Helper script

Make sure the project is built before running the script:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Then install:

```bash
chmod +x scripts/install.sh
sudo ./scripts/install.sh
```

**What it installs:**

```
/usr/local/bin/kraftaEditor                          ← executable
/usr/share/kraftaEditor/
├── assets/
├── languages/
├── config/
└── i18n/
/usr/share/icons/hicolor/256x256/apps/kraftaEditor.png
/usr/share/applications/krafta-editor.desktop
```

After installation you can launch Krafta Editor from the applications menu or run it directly:

```bash
kraftaEditor
```

---

### 📦 Method 2 — CMake install

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
sudo cmake --install build
```

To install to a custom prefix (no `sudo` needed):

```bash
cmake --install build --prefix ~/.local
```

CMake handles everything the script does, including updating the desktop database and icon cache.

---

### 🗑️ Uninstall

```bash
chmod +x scripts/uninstall.sh
sudo ./scripts/uninstall.sh
```

**What it removes:**

```
/usr/local/bin/kraftaEditor
/usr/share/kraftaEditor/
/usr/share/icons/hicolor/256x256/apps/kraftaEditor.png
/usr/share/applications/krafta-editor.desktop
```

The desktop database and icon cache are refreshed automatically. After uninstalling, no system files related to Krafta Editor remain.

---

### ℹ️ Notes

* The helper scripts are intended for **local/system-wide installs from source**
* They are **not a replacement for package managers**
* For distribution packaging (`.deb`, `.rpm`, Arch PKGBUILD), use `cmake --install` with the appropriate prefix and reference the scripts as a guide

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

## 📜 **License**

Krafta Editor is licensed under the **GNU LGPLv3**.

© 2023–2026 **Okarin Softwares**

---

## 💬 **Support**

* 🐞 [Open an Issue](https://github.com/apozinn/kraftaEditor/issues)
* 💡 Use Discussions for feature ideas

---

⭐ **If you like Krafta Editor, consider giving it a star — it helps a lot!**

---