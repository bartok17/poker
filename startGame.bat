@echo off
setlocal

if exist build rmdir /s /q build
cmake -S . -B build
cmake --build build --config Release

if exist "build\Release\poker.exe" (
  "build\Release\poker.exe"
) else if exist "build\poker.exe" (
  "build\poker.exe"
) else (
  echo Could not find built executable. Check the build output.
  exit /b 1
)

endlocal