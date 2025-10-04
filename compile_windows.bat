@echo off
REM Script de compilación para Windows
REM Requiere MinGW o Visual Studio Build Tools

echo ========================================
echo  Sistema de Telemetria - Vehiculo Autonomo
echo  Script de Compilacion para Windows
echo ========================================
echo.

REM Verificar si gcc está disponible
gcc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: GCC no encontrado.
    echo Por favor instale MinGW o use Visual Studio Developer Command Prompt
    echo.
    echo Descarga MinGW desde: https://mingw-w64.org/
    pause
    exit /b 1
)

echo 1. Compilando servidor en C...
gcc -Wall -Wextra -std=c99 -pthread server.c -o server.exe -lws2_32
if %errorlevel% neq 0 (
    echo ERROR: Fallo en la compilacion del servidor
    pause
    exit /b 1
)
echo    ✓ Servidor compilado: server.exe

echo.
echo 2. Verificando Java...
javac -version >nul 2>&1
if %errorlevel% neq 0 (
    echo ADVERTENCIA: javac no encontrado. Cliente Java no se compilará.
    echo Instale Java JDK desde: https://adoptopenjdk.net/
) else (
    echo    Compilando cliente Java...
    javac VehicleClient.java
    if %errorlevel% neq 0 (
        echo ERROR: Fallo en la compilacion del cliente Java
    ) else (
        echo    ✓ Cliente Java compilado: VehicleClient.class
    )
)

echo.
echo 3. Verificando Python...
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ADVERTENCIA: Python no encontrado en PATH
    echo Intente con python3...
    python3 --version >nul 2>&1
    if %errorlevel% neq 0 (
        echo ADVERTENCIA: python3 tampoco encontrado
        echo Instale Python desde: https://python.org/
    ) else (
        echo    ✓ Python3 encontrado
        python3 -m py_compile client_python.py
        if %errorlevel% neq 0 (
            echo ERROR: Error en sintaxis del cliente Python
        ) else (
            echo    ✓ Cliente Python verificado
        )
    )
) else (
    echo    ✓ Python encontrado
    python -m py_compile client_python.py
    if %errorlevel% neq 0 (
        echo ERROR: Error en sintaxis del cliente Python
    ) else (
        echo    ✓ Cliente Python verificado
    )
)

echo.
echo ========================================
echo  Compilación completada
echo ========================================
echo.
echo Para ejecutar:
echo   Servidor:       server.exe 8080 vehicle.log
echo   Cliente Java:   java VehicleClient
echo   Cliente Python: python client_python.py
echo.

REM Crear script de ejecución del servidor
echo @echo off > run_server.bat
echo echo Iniciando servidor en puerto 8080... >> run_server.bat
echo server.exe 8080 vehicle.log >> run_server.bat
echo pause >> run_server.bat

echo Script de ejecución creado: run_server.bat
echo.
pause