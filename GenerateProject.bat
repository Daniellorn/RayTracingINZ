@echo off

echo Building Assimp
python Scripts\BuildDependencies.py

if %errorlevel% neq 0 (
	echo Python script error
	pause
	exit /b %errorlevel%
)

echo Generating solution
premake\premake5.exe vs2022

if %errorlevel% neq 0 (
	echo Premake script error
	pause
	exit /b %errorlevel%
)

echo.
echo Done
PAUSE