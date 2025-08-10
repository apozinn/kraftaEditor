# **Krafta Editor**  

<div align="center">
  <img width="200" src="https://raw.githubusercontent.com/apozinn/kraftaEditor/refs/heads/main/assets/images/kraftaEditor.png" alt="Krafta Editor Logo">
  
  <h1 style="font-size: 2.5rem; margin: 0.5em 0">Krafta Editor</h1>
  
  <p style="font-size: 1.2rem; max-width: 600px; margin: 0 auto">
    A fast, lightweight, and cross-platform code editor built with C++ and wxWidgets
  </p>
  
  <div>
    <img src="https://img.shields.io/badge/C++-17+-00599C?logo=cplusplus" alt="C++ 17+">
    <img src="https://img.shields.io/badge/wxWidgets-3.1+-0078D6?logo=windowsterminal" alt="wxWidgets 3.1+">
    <img src="https://img.shields.io/badge/License-LGPLv3-blue" alt="LGPLv3 License">
  </div>
</div>

## ✨ **Features**  

- **Blazing fast performance** with native C++ core  
- **Cross-platform** (Windows, Linux, macOS)  
- **Modern and customizable interface** with wxWidgets  
- **Syntax highlighting** for multiple programming languages  
- **Lightweight** compared to Electron-based editors  
- **Intuitive interface** focused on productivity  

![Krafta Editor Screenshot](https://i.imgur.com/cj3d00m.png)  

## 🚀 **Getting Started**  

### Prerequisites

| Platform       | Requirements |
|---------------|-------------|
| **All**       | CMake ≥ 3.21, C++20 compiler |
| **Linux**     | GTK3, wxGTK |
| **macOS**     | Xcode Command Line Tools |
| **Windows**   | Visual Studio 2022+ | 

### **Note**
Alternatively, you can leverage CMake's FetchContent module to automatically download and integrate **wxWidgets**. While this simplifies the initial setup, be aware that it will download the complete **wxWidgets** source code, resulting in a much larger build directory.

### **Installation**  

#### **Linux (Debian/Ubuntu)**  
```bash
sudo apt install build-essential cmake libgtk-3-dev libwxgtk3.0-gtk3-dev
```  

#### **macOS (Homebrew)**  
```bash
brew install cmake wxwidgets
```  

#### **Windows**  
- Install [CMake](https://cmake.org/download/)  
- Install [wxWidgets](https://www.wxwidgets.org/downloads/)  
- Install **Visual Studio** with C++ support  

### **Building from Source**  

```bash
# Clone the repository
git clone https://github.com/apozinn/kraftaEditor.git
cd kraftaEditor

# Configure (Debug build)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --parallel

# Run
./build/bin/DEBUG/KraftaEditor
```  

### Build Options:

``` 
-DUSE_SYSTEM_WXWIDGETS=ON - Use system wxWidgets

-DENABLE_TESTS=ON - Build test suite

-DCMAKE_BUILD_TYPE=Release - Optimized release build
```  

## 🛠️ **Development**  

Contributions are welcome! Follow these steps:  

1. **Fork** the repository  
2. Create a **branch** (`git checkout -b feature/new-feature`)  
3. **Commit** your changes (`git commit -am 'Add new feature'`)  
4. **Push** to the branch (`git push origin feature/new-feature`)  
5. Open a **Pull Request**  

### **Code Style**  
- Follow the current style (4-space indentation)  
- Keep commits **clear and concise**  
- Update documentation when needed  

## 📜 **License**  

Krafta Editor is licensed under the [GNU LGPLv3](https://github.com/apozinn/kraftaEditor/blob/main/LICENSE).  

Copyright © 2023 Okarin Softwares. All rights reserved.  

## 🤝 **Support**  

Questions, suggestions, or bug reports?  
- [Open an **Issue**](https://github.com/apozinn/kraftaEditor/issues)  
- Join our [**Discord**](https://discord.gg/yWzdJbu2KP)