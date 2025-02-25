echo Build completed successfully.


set "TARGET_DIR=%CURRENT_DIR%include\"

:: 确保目标目录存在
if not exist "%TARGET_DIR%" (
    mkdir "%TARGET_DIR%"
)

:: 复制当前目录及其子目录中的所有头文件到目标目录
for /R "%SOURCE_DIR%" %%f in (*.h) do (
    if "%%~dpf" neq "%TARGET_DIR%" (
        copy /Y "%%f" "%TARGET_DIR%" >nul
		if errorlevel 1 (
			echo "Failed to copy: %%f"
		) else (
			echo "Copying Files: %%f"
		)
    )
)

echo "Copy Successfully"