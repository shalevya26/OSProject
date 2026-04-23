
cmake_minimum_required(VERSION 4.0)

# Force CMake to allow older policies for external libraries like Raylib
set(CMAKE_POLICY_VERSION_MINIMUM "3.5" CACHE STRING "Minimum policy version" FORCE)

project(OSProject C)

set(CMAKE_C_STANDARD 99)

# 1. Include the FetchContent module
include(FetchContent)

# 2. Tell CMake where to download Raylib
FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        5.0
)

# 3. Download and make Raylib available
FetchContent_MakeAvailable(raylib)

# 4. Create your executable

add_executable(OSproject main.c Dijkstra.c Gui.c Dijkstra.h Gui.h)

# 5. Link Raylib to your project
target_link_libraries(OSproject PRIVATE raylib m)