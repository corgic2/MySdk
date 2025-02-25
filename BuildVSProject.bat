@REM CMake build
md build
cd build && cmake .. 

@echo off
setlocal enabledelayedexpansion

:: 获取当前目录
set "CURRENT_DIR=%~dp0"
set "SOURCE_DIR=%CURRENT_DIR%"

:: 进入构建目录并运行 CMake
echo Generating Visual Studio solution...
cmake -DCMAKE_CONFIGURATION_TYPES=Release .. -G "Visual Studio 17 2022" 

:: 检查 CMake 是否成功
if %ERRORLEVEL% neq 0 (
    echo CMake failed to generate the solution.
    pause
    exit /b 1
)

echo Solution generated successfully.

:: 编译项目
echo Building the project...
cmake --build . --config Release

:: 检查编译是否成功
if %ERRORLEVEL% neq 0 (
    echo Build failed.
    pause
    exit /b 1
)

echo Build completed successfully.

pause