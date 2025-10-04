# Protocolo de Comunicación - Vehículo Autónomo

## Tipo de Socket
- **SOCK_STREAM (TCP)**: Para comandos y autenticación (requiere confiabilidad)
- **SOCK_DGRAM (UDP)**: Para telemetría (permite pérdida ocasional de paquetes)

## Formato de Mensajes
Todos los mensajes siguen el formato: `TIPO:DATOS\n`

### 1. Autenticación y Conexión

#### Cliente → Servidor
- `AUTH:ADMIN:password` - Autenticación como administrador
- `AUTH:OBSERVER:` - Conexión como observador
- `PING:` - Keepalive

#### Servidor → Cliente
- `AUTH:OK:ADMIN` - Autenticación exitosa como admin
- `AUTH:OK:OBSERVER` - Autenticación exitosa como observer
- `AUTH:FAIL:reason` - Fallo en autenticación
- `PONG:` - Respuesta a ping

### 2. Telemetría (cada 10 segundos)

#### Servidor → Todos los Clientes
- `TELEMETRY:speed,battery,temperature,direction`
  - speed: velocidad en km/h (0-120)
  - battery: nivel de batería en % (0-100)
  - temperature: temperatura en °C (-20 a 80)
  - direction: FORWARD, LEFT, RIGHT, BACKWARD

Ejemplo: `TELEMETRY:45.5,87,23.2,FORWARD`

### 3. Comandos de Control

#### Cliente Admin → Servidor
- `CMD:SPEED_UP` - Aumentar velocidad
- `CMD:SLOW_DOWN` - Reducir velocidad
- `CMD:TURN_LEFT` - Girar a la izquierda
- `CMD:TURN_RIGHT` - Girar a la derecha
- `CMD:LIST_CLIENTS` - Listar clientes conectados

#### Servidor → Cliente Admin
- `CMD:OK:command_executed` - Comando ejecutado exitosamente
- `CMD:FAIL:reason` - Error en ejecución
- `CLIENTS:count,client1_ip:port:type,client2_ip:port:type,...`

### 4. Desconexión
- `DISCONNECT:` - Cliente se desconecta
- `DISCONNECT:reason` - Servidor desconecta cliente

## Códigos de Error
- `ERROR:INVALID_FORMAT` - Formato de mensaje inválido
- `ERROR:UNAUTHORIZED` - Operación no autorizada
- `ERROR:UNKNOWN_COMMAND` - Comando desconocido
- `ERROR:CONNECTION_LOST` - Conexión perdida

## Flujo de Comunicación

1. Cliente se conecta al servidor
2. Cliente envía mensaje de autenticación
3. Servidor responde con confirmación o error
4. Si es exitoso:
   - Observer: recibe telemetría cada 10s
   - Admin: recibe telemetría + puede enviar comandos
5. Cliente puede desconectarse enviando DISCONNECT