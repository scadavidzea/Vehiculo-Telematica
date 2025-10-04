# Instrucciones de Compilación - Sistema Telemetría Vehículo

## ✅ Estado Actual

### COMPLETADO:
- ✅ **Cliente Python**: Compilado y verificado
- ✅ **Código corregido**: Todos los errores de sintaxis solucionados
- ✅ **Compatibilidad Windows**: Código adaptado para Winsock

### PENDIENTE:
- ⚠️ **Servidor C**: Requiere GCC o Visual Studio
- ⚠️ **Cliente Java**: Requiere JDK

## 🔧 Pasos para Completar la Compilación

### 1. Instalar Herramientas de Desarrollo

#### Opción A: MinGW-w64 (Recomendado)
```bash
# Descargar desde: https://www.mingw-w64.org/downloads/
# O usar MSYS2: https://www.msys2.org/

# Después de instalar, abrir terminal y ejecutar:
gcc -Wall -Wextra -std=c99 -pthread server.c -o server.exe -lws2_32
```

#### Opción B: Visual Studio Build Tools
```cmd
# Instalar desde: https://visualstudio.microsoft.com/visual-cpp-build-tools/
# Abrir "Developer Command Prompt" y ejecutar:
cl server.c /Fe:server.exe ws2_32.lib
```

### 2. Compilar Cliente Java
```bash
# Instalar JDK desde: https://adoptopenjdk.net/
# Ejecutar:
javac VehicleClient.java
```

## 🚀 Comandos de Compilación Final

### Con MinGW:
```bash
# Servidor C
gcc -Wall -Wextra -std=c99 -pthread server.c -o server.exe -lws2_32

# Cliente Java
javac VehicleClient.java

# Cliente Python (ya verificado)
python -m py_compile client_python.py
```

### Con Visual Studio:
```cmd
# Servidor C
cl server.c /Fe:server.exe ws2_32.lib

# Cliente Java
javac VehicleClient.java
```

## 🧪 Verificar Compilación

```bash
# Verificar que los archivos ejecutables existen
ls -la server.exe VehicleClient.class

# Probar servidor (en segundo plano)
./server.exe 8080 test.log

# Probar cliente Python
python client_python.py

# Probar cliente Java
java VehicleClient
```

## 🐛 Solución de Problemas

### Error: "gcc: command not found"
- Instalar MinGW-w64 o usar Visual Studio
- Verificar que GCC esté en el PATH del sistema

### Error: "javac: command not found"
- Instalar Java JDK
- Agregar Java bin al PATH del sistema

### Error de compilación con pthread
- En MinGW, usar: `-pthread -lws2_32`
- En Visual Studio, pthread no es necesario

### Error de Winsock
- Verificar que `-lws2_32` esté incluido en el comando de compilación
- En Visual Studio, usar `ws2_32.lib`

## 📋 Correcciones Aplicadas

1. **Función log_message**: Declaración corregida para soportar formato variable
2. **setsockopt**: Adaptado para usar `char*` en Windows vs `int*` en Linux
3. **sleep()**: Reemplazado por `Sleep()` en Windows (milisegundos vs segundos)
4. **Pragma comment**: Removido para evitar warnings en GCC

## 🎯 Próximos Pasos

1. Instalar herramientas de desarrollo (MinGW o Visual Studio)
2. Ejecutar comandos de compilación
3. Probar el sistema completo:
   - Iniciar servidor: `./server.exe 8080 vehicle.log`
   - Conectar clientes Python y Java
   - Verificar telemetría y comandos

## 📞 Ayuda Adicional

Si encuentras problemas:
1. Verificar versiones de herramientas instaladas
2. Revisar PATH del sistema
3. Consultar logs de compilación para errores específicos

¡El código está listo y las correcciones aplicadas! Solo falta tener las herramientas de desarrollo instaladas.