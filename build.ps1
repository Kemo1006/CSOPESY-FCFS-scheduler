# Builds build\emulator.exe with g++ (no CMake needed).
$root = $PSScriptRoot
New-Item -ItemType Directory -Force "$root\build" | Out-Null
g++ -std=c++17 -O2 -Wall -Wextra -pthread -static "$root\src\main.cpp" "$root\src\scheduler.cpp" -o "$root\build\emulator.exe"
