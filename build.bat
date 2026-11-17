@echo off
setlocal enabledelayedexpansion
cd /d %~dp0

:: Initialize EDK2 if directory is missing
if not exist "edk2" (
    echo EDK2 directory not found. Initializing repository...
    git clone https://github.com/tianocore/edk2.git || exit /b 1
    pushd edk2
    git submodule update --init || (popd & exit /b 1)
    call edksetup.bat Rebuild
    popd
)

:: Validate required build tools in PATH
for %%T in (python nasm iasl) do (
    where %%T >nul 2>&1 || (
        echo Error: '%%T' is required but not found in PATH.
        exit /b 1
    )
)

:MENU_LOOP
echo.
echo ===================================================
echo     EDK2 Builder for PlaygroundPkg (OVMF X64)
echo ===================================================
echo.

:: 1. Select Target
echo [1] DEBUG (default)
echo [2] RELEASE
set /p TARGET_CHOICE="Select Target [1-2]: "
set "TARGET=DEBUG"
if "%TARGET_CHOICE%"=="2" set "TARGET=RELEASE"

:: 2. Select Build Type
echo.
echo [1] Incremental (default)
echo [2] Clean
set /p BUILD_CHOICE="Select Build Type [1-2]: "
set "CLEAN_FLAG="
if "%BUILD_CHOICE%"=="2" set "CLEAN_FLAG=--clean"

echo.
echo Building: X64 ^| %TARGET% ^| VS2026 %CLEAN_FLAG%
echo.

python build.py --arch X64 --target %TARGET% --toolchain VS2026 %CLEAN_FLAG% %*
if !errorlevel! neq 0 (
    echo.
    echo Build failed with error code !errorlevel!.
)

echo.
set /p BUILD_AGAIN="Build again? [Y/N] (default: Y): "
if /i not "%BUILD_AGAIN%"=="N" (
    goto :MENU_LOOP
)

endlocal
exit /b 0
