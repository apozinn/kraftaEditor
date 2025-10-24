# ⚡ **Krafta Editor**

<p align="center">
  <img width="180" src="https://raw.githubusercontent.com/apozinn/kraftaEditor/refs/heads/main/assets/images/kraftaEditor.png" alt="Krafta Editor Logo">
</p>

<p align="center">
  <b>A fast, lightweight, and cross-platform code editor built with C++ and wxWidgets.</b><br>
  Designed for developers who value <b>speed, simplicity, and native performance</b>.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C++-17+-00599C?style=for-the-badge&logo=cplusplus" alt="C++ 17+">
  <img src="https://img.shields.io/badge/wxWidgets-3.1+-0078D6?style=for-the-badge&logo=windowsterminal" alt="wxWidgets 3.1+">
  <img src="https://img.shields.io/badge/Version-0.8.0-orange?style=for-the-badge" alt="Version 0.8.0">
  <img src="https://img.shields.io/badge/License-LGPLv3-blue?style=for-the-badge" alt="LGPLv3 License">
</p>

---

## ✨ **Key Features**

- ⚡ **Blazing fast performance** powered by native C++
- 🧩 **Cross-platform support:** Windows, Linux, macOS
- 🎨 **Modern, customizable UI** built with wxWidgets
- 🧠 **Smart syntax highlighting** for multiple languages
- 🪶 **Lightweight** — no Electron or heavy dependencies
- 🧰 **Productivity-first design** with intuitive workflows

[![Krafta Editor Screenshot]([https://i.postimg.cc/C122qcM4/image.png](https://i.postimg.cc/C122qcM4/image.png))](https://postimg.cc/Vrj4yBCS)

---

## 🚀 **Getting Started**

### Prerequisites

| Platform | Requirements |
|-----------|--------------|
| **All**   | CMake ≥ 3.21, C++20 compiler |
| **Linux** | GTK3, wxGTK |
| **macOS** | Xcode Command Line Tools |
| **Windows** | Visual Studio 2022+ |

> 💡 *Tip:* You can use CMake’s `FetchContent` to automatically download wxWidgets.  
> It simplifies setup but increases the build directory size.

---

### 🧩 **Installation**

#### Linux (Debian/Ubuntu)
```bash
sudo apt install build-essential cmake libgtk-3-dev libwxgtk3.0-gtk3-dev
````

#### macOS (Homebrew)

```bash
brew install cmake wxwidgets
```

#### Windows

1. Install [CMake](https://cmake.org/download/)
2. Install [wxWidgets](https://www.wxwidgets.org/downloads/)
3. Install **Visual Studio 2022+** with C++ support

---

### ⚙️ **Building from Source**

```bash
# Clone the repository
git clone https://github.com/apozinn/kraftaEditor.git
cd kraftaEditor

# Configure (Debug build)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --parallel

# Run
./build/bin/KraftaEditor
```

#### Optional CMake Flags

```bash
-DUSE_SYSTEM_WXWIDGETS=ON   # Use system wxWidgets
-DENABLE_TESTS=ON            # Build test suite
-DCMAKE_BUILD_TYPE=Release   # Optimized release build
```

---

## 🛠️ **Development**

Contributions are welcome!

1. **Fork** the repository
2. Create a **branch:** `git checkout -b feature/new-feature`
3. **Commit:** `git commit -am 'Add new feature'`
4. **Push:** `git push origin feature/new-feature`
5. Open a **Pull Request**

### Code Style

* Follow the existing style (4-space indentation)
* Keep commits **clear and concise**
* Update documentation when applicable

---

## 🧾 **Release Highlights — v0.8.0 “Boosting Editor Productivity”**

Version 0.8.0 delivers a major overhaul to editing fluidity and core stability — introducing modern features that improve multi-cursor selection, auto-completion, and overall responsiveness.

### 🧠 **Code Editing Workflow**

| Feature                         | Description                                                                           |
| ------------------------------- | ------------------------------------------------------------------------------------- |
| **Smart HTML Auto-Completion**  | Automatically closes and completes HTML tags to reduce boilerplate typing.            |
| **Select Next Occurrence**      | Instantly select the next matching instance for multi-cursor editing and refactoring. |
| **Line Duplication Commands**   | Quickly duplicate the current line or block above/below the cursor.                   |
| **Optimized Cursor Navigation** | Faster cursor movement and smoother text manipulation.                                |

### 🎨 **Customization & UI**

| Feature                    | Description                                                              |
| -------------------------- | ------------------------------------------------------------------------ |
| **Theming for Indicators** | Indicator colors (e.g., selections, highlights) are now fully themeable. |
| **New Language Icons**     | Added dedicated icons for JavaScript, XML, and TypeScript files.         |

### 🧩 **Stability & Quality**

| Area                     | Improvements                                                                                 |
| ------------------------ | -------------------------------------------------------------------------------------------- |
| **Core Editor**          | Refactored for better backspace handling, unsaved state display, and selection highlighting. |
| **Event System**         | Fixed a critical issue where the `CharAdded` event failed to trigger for auto-completion.    |
| **User Experience (UX)** | Fixed misaligned tab icons, removed unused assets to reduce binary size.                     |

> 🔧 We strongly recommend updating to **v0.8.0** for improved editing speed, stability, and overall UX.

---

## 📜 **License**

Krafta Editor is licensed under the [GNU LGPLv3](https://github.com/apozinn/kraftaEditor/blob/main/LICENSE).

© 2023–2025 Okarin Softwares. Released under the LGPLv3 License.

---

## 💬 **Support**

Questions, suggestions, or bug reports?

* [Open an **Issue**](https://github.com/apozinn/kraftaEditor/issues)
* Or start a **Discussion** for feature ideas

---

## 🗺️ **Roadmap (Preview)**

* [ ] Plugin system (extensions support)
* [ ] Split-view editing
* [ ] Improved search & replace panel
* [ ] Portable Windows build
* [ ] Theme and syntax editor UI

---

⭐ **If you like Krafta Editor, give it a star on GitHub — it helps a lot!**
