# 🚗 Vehículo Telemática – Sistema de Telemetría
Integrantes: Yessetk Rodriguez,Camilo Sanchez, Samuel Cadavid y Juan Sierra
Sistema servidor-cliente para monitorear y controlar un vehículo autónomo con telemetría en tiempo real.  

## ⚙️ Requisitos
- **Servidor (C):** GCC con pthread (Linux/macOS o MinGW en Windows).  
- **Cliente Python:** Python 3.6+ con `tkinter`.  
- **Cliente Java:** JDK 8+.  

## 🔧 Instalación y Compilación

### Usando Makefile
```bash
make all        # Compila todo
make run-server # Inicia servidor
make run-python # Ejecuta cliente Python
make run-java   # Ejecuta cliente Java
Manual
bash
Copiar código
gcc -pthread server.c -o server
javac VehicleClient.java
python3 client_python.py
🚀 Uso
Iniciar servidor:

bash
Copiar código
./server 8080 vehicle.log
Ejecutar cliente (Python o Java).

Configuración del cliente:

Host: localhost (o IP del servidor)

Puerto: 8080

Usuario:

Observer: solo recibe telemetría

Admin: recibe y envía comandos (contraseña: admin123)

📡 Protocolo
Autenticación
ruby
Copiar código
AUTH:ADMIN:password
AUTH:OBSERVER:
Telemetría (cada 10s)
less
Copiar código
TELEMETRY:speed,battery,temp,direction
Ejemplo: TELEMETRY:45.5,87,23.2,FORWARD
Comandos (solo Admin)
makefile
Copiar código
CMD:SPEED_UP
CMD:SLOW_DOWN
CMD:TURN_LEFT
CMD:TURN_RIGHT
CMD:LIST_CLIENTS
📂 Estructura
bash
Copiar código
server.c / server.h   # Servidor
client_python.py      # Cliente Python
VehicleClient.java    # Cliente Java
Makefile              # Compilación
protocol.md           # Protocolo
README.md             # Documentación
🔐 Seguridad
Contraseña por defecto de administrador: admin123 (cambiar en producción).

Comunicación en texto plano (considerar TLS para entornos reales).
