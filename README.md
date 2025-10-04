# Sistema de Telemetría para Vehículo Autónomo

Este proyecto implementa un sistema servidor-cliente para monitorear y controlar un vehículo autónomo mediante telemetría en tiempo real.

## 🚗 Características

- **Servidor en C** con soporte para múltiples clientes simultáneos usando threads
- **Clientes en Python y Java** con interfaces gráficas
- **Telemetría en tiempo real** (velocidad, batería, temperatura, dirección)
- **Dos tipos de usuarios**: Administrador y Observador
- **Sistema de logging** completo
- **Protocolo de comunicación** robusto basado en TCP

## 📋 Requisitos del Sistema

### Para el Servidor (C)
- GCC con soporte para pthread
- Sistema operativo Unix-like (Linux, macOS) o Windows con MinGW

### Para el Cliente Python
- Python 3.6 o superior
- tkinter (incluido en la mayoría de instalaciones de Python)

### Para el Cliente Java
- Java JDK 8 o superior
- Compilador javac

## 🔧 Compilación e Instalación

### Usando Makefile (Recomendado)

```bash
# Compilar todo
make all

# Solo servidor
make server

# Solo cliente Java
make java

# Verificar cliente Python
make python

# Verificar dependencias
make install-deps
```

### Compilación Manual

#### Servidor en C
```bash
gcc -Wall -Wextra -std=c99 -pthread server.c -o server
```

#### Cliente Java
```bash
javac VehicleClient.java
```

#### Cliente Python
```bash
python3 -m py_compile client_python.py
```

## 🚀 Uso

### 1. Iniciar el Servidor

```bash
# Usando Makefile
make run-server

# Manual
./server <puerto> <archivo_logs>
./server 8080 vehicle.log
```

### 2. Ejecutar Clientes

#### Cliente Python
```bash
# Usando Makefile
make run-python

# Manual
python3 client_python.py
```

#### Cliente Java
```bash
# Usando Makefile
make run-java

# Manual
java VehicleClient
```

### 3. Configuración de Clientes

1. **Host**: Dirección IP del servidor (por defecto: localhost)
2. **Puerto**: Puerto del servidor (por defecto: 8080)
3. **Tipo de Usuario**:
   - **Observer**: Solo recibe telemetría
   - **Admin**: Recibe telemetría y puede enviar comandos
4. **Contraseña**: Para administrador (por defecto: admin123)

## 📡 Protocolo de Comunicación

### Autenticación
```
Cliente → Servidor: AUTH:ADMIN:password
Cliente → Servidor: AUTH:OBSERVER:
Servidor → Cliente: AUTH:OK:ADMIN
Servidor → Cliente: AUTH:FAIL:reason
```

### Telemetría (cada 10 segundos)
```
Servidor → Clientes: TELEMETRY:speed,battery,temperature,direction
Ejemplo: TELEMETRY:45.5,87,23.2,FORWARD
```

### Comandos de Control (Solo Admin)
```
Cliente → Servidor: CMD:SPEED_UP
Cliente → Servidor: CMD:SLOW_DOWN
Cliente → Servidor: CMD:TURN_LEFT
Cliente → Servidor: CMD:TURN_RIGHT
Cliente → Servidor: CMD:LIST_CLIENTS
```

## 🎮 Funcionalidades

### Servidor
- ✅ Manejo de múltiples clientes con threads
- ✅ Envío de telemetría cada 10 segundos
- ✅ Autenticación de usuarios
- ✅ Procesamiento de comandos
- ✅ Logging en consola y archivo
- ✅ Lista de clientes conectados
- ✅ Manejo de desconexiones

### Cliente Python
- ✅ Interfaz gráfica con tkinter
- ✅ Visualización de telemetría en tiempo real
- ✅ Controles para administrador
- ✅ Sistema de logs integrado
- ✅ Indicadores visuales (colores para batería/velocidad)

### Cliente Java
- ✅ Interfaz gráfica con Swing
- ✅ Mismas funcionalidades que el cliente Python
- ✅ Look and Feel nativo del sistema
- ✅ Manejo robusto de hilos

## 🧪 Pruebas

```bash
# Ejecutar pruebas básicas
make test

# Limpiar archivos compilados
make clean

# Limpiar logs
make clean-logs
```

## 📁 Estructura del Proyecto

```
VEHICULO TELEMATICA/
├── server.h              # Header del servidor
├── server.c              # Implementación del servidor
├── client_python.py      # Cliente en Python
├── VehicleClient.java    # Cliente en Java
├── protocol.md          # Documentación del protocolo
├── Makefile             # Automatización de compilación
└── README.md            # Esta documentación
```

## 🔍 Solución de Problemas

### Error: "Permission denied" al ejecutar servidor
```bash
chmod +x server
```

### Error: "Address already in use"
- Cambiar el puerto o esperar unos segundos
- Verificar procesos en ejecución: `lsof -i :8080`

### Cliente no puede conectar
- Verificar que el servidor esté ejecutándose
- Comprobar firewall y configuración de red
- Revisar logs del servidor

### Error de compilación con pthread
```bash
# Instalar build-essential en Ubuntu/Debian
sudo apt-get install build-essential

# En macOS, instalar Xcode Command Line Tools
xcode-select --install
```

## 📈 Datos de Telemetría

| Parámetro | Rango | Unidad |
|-----------|-------|--------|
| Velocidad | 0-120 | km/h |
| Batería | 0-100 | % |
| Temperatura | -20 a 80 | °C |
| Dirección | FORWARD, LEFT, RIGHT, BACKWARD | - |

## 🔐 Seguridad

- Contraseña de administrador: `admin123` (cambiar en producción)
- Comunicación en texto plano (considerar TLS para producción)
- Logging de todas las acciones de administrador

## 🛠 Desarrollo

### Compilación en Desarrollo
```bash
# Modo debug con símbolos
gcc -Wall -Wextra -std=c99 -pthread -g -DDEBUG server.c -o server_debug
```

### Agregar Nuevos Comandos
1. Modificar `handle_command()` en `server.c`
2. Actualizar protocolo en `protocol.md`
3. Agregar botones en interfaces de cliente

## 📝 Licencia

Este proyecto es de código abierto y está disponible bajo la licencia MIT.

## 👥 Contribución

Para contribuir al proyecto:
1. Fork el repositorio
2. Crear una rama para tu feature
3. Hacer commit de los cambios
4. Crear un Pull Request

## 📞 Soporte

Para reportar bugs o solicitar features, crear un issue en el repositorio del proyecto.