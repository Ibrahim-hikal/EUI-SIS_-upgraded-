@echo off
SETLOCAL EnableDelayedExpansion

set BUILD_DIR=build
set EXE_NAME=EUI_SIS.exe

echo [1/4] Cleaning up old build artifacts...
if exist %BUILD_DIR% rd /s /q %BUILD_DIR%
mkdir %BUILD_DIR%

echo.
echo [2/4] Configuring project with CMake...
:: Your CMakeLists.txt will auto-generate the icon file during this step
cd %BUILD_DIR%
cmake -DCMAKE_BUILD_TYPE=Release ..
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [3/4] Building the executable...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b %ERRORLEVEL%
)
cd ..

echo.
echo [4/4] Moving the executable to the main folder...
:: Find where CMake put the EUI_SIS.exe and bring it out to the root folder
if exist "%BUILD_DIR%\Release\%EXE_NAME%" (
    copy /Y "%BUILD_DIR%\Release\%EXE_NAME%" "%EXE_NAME%" >nul
) else if exist "%BUILD_DIR%\%EXE_NAME%" (
    copy /Y "%BUILD_DIR%\%EXE_NAME%" "%EXE_NAME%" >nul
)

echo.
echo Checking for required DLL...
if not exist "slint_cpp.dll" (
    echo [WARNING] slint_cpp.dll was not found in the main folder!
    echo Please copy it from your Slint bin folder and paste it here so the app can launch.
) else (
    echo slint_cpp.dll is present.
)

echo.
echo ==========================================
echo SUCCESS! 
echo Your executable (%EXE_NAME%) is now in the main folder next to main.cpp.
echo ==========================================
pause