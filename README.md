# 🚗 Sistema de Telemetría para Vehículo Autónomo

Sistema cliente-servidor para monitoreo en tiempo real de vehículos autónomos con telemetría avanzada. Implementado con servidor en C y clientes en Java y Python.

## 👥 Integrantes del Proyecto

- **Yessetk Rodriguez**
- **Camilo Sanchez**
- **Samuel Cadavid**
- **Juan Sierra**
- Link del video de Youtube : https://www.youtube.com/watch?v=cIQIHxKmFew
---
##  Descripción

Este proyecto implementa un sistema de telemetría que permite:

- 📡 **Monitoreo en tiempo real** de datos del vehículo (velocidad, temperatura, coordenadas GPS)
- 🔐 **Autenticación segura** de clientes mediante login
- 📊 **Registro de eventos** en archivos de log
- 🌐 **Comunicación cliente-servidor** mediante sockets TCP/IP
- 🔄 **Múltiples clientes** conectados simultáneamente

---

# Compilacion manual

## MinGW/MSYS 
gcc -o server.exe server.c -lws2_32 -lpthread
## Cliente Java
javac VehicleClient.java
## Cliente Python
python3 -m py_compile client_python.py
## Iniciar el servidor
./server.exe 8080 vehicle.log
## Cuenta de Administrador
Usuario:admin
Contraseña: admin123



