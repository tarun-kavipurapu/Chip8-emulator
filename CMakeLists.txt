cmake_minimum_required(VERSION 3.14)
project(chip8)

set(CMAKE_CXX_STANDARD 11)

# Add glad, SDL, and imgui as subdirectories
add_subdirectory(Extra/glad EXCLUDE_FROM_ALL)
add_subdirectory(Extra/sdl-2.30.7 EXCLUDE_FROM_ALL)
add_subdirectory(Extra/imgui-1.91.1 EXCLUDE_FROM_ALL)

# Set up the executable
add_executable(
	chip8
	Source/Chip8.cpp
	Source/Main.cpp
	Source/Platform.cpp)

# Compile with Wall warnings
target_compile_options(chip8 PRIVATE -Wall)

# Link glad, SDL2, and imgui
target_link_libraries(chip8 PRIVATE glad SDL2 imgui)
