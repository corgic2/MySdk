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

pause