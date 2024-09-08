Here’s a sample **README** file for your CHIP-8 Emulator project. You can adapt this based on your specific setup and preferences:

---

# CHIP-8 Emulator

This project is a CHIP-8 emulator developed in C++ using SDL2 for handling input and display, GLAD for OpenGL function loading, and ImGui for building a graphical interface. The emulator simulates the CHIP-8 virtual machine, capable of running classic CHIP-8 programs.

## Features
- **Basic CHIP-8 CPU Emulation**: Supports all 35 CHIP-8 instructions.
- **OpenGL Rendering**: The emulator uses OpenGL for graphical output, with SDL2 for handling windowing and input.
- **User Interface with ImGui**: An optional UI layer for controls and debugging.
- **Cross-Platform Support**: Runs on Windows (MinGW or Visual Studio), and can be configured to run on other platforms.

## Dependencies
This project uses the following external libraries:
- **SDL2**: For handling input, window management, and sound.
- **GLAD**: For loading OpenGL function pointers.
- **ImGui**: For building a simple GUI for controlling the emulator.

All dependencies are included in the `Extra` folder and automatically linked via CMake.
