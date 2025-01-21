@REM CMake build
md build
cd build && cmake .. 


@echo off
setlocal enabledelayedexpansion

:: 获取当前目录
set "CURRENT_DIR=%~dp0"
set "SOURCE_DIR=%CURRENT_DIR%"
set "TARGET_DIR=%CURRENT_DIR%include\"

:: 确保目标目录存在
if not exist "%TARGET_DIR%" (
    mkdir "%TARGET_DIR%"
)

:: 复制当前目录及其子目录中的所有头文件到目标目录
for /R "%SOURCE_DIR%" %%f in (*.h) do (
    if "%%~dpf" neq "%TARGET_DIR%" (
        copy "%%f" "%TARGET_DIR%" >nul
		echo "Coping Files : " %%f
    )
)

echo "Copy Successfully"

endlocal

:: 进入构建目录并运行 CMake
echo Generating Visual Studio solution...
cmake -G "Visual Studio 15 2017" ..

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