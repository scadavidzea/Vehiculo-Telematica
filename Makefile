# Makefile para Proyecto Vehículo Autónomo con Telemetría

# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread
LDFLAGS = -pthread

# Directorios
SRCDIR = .
OBJDIR = obj
BINDIR = bin

# Archivos fuente
SOURCES = $(SRCDIR)/server.c
OBJECTS = $(OBJDIR)/server.o
TARGET = $(BINDIR)/server

# Archivos de Java
JAVA_SRC = VehicleClient.java
JAVA_CLASS = VehicleClient.class

# Regla principal
.PHONY: all clean server java python test help

all: server java

# Crear directorios si no existen
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Compilar servidor en C
server: $(TARGET)

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Servidor compilado exitosamente: $(TARGET)"

$(OBJDIR)/server.o: $(SRCDIR)/server.c $(SRCDIR)/server.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar cliente Java
java: $(JAVA_CLASS)

$(JAVA_CLASS): $(JAVA_SRC)
	javac $(JAVA_SRC)
	@echo "Cliente Java compilado exitosamente: $(JAVA_CLASS)"

# Verificar cliente Python (sintaxis)
python:
	@if command -v python3 >/dev/null 2>&1; then \
		python3 -m py_compile client_python.py; \
		echo "Cliente Python verificado exitosamente"; \
	else \
		echo "Python3 no encontrado. Instale Python3 para verificar el cliente."; \
	fi

# Ejecutar servidor (ejemplo)
run-server: server
	@echo "Iniciando servidor en puerto 8080..."
	@echo "Logs se guardarán en server.log"
	$(TARGET) 8080 server.log

# Ejecutar cliente Python
run-python: python
	@if command -v python3 >/dev/null 2>&1; then \
		python3 client_python.py; \
	else \
		echo "Python3 no encontrado."; \
	fi

# Ejecutar cliente Java
run-java: java
	java VehicleClient

# Pruebas básicas
test: server
	@echo "Ejecutando pruebas básicas..."
	@echo "1. Verificando que el servidor se puede compilar..."
	@if [ -f $(TARGET) ]; then \
		echo "   ✓ Servidor compilado correctamente"; \
	else \
		echo "   ✗ Error: Servidor no compilado"; \
		exit 1; \
	fi

	@echo "2. Verificando cliente Java..."
	@if [ -f $(JAVA_CLASS) ]; then \
		echo "   ✓ Cliente Java compilado correctamente"; \
	else \
		echo "   ✗ Error: Cliente Java no compilado"; \
	fi

	@echo "3. Verificando cliente Python..."
	@if [ -f client_python.py ]; then \
		echo "   ✓ Cliente Python encontrado"; \
	else \
		echo "   ✗ Error: Cliente Python no encontrado"; \
	fi

	@echo "Todas las verificaciones completadas."

# Limpiar archivos compilados
clean:
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)
	rm -f *.class
	rm -f *.pyc
	rm -f __pycache__/*
	rmdir __pycache__ 2>/dev/null || true
	@echo "Archivos compilados eliminados"

# Limpiar logs
clean-logs:
	rm -f *.log
	@echo "Archivos de log eliminados"

# Limpiar todo
clean-all: clean clean-logs

# Instalar dependencias (para sistemas Unix-like)
install-deps:
	@echo "Verificando dependencias..."
	@echo "Para el servidor en C:"
	@echo "  - GCC: $(shell gcc --version 2>/dev/null | head -1 || echo 'No instalado')"
	@echo "  - pthread: $(shell echo '#include <pthread.h>' | gcc -E - >/dev/null 2>&1 && echo 'Disponible' || echo 'No disponible')"
	@echo ""
	@echo "Para el cliente Java:"
	@echo "  - Java: $(shell java -version 2>&1 | head -1 || echo 'No instalado')"
	@echo "  - javac: $(shell javac -version 2>&1 || echo 'No instalado')"
	@echo ""
	@echo "Para el cliente Python:"
	@echo "  - Python3: $(shell python3 --version 2>/dev/null || echo 'No instalado')"
	@echo "  - tkinter: $(shell python3 -c 'import tkinter' 2>/dev/null && echo 'Disponible' || echo 'No disponible')"

# Crear paquete de distribución
package: clean all
	@echo "Creando paquete de distribución..."
	mkdir -p dist/vehiculo-telematica
	cp $(TARGET) dist/vehiculo-telematica/
	cp $(JAVA_CLASS) dist/vehiculo-telematica/
	cp client_python.py dist/vehiculo-telematica/
	cp protocol.md dist/vehiculo-telematica/
	cp README.md dist/vehiculo-telematica/ 2>/dev/null || true
	cp Makefile dist/vehiculo-telematica/
	@echo "Paquete creado en: dist/vehiculo-telematica/"

# Ayuda
help:
	@echo "Makefile para Proyecto Vehículo Autónomo con Telemetría"
	@echo ""
	@echo "Objetivos disponibles:"
	@echo "  all          - Compilar servidor y cliente Java"
	@echo "  server       - Compilar solo el servidor en C"
	@echo "  java         - Compilar solo el cliente Java"
	@echo "  python       - Verificar sintaxis del cliente Python"
	@echo ""
	@echo "Ejecución:"
	@echo "  run-server   - Ejecutar servidor en puerto 8080"
	@echo "  run-python   - Ejecutar cliente Python"
	@echo "  run-java     - Ejecutar cliente Java"
	@echo ""
	@echo "Mantenimiento:"
	@echo "  test         - Ejecutar pruebas básicas"
	@echo "  clean        - Limpiar archivos compilados"
	@echo "  clean-logs   - Limpiar archivos de log"
	@echo "  clean-all    - Limpiar todo"
	@echo ""
	@echo "Utilidades:"
	@echo "  install-deps - Verificar dependencias del sistema"
	@echo "  package      - Crear paquete de distribución"
	@echo "  help         - Mostrar esta ayuda"
	@echo ""
	@echo "Ejemplos:"
	@echo "  make server && make run-server"
	@echo "  make java && make run-java"
	@echo "  make all && make test"