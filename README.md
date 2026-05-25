# <img src="https://raw.githubusercontent.com/apozinn/kraftaEditor/refs/heads/main/assets/images/kraftaEditor.png" alt="Krafta Editor" width="54" align="middle"> &nbsp; Krafta Editor

![Version](https://img.shields.io/badge/Version-0.9.0-orange)
![License](https://img.shields.io/badge/License-LGPLv3-blue)
![C++20](https://img.shields.io/badge/C++-20-00599C?logo=cplusplus)
![Platforms](https://img.shields.io/badge/Platforms-Windows%20%7C%20Linux%20%7C%20macOS-success)

A fast, native code editor built with C++ and wxWidgets.

No Electron. No Chromium. No web stack.
Just a lightweight native application focused on speed, responsiveness, and low memory usage.

![Krafta Editor — dark theme](https://i.imgur.com/GM7mMJ6.png)

## Performance

Measured on a Release build running on Linux.

| Metric           | Krafta Editor | Visual Studio Code |
| ---------------- | ------------- | ------------------ |
| Startup time     | < 200 ms      | ~2–4 seconds       |
| RAM usage (idle) | ~60–70 MB     | ~300–500 MB        |
| Executable size  | ~11 MB        | ~350–400 MB        |

> Values are approximate and may vary depending on platform, compiler, extensions, and system configuration.

---

## Platform Status

| Platform | Status       |
| -------- | ------------ |
| Linux    | Stable       |
| Windows  | Stable       |
| macOS    | Experimental |

---

## Tech Stack

* C++20
* wxWidgets 3.3+
* CMake 3.27+

---

## Prerequisites

| Platform | Requirements                                 |
| -------- | -------------------------------------------- |
| All      | CMake ≥ 3.27 and a C++20-compatible compiler |
| Linux    | GTK3 (`libgtk-3-dev`)                        |
| macOS    | Xcode Command Line Tools                     |
| Windows  | Visual Studio 2022+                          |

wxWidgets 3.3+ is required.

By default, CMake downloads and builds wxWidgets automatically using FetchContent, so no manual installation is required.

See [wxWidgets Setup](#wxwidgets-setup) for system-installed alternatives.

---

## Build from Source

```bash
git clone https://github.com/apozinn/kraftaEditor.git
cd kraftaEditor

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run directly after building
cmake --build build --target run --config Release

```

The executable is generated at:

```text
build/bin/Release/kraftaEditor

```

> On Windows with Visual Studio, `--config Release` is mandatory.
> On Linux/macOS with Ninja or Make, it is optional but kept for consistency.

---

## wxWidgets Setup

### Option 1 — FetchContent (Recommended)

CMake automatically downloads and compiles wxWidgets 3.3.

No additional setup is required.

The first build may take longer due to dependency compilation, but subsequent builds use cached artifacts.

---

### Option 2 — System-installed wxWidgets

Use an existing wxWidgets installation by passing the appropriate flag to CMake:

```text
-DUSE_SYSTEM_WXWIDGETS=ON

```

If the detected version is older than 3.3, the build automatically falls back to FetchContent.

---

### Linux (Debian/Ubuntu)

wxWidgets 3.3 is not yet available in official apt repositories.

Install only the required system dependencies:

```bash
sudo apt install build-essential cmake libgtk-3-dev

```

Then build normally:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

```

> `-DUSE_SYSTEM_WXWIDGETS=ON` is currently not applicable on Linux until wxWidgets 3.3 becomes available in official repositories.

---

### macOS

```bash
brew install cmake wxwidgets

cmake -S . -B build \
  -DUSE_SYSTEM_WXWIDGETS=ON \
  -DCMAKE_BUILD_TYPE=Release

```

---

### Windows

1. Download wxWidgets 3.3+ from: [https://www.wxwidgets.org/downloads/](https://www.wxwidgets.org/downloads/)
2. Build and install it following the official documentation.
3. Set `wxWidgets_ROOT_DIR` to the installation path.
4. Configure the project:

```cmd
cmake -S . -B build ^
  -DUSE_SYSTEM_WXWIDGETS=ON ^
  -DCMAKE_BUILD_TYPE=Release

```

---

## CMake Options

| Flag | Default | Description |
| --- | --- | --- |
| `-DUSE_SYSTEM_WXWIDGETS` | OFF | Use system wxWidgets instead of FetchContent |
| `-DENABLE_TESTS` | OFF | Build test suite |
| `-DENABLE_CLANG_TIDY` | OFF | Enable clang-tidy analysis |
| `-DENABLE_WERROR` | OFF | Treat warnings as errors |
| `-DENABLE_UNITY_BUILD` | ON | Disable unity build for faster incremental builds |

---

## Linux Installation

Two installation methods are available. Both install the executable, assets, desktop entry, and application icon, as well as refreshing desktop and icon caches automatically. Root permissions are required for system-wide installation.

---

### Method 1 — Install Script

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

chmod +x scripts/install.sh
sudo ./scripts/install.sh

```

Installed paths:

```text
/usr/local/bin/kraftaEditor
/usr/share/kraftaEditor/{assets,languages,config,i18n}
/usr/share/icons/hicolor/256x256/apps/kraftaEditor.png
/usr/share/applications/krafta-editor.desktop

```

---

### Method 2 — CMake Install

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

sudo cmake --install build

```

Install locally without `sudo`:

```bash
cmake --install build --prefix ~/.local

```

---

### Uninstall

```bash
chmod +x scripts/uninstall.sh
sudo ./scripts/uninstall.sh

```

Removes all installed files and refreshes desktop/icon caches.

> The install scripts are intended for local/system-wide source installs and are not replacements for package managers.
> For `.deb`, `.rpm`, or Arch Linux packaging, prefer `cmake --install` with the appropriate prefix.

---

## Roadmap

* [ ] LSP integration (Language Server Protocol)
* [ ] Integrated terminal
* [ ] Plugin API
* [ ] Git integration
* [ ] Multi-cursor editing

---

## Contributing

### Guidelines

1. Fork the repository
2. Create a branch from `main`
3. Keep commits focused and descriptive
4. Follow the existing code style
5. Update documentation when applicable
6. Open a pull request with a clear description

For bug reports and feature requests, use:
[https://github.com/apozinn/kraftaEditor/issues](https://github.com/apozinn/kraftaEditor/issues)

---

## License

Krafta Editor is licensed under the GNU LGPLv3 license. See the `LICENSE` file for details.

---

© 2023–2026 Okarin Softwares