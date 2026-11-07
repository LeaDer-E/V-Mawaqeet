@echo off
setlocal
cd /d "%~dp0"

set "PATH=C:\Qt\Tools\mingw1510_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\CMake_64\bin;C:\Qt\6.12.0\mingw_64\bin;%PATH%"

if exist build rmdir /s /q build
if exist dist\V-Mawaqeet rmdir /s /q dist\V-Mawaqeet

cmake -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.12.0/mingw_64" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1

cmake --build build --parallel
if errorlevel 1 exit /b 1

mkdir dist\V-Mawaqeet
copy /y build\V-Mawaqeet.exe dist\V-Mawaqeet\ >nul
xcopy /e /i /y assets dist\V-Mawaqeet\assets >nul

windeployqt --no-translations --no-opengl-sw dist\V-Mawaqeet\V-Mawaqeet.exe

echo.
echo ============================================================
echo Clean Portable Build Ready in: dist\V-Mawaqeet\
echo ============================================================
