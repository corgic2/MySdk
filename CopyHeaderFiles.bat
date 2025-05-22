@Echo off
echo Build completed successfully.

set "CURRENT_DIR=%~dp0"
set "TARGET_DIR=%~dp0..\include\"

:: 确保目标目录存在
if not exist "%TARGET_DIR%" (
    mkdir "%TARGET_DIR%"
)

:: 使用xcopy保持目录结构
xcopy "%SOURCE_DIR%*.h" "%TARGET_DIR%" /S /I /Y 

echo "Copy Successfully"
pause