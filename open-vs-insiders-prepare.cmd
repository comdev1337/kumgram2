@echo off
setlocal

if /i not "%~1" == "--inside" (
	start "Telegram VS Insiders Prepare" "%ComSpec%" /k call "%~f0" --inside
	exit /b
)

set "VS_VCVARS=C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"
set "PREPARE_DIR=%~dp0Telegram\build\prepare"

if not exist "%VS_VCVARS%" (
	echo Missing Visual Studio vcvars script:
	echo %VS_VCVARS%
	exit /b 1
)

if not exist "%PREPARE_DIR%\win.bat" (
	echo Missing prepare script:
	echo %PREPARE_DIR%\win.bat
	exit /b 1
)

call "%VS_VCVARS%" -vcvars_ver=14.44
if %errorlevel% neq 0 exit /b %errorlevel%

cd /d "%PREPARE_DIR%"
if %errorlevel% neq 0 exit /b %errorlevel%

call win.bat
