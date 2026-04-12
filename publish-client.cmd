@echo off
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0publish-client.ps1" %*
exit /b %ERRORLEVEL%
