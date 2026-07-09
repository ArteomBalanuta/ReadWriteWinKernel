@echo off
setlocal

rem Copy built artifacts to the goods folder, overwriting any existing files.
rem Usage: deploy-goods.cmd [Configuration] [Platform]
rem   Configuration defaults to Debug, Platform defaults to x64.

set "CONFIG=%~1"
set "PLATFORM=%~2"

if "%CONFIG%"=="" set "CONFIG=Debug"
if "%PLATFORM%"=="" set "PLATFORM=x64"

set "DEST=C:\Users\arteo\Desktop\goods"
set "SRC=%~dp0build"

echo Deploying %CONFIG% %PLATFORM% artifacts to %DEST%
echo.

rem Driver
xcopy /Y /Q "%SRC%\driver\%PLATFORM%\%CONFIG%\dummy.sys" "%DEST%\"
xcopy /Y /Q "%SRC%\driver\%PLATFORM%\%CONFIG%\dummy.pdb" "%DEST%\"

rem Controller
xcopy /Y /Q "%SRC%\controller\%PLATFORM%\%CONFIG%\controller.exe" "%DEST%\"
xcopy /Y /Q "%SRC%\controller\%PLATFORM%\%CONFIG%\controller.pdb" "%DEST%\"

rem C# client
xcopy /Y /Q /S "%SRC%\client\%PLATFORM%\%CONFIG%\net8.0-windows\*.dll" "%DEST%\"
xcopy /Y /Q /S "%SRC%\client\%PLATFORM%\%CONFIG%\net8.0-windows\*.exe" "%DEST%\"
xcopy /Y /Q /S "%SRC%\client\%PLATFORM%\%CONFIG%\net8.0-windows\*.pdb" "%DEST%\"

echo Done.
