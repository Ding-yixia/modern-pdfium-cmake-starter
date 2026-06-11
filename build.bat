@echo off
setlocal enabledelayedexpansion

REM ============================================================================
REM build.bat - Build script for modern-pdfium-cmake-starter
REM ============================================================================
REM
REM Usage:
REM   build.bat <path-to-pdfium_cmake_support>
REM
REM Examples:
REM   build.bat C:\Users\User\Documents\trae_projects\pdfium_cmake_support
REM   build.bat ..\pdfium_cmake_support
REM
REM Prerequisites:
REM   1. CMake 3.20+ (https://cmake.org/download/)
REM   2. Ninja (https://ninja-build.org/)
REM   3. LLVM/Clang (https://llvm.org/)
REM   4. pdfium_cmake_support built successfully first
REM ============================================================================

set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

REM ---- Check arguments ----
if "%~1"=="" (
    echo [ERROR] Missing PDFIUM_ROOT argument.
    echo Usage: build.bat ^<path-to-pdfium_cmake_support^>
    echo Example: build.bat C:\Users\User\Documents\trae_projects\pdfium_cmake_support
    exit /b 1
)

set "PDFIUM_ROOT=%~f1"

REM ---- Validate PDFIUM_ROOT ----
if not exist "%PDFIUM_ROOT%" (
    echo [ERROR] PDFIUM_ROOT directory not found: %PDFIUM_ROOT%
    exit /b 1
)
if not exist "%PDFIUM_ROOT%\build\cmake_build\lib" (
    echo [ERROR] PDFium static libraries not found at "%PDFIUM_ROOT%\build\cmake_build\lib"
    echo.
    echo Please build pdfium_cmake_support first:
    echo   cd "%PDFIUM_ROOT%"
    echo   cmake -B build/cmake_build -G Ninja -DCMAKE_BUILD_TYPE=Release
    echo   cmake --build build/cmake_build
    exit /b 1
)

echo ============================================================================
echo  modern-pdfium-cmake-starter Build Script
echo ============================================================================
echo  PDFIUM_ROOT: %PDFIUM_ROOT%
echo  Output dir : %SCRIPT_DIR%\build
echo ============================================================================

REM ---- Configure CMake ----
echo [1/2] Configuring CMake...
cmake -B "%SCRIPT_DIR%\build" -S "%SCRIPT_DIR%" ^
    -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DPDFIUM_ROOT="%PDFIUM_ROOT%" ^
    -DBUILD_EXAMPLES=ON
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

REM ---- Build ----
echo [2/2] Building...
cmake --build "%SCRIPT_DIR%\build"
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo ============================================================================
echo  Build successful!
echo  Output: %SCRIPT_DIR%\build\examples\
echo ============================================================================

REM ---- Run basic test ----
set "TEST_PDF=%PDFIUM_ROOT%\third_party\zlib\doc\crc-doc.1.0.pdf"
if exist "%TEST_PDF%" (
    echo.
    echo Running basic test with: %TEST_PDF%
    "%SCRIPT_DIR%\build\examples\pdfium_basic.exe" "%TEST_PDF%"
    if errorlevel 1 (
        echo [WARNING] Basic test exited with error code !errorlevel!.
    ) else (
        echo Basic test passed!
    )
)

endlocal