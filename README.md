# <img src="https://raw.githubusercontent.com/apozinn/kraftaEditor/refs/heads/main/assets/images/kraftaEditor.png" alt="Krafta Editor" width="54" align="middle"> &nbsp; Krafta Editor

**C++. Cross-platform. LSP built in. 20+ languages.**

**Fast. Lean. Native.**

A native code editor, written in C++.

![Version](https://img.shields.io/badge/Version-0.9.3-orange)
![License](https://img.shields.io/badge/License-Apache_2.0-blue)
![C++20](https://img.shields.io/badge/C++-20-00599C?logo=cplusplus)
![Platforms](https://img.shields.io/badge/Platforms-Windows%20%7C%20Linux%20%7C%20macOS-success)

<p align="center">
  <img src="assets/screenshots/dark.png" alt="Krafta Editor with LSP completion, dark theme">
  <br>
  <em>LSP completion in C++ with clangd</em>
</p>

## Built close to the machine

Krafta is written in C++ with native widgets, from the interface to the text buffer. The result is a compact executable with a small memory and CPU footprint, and an editor that responds the instant you type.

- **Fast to start.** Ready in under 200 ms.
- **Light on resources.** About 60–70 MB of RAM at idle, in an ~11 MB executable.
- **Language-aware.** Syntax highlighting for 20+ languages, plus LSP for C/C++, Python and JavaScript/TypeScript.
- **The same everywhere.** One codebase for Linux, Windows and macOS.

| Metric           | Krafta Editor |
| ---------------- | ------------- |
| Startup time     | < 200 ms      |
| RAM usage (idle) | ~60–70 MB     |
| Executable size  | ~11 MB        |

> Measured on a Release build on Linux. Values are approximate and vary with platform, compiler and system configuration.

## Features

- **LSP client**: completion, hover, go to definition, references, rename and formatting.
- **Multicaret editing** and **split editor**.
- **Minimap** for quick navigation.
- **Color themes** defined in JSON.
- **Translations**: English and Português (Brasil).

## Themes

Three built-in themes, plus your own through JSON.

<table>
  <tr>
    <td><img src="assets/screenshots/light.png" alt="Light theme"></td>
    <td><img src="assets/screenshots/dark.png" alt="Dark theme"></td>
    <td><img src="assets/screenshots/dracula.png" alt="Dracula theme"></td>
  </tr>
  <tr align="center">
    <td>Light</td>
    <td>Dark</td>
    <td>Dracula</td>
  </tr>
</table>

## Language support

| Support             | Languages                                               |
| ------------------- | ------------------------------------------------------- |
| Syntax highlighting | 20+ languages                                           |
| LSP                 | C/C++ (clangd), Python (pylsp), JavaScript / TypeScript |

## Platform status

| Platform | Status       |
| -------- | ------------ |
| Linux    | Stable       |
| Windows  | Stable       |
| macOS    | Experimental |

## Get started

```bash
git clone https://github.com/apozinn/kraftaEditor.git
cd kraftaEditor

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run it
cmake --build build --target run
```

CMake downloads and builds wxWidgets automatically, so the first build takes longer. Later builds reuse the cache.

**Requirements:** CMake ≥ 3.27 and a C++20 compiler.

| Platform | Also needs                                           |
| -------- | ---------------------------------------------------- |
| Linux    | GTK3 — `libgtk-3-dev` (Debian/Ubuntu), `gtk3` (Arch) |
| macOS    | Xcode Command Line Tools                             |
| Windows  | Visual Studio 2022+                                  |

On Windows with Visual Studio, pass `--config Release` to the build command.

The executable is generated at `build/bin/Release/kraftaEditor`.

## Open from the terminal

```bash
kraftaEditor ./project     # open a folder
kraftaEditor ./file.txt    # open a file
kraftaEditor .             # open the current directory
```

## Linux installation

Both methods install the executable, assets, desktop entry and icon, and refresh the desktop and icon caches. System-wide installation needs root.

**Install script**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

chmod +x scripts/install.sh
sudo ./scripts/install.sh
```

**CMake install**

```bash
sudo cmake --install build                    # system-wide
cmake --install build --prefix ~/.local       # current user, no sudo
```

**Uninstall**

```bash
chmod +x scripts/uninstall.sh
sudo ./scripts/uninstall.sh
```

<details>
<summary>Installed paths (install script)</summary>

```text
/usr/local/bin/kraftaEditor
/usr/share/kraftaEditor/{assets,languages,config,i18n}
/usr/share/icons/hicolor/256x256/apps/kraftaEditor.png
/usr/share/applications/krafta-editor.desktop
```

</details>

> The install scripts are meant for source installs, not as a replacement for a package manager. For `.deb`, `.rpm` or Arch packaging, use `cmake --install` with the appropriate prefix.

## Build reference

<details>
<summary>wxWidgets setup</summary>

**Option 1 — FetchContent (recommended).** CMake downloads and compiles wxWidgets 3.3 automatically. No setup needed.

**Option 2 — system-installed wxWidgets.** Pass `-DUSE_SYSTEM_WXWIDGETS=ON`. If the detected version is older than 3.3, the build falls back to FetchContent.

- **Linux:** wxWidgets 3.3 is not yet in the official Debian/Ubuntu repositories, so this option does not apply there for now.
- **macOS:**
  ```bash
  brew install cmake wxwidgets
  cmake -S . -B build -DUSE_SYSTEM_WXWIDGETS=ON -DCMAKE_BUILD_TYPE=Release
  ```
- **Windows:** download wxWidgets 3.3+ from [wxwidgets.org](https://www.wxwidgets.org/downloads/), build and install it, set `wxWidgets_ROOT_DIR` to the install path, then:
  ```cmd
  cmake -S . -B build ^
    -DUSE_SYSTEM_WXWIDGETS=ON ^
    -DCMAKE_BUILD_TYPE=Release
  ```

</details>

<details>
<summary>CMake options</summary>

| Flag                     | Default | Description                                           |
| ------------------------ | ------- | ----------------------------------------------------- |
| `-DUSE_SYSTEM_WXWIDGETS` | OFF     | Use system wxWidgets instead of FetchContent          |
| `-DENABLE_TESTS`         | OFF     | Build the test suite                                  |
| `-DENABLE_CLANG_TIDY`    | OFF     | Enable clang-tidy analysis                            |
| `-DENABLE_WERROR`        | OFF     | Treat warnings as errors                              |
| `-DENABLE_UNITY_BUILD`   | ON      | Unity build; set to OFF for faster incremental builds |

</details>

**Tech stack:** C++20 · wxWidgets 3.3+ · CMake 3.27+

## Contributing

1. Fork the repository.
2. Create a branch from `main`.
3. Keep commits focused and descriptive.
4. Follow the existing code style.
5. Update the documentation when applicable.
6. Open a pull request with a clear description.

Bug reports and feature requests: [GitHub Issues](https://github.com/apozinn/kraftaEditor/issues)

## License

Apache License 2.0. See the `LICENSE` file for details.

© 2023–2026 Okarin Softwares