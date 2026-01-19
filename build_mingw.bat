@echo off
echo Building Black-Scholes Python Module with MinGW...

REM Check for g++
where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: g++ not found. Please add C:\msys64\mingw64\bin to PATH
    pause
    exit /b 1
)

REM Get Python include directory
for /f "tokens=*" %%i in ('python -c "import sysconfig; print(sysconfig.get_path('include'))"') do set PYTHON_INCLUDE=%%i

REM Get Python library directory  
for /f "tokens=*" %%i in ('python -c "import sysconfig; print(sysconfig.get_config_var('LIBDIR'))"') do set PYTHON_LIB=%%i

REM Get pybind11 include directory
for /f "tokens=*" %%i in ('python -c "import pybind11; print(pybind11.get_include())"') do set PYBIND11_INCLUDE=%%i

echo Python Include: %PYTHON_INCLUDE%
echo Pybind11 Include: %PYBIND11_INCLUDE%

REM Compile
g++ -O3 -Wall -shared -std=c++17 ^
    -I"%PYTHON_INCLUDE%" ^
    -I"%PYBIND11_INCLUDE%" ^
    -Isrc ^
    src/bindings.cpp src/BlackScholes.cpp ^
    -o black_scholes.pyd ^
    -L"%PYTHON_LIB%" ^
    -lpython314

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Build successful! 
    echo Module: black_scholes.pyd
    echo ========================================
    echo.
    echo Test it with: python -c "import black_scholes as bs; print(bs.call_price(100, 100, 0.05, 1.0, 0.2))"
) else (
    echo Build failed!
)

pause
