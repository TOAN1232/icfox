@echo off
title Building icfox ESP...

echo === icfox Build Script ===
echo.

REM Check for Visual Studio
if not defined DevEnvDir (
    echo [!] Visual Studio not detected in PATH.
    echo [*] Attempting to find vcvarsall.bat...
    
    REM Try common VS installation paths
    set "VS_PATH="
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
    
    if defined VS_PATH (
        echo [*] Found Visual Studio at: %VS_PATH%
        call "%VS_PATH%" x64
    ) else (
        echo [!] Could not find Visual Studio automatically.
        echo [*] Please run this script from a Visual Studio Developer Command Prompt.
        echo [*] Or manually set the path to vcvarsall.bat above.
        pause
        exit /b 1
    )
)

echo [*] Compiling icfox ESP...

REM Compile with MSVC
cl /nologo /O2 /EHsc /MD ^
    /I. ^
    /Feicfox.exe ^
    main.cpp Renderer.cpp ^
    /link ^
    user32.lib gdi32.lib ^
    d3d11.lib d3dx11.lib d3dx10.lib dxgi.lib dwmapi.lib ^
    /SUBSYSTEM:WINDOWS ^
    /OUT:icfox.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [+] Build successful! Output: icfox.exe
    echo [*] Make sure Roblox is running before launching icfox.exe
) else (
    echo.
    echo [!] Build failed with error code %ERRORLEVEL%
)

pause