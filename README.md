<div align="center">
<img width="200" src="https://raw.githubusercontent.com/apozinn/kraftaEditor/refs/heads/main/assets/kraftaEditor.png">
</div>

**<p align="center" style="font-size: 40px; font-weight: bold">Krafta Editor</p>**

<p align="center">Fast, lightweight and cross-platform code editor</p>

#

<img src="https://i.imgur.com/cj3d00m.png">

Written in c++. Krafta Editor was created to be fast, efficient and cross-platform(with GUI framework [wxWidgets](https://wxwidgets.org)).

# Purpose

With a focus on performance, functionality, Krafta tries to bring all the necessary tools to edit any code, in any language, and still with a modern and intuitive interface.

### System Requirements

To get started with this project, ensure you have the following tools and libraries:

- **CMake (Version 3.14 or later):** Essential for building and managing the project files.
- **C++ Compiler:** Compatible with Clang, GCC, or MSVC. Choose one based on your development environment.
- **GTK3 Development Libraries (for Linux users):** Necessary for GUI development on Linux platforms.

### Building the Project

#### Debug

Follow these steps to build the project:

1. **Create a build directory & configure the build:**
   ```bash
   cmake -S. -Bbuild
   ```

2. **Build the project:**
   ```bash
   cmake --build build -j
   ```

This will create a `build` directory and compile all necessary artifacts there. The main executable will be located in `build/`.

#### Release

For release build use `--config Release` on Windows:

```bash
cmake -S. -Bbuild
cmake --build build -j --config Release
```

Artifacts for both configurations will be generated in the `build` directory.

On Mac or Linux you'll need to maintain two build trees:

```bash
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
cmake -S. -Bbuild-rel -DCMAKE_BUILD_TYPE=Release
cmake --build build-rel -j
```
# Contributing

Any help that will improve and implement the project is welcome. Feel free to send suggestions, revisions or new features.

# License

Copyright (c) Okarin Softwares. All rights reserved.

Licensed under the [GNU LGPLv3](https://github.com/apozinn/kraftaEditor/blob/main/LICENSE) license.
