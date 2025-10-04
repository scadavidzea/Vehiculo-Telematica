#!/usr/bin/env python3

import socket
import threading
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import json
import time
from datetime import datetime

class VehicleClient:
    def __init__(self):
        self.socket = None
        self.connected = False
        self.client_type = None
        self.running = False

        # Datos de telemetría
        self.speed = 0.0
        self.battery = 100
        self.temperature = 22.5
        self.direction = "FORWARD"

        # Configurar interfaz gráfica
        self.setup_gui()

    def setup_gui(self):
        self.root = tk.Tk()
        self.root.title("Cliente Vehículo Autónomo")
        self.root.geometry("600x700")
        self.root.resizable(False, False)

        # Frame de conexión
        connection_frame = ttk.LabelFrame(self.root, text="Conexión")
        connection_frame.pack(fill="x", padx=10, pady=5)

        # Host y puerto
        ttk.Label(connection_frame, text="Host:").grid(row=0, column=0, padx=5, pady=5, sticky="e")
        self.host_entry = ttk.Entry(connection_frame, width=20)
        self.host_entry.insert(0, "localhost")
        self.host_entry.grid(row=0, column=1, padx=5, pady=5)

        ttk.Label(connection_frame, text="Puerto:").grid(row=0, column=2, padx=5, pady=5, sticky="e")
        self.port_entry = ttk.Entry(connection_frame, width=10)
        self.port_entry.insert(0, "8080")
        self.port_entry.grid(row=0, column=3, padx=5, pady=5)

        # Tipo de cliente
        ttk.Label(connection_frame, text="Tipo:").grid(row=1, column=0, padx=5, pady=5, sticky="e")
        self.client_type_var = tk.StringVar(value="OBSERVER")
        type_combo = ttk.Combobox(connection_frame, textvariable=self.client_type_var,
                                  values=["OBSERVER", "ADMIN"], state="readonly", width=15)
        type_combo.grid(row=1, column=1, padx=5, pady=5)

        # Contraseña para admin
        ttk.Label(connection_frame, text="Contraseña:").grid(row=1, column=2, padx=5, pady=5, sticky="e")
        self.password_entry = ttk.Entry(connection_frame, show="*", width=15)
        self.password_entry.grid(row=1, column=3, padx=5, pady=5)

        # Botones de conexión
        button_frame = ttk.Frame(connection_frame)
        button_frame.grid(row=2, column=0, columnspan=4, pady=10)

        self.connect_btn = ttk.Button(button_frame, text="Conectar", command=self.connect)
        self.connect_btn.pack(side="left", padx=5)

        self.disconnect_btn = ttk.Button(button_frame, text="Desconectar",
                                       command=self.disconnect, state="disabled")
        self.disconnect_btn.pack(side="left", padx=5)

        # Frame de telemetría
        telemetry_frame = ttk.LabelFrame(self.root, text="Telemetría del Vehículo")
        telemetry_frame.pack(fill="x", padx=10, pady=5)

        # Velocidad
        speed_frame = ttk.Frame(telemetry_frame)
        speed_frame.pack(fill="x", padx=10, pady=5)
        ttk.Label(speed_frame, text="Velocidad:", font=("Arial", 12, "bold")).pack(side="left")
        self.speed_label = ttk.Label(speed_frame, text="0.0 km/h", font=("Arial", 16))
        self.speed_label.pack(side="right")

        # Batería
        battery_frame = ttk.Frame(telemetry_frame)
        battery_frame.pack(fill="x", padx=10, pady=5)
        ttk.Label(battery_frame, text="Batería:", font=("Arial", 12, "bold")).pack(side="left")
        self.battery_label = ttk.Label(battery_frame, text="100%", font=("Arial", 16))
        self.battery_label.pack(side="right")

        # Progreso de batería
        self.battery_progress = ttk.Progressbar(telemetry_frame, length=300, mode='determinate')
        self.battery_progress.pack(pady=5)
        self.battery_progress['value'] = 100

        # Temperatura
        temp_frame = ttk.Frame(telemetry_frame)
        temp_frame.pack(fill="x", padx=10, pady=5)
        ttk.Label(temp_frame, text="Temperatura:", font=("Arial", 12, "bold")).pack(side="left")
        self.temp_label = ttk.Label(temp_frame, text="22.5°C", font=("Arial", 16))
        self.temp_label.pack(side="right")

        # Dirección
        direction_frame = ttk.Frame(telemetry_frame)
        direction_frame.pack(fill="x", padx=10, pady=5)
        ttk.Label(direction_frame, text="Dirección:", font=("Arial", 12, "bold")).pack(side="left")
        self.direction_label = ttk.Label(direction_frame, text="FORWARD", font=("Arial", 16))
        self.direction_label.pack(side="right")

        # Frame de controles (solo para admin)
        self.controls_frame = ttk.LabelFrame(self.root, text="Controles (Solo Administrador)")
        self.controls_frame.pack(fill="x", padx=10, pady=5)

        # Botones de control
        controls_grid = ttk.Frame(self.controls_frame)
        controls_grid.pack(pady=10)

        self.speed_up_btn = ttk.Button(controls_grid, text="Acelerar",
                                     command=lambda: self.send_command("SPEED_UP"), state="disabled")
        self.speed_up_btn.grid(row=0, column=0, padx=5, pady=5)

        self.slow_down_btn = ttk.Button(controls_grid, text="Frenar",
                                      command=lambda: self.send_command("SLOW_DOWN"), state="disabled")
        self.slow_down_btn.grid(row=0, column=1, padx=5, pady=5)

        self.turn_left_btn = ttk.Button(controls_grid, text="Girar Izquierda",
                                      command=lambda: self.send_command("TURN_LEFT"), state="disabled")
        self.turn_left_btn.grid(row=1, column=0, padx=5, pady=5)

        self.turn_right_btn = ttk.Button(controls_grid, text="Girar Derecha",
                                       command=lambda: self.send_command("TURN_RIGHT"), state="disabled")
        self.turn_right_btn.grid(row=1, column=1, padx=5, pady=5)

        self.list_clients_btn = ttk.Button(controls_grid, text="Listar Clientes",
                                         command=lambda: self.send_command("LIST_CLIENTS"), state="disabled")
        self.list_clients_btn.grid(row=2, column=0, columnspan=2, padx=5, pady=5)

        # Frame de estado y logs
        status_frame = ttk.LabelFrame(self.root, text="Estado y Logs")
        status_frame.pack(fill="both", expand=True, padx=10, pady=5)

        # Estado de conexión
        self.status_label = ttk.Label(status_frame, text="Estado: Desconectado",
                                    font=("Arial", 10, "bold"))
        self.status_label.pack(anchor="w", padx=5, pady=2)

        # Log de mensajes
        self.log_text = scrolledtext.ScrolledText(status_frame, height=8, width=70)
        self.log_text.pack(fill="both", expand=True, padx=5, pady=5)

        # Configurar cierre de ventana
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)

    def connect(self):
        try:
            host = self.host_entry.get()
            port = int(self.port_entry.get())
            client_type = self.client_type_var.get()
            password = self.password_entry.get()

            # Crear socket
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((host, port))

            # Solo enviar autenticación si es ADMIN
            if client_type == "ADMIN":
                auth_msg = f"AUTH:ADMIN:{password}\n"
                self.socket.send(auth_msg.encode())

                # Esperar respuesta de autenticación
                response = self.socket.recv(1024).decode().strip()

                if not response.startswith("AUTH:OK"):
                    self.socket.close()
                    messagebox.showerror("Error", f"Error de autenticación: {response}")
                    return

            # Conexión exitosa
            self.connected = True
            self.client_type = client_type
            self.running = True

            # Actualizar UI
            self.connect_btn.config(state="disabled")
            self.disconnect_btn.config(state="normal")
            self.status_label.config(text=f"Estado: Conectado como {client_type}")

            if client_type == "ADMIN":
                self.enable_admin_controls()

            # Iniciar hilo de recepción
            self.receive_thread = threading.Thread(target=self.receive_messages)
            self.receive_thread.daemon = True
            self.receive_thread.start()

            self.log_message(f"Conectado exitosamente como {client_type}")

        except Exception as e:
            if self.socket:
                self.socket.close()
            messagebox.showerror("Error", f"Error de conexión: {str(e)}")

    def disconnect(self):
        try:
            if self.connected and self.socket:
                self.socket.send("DISCONNECT:\n".encode())

        except:
            pass

        self.cleanup_connection()

    def cleanup_connection(self):
        self.running = False
        self.connected = False

        if self.socket:
            self.socket.close()
            self.socket = None

        # Actualizar UI
        self.connect_btn.config(state="normal")
        self.disconnect_btn.config(state="disabled")
        self.status_label.config(text="Estado: Desconectado")
        self.disable_admin_controls()

        self.log_message("Desconectado del servidor")

    def enable_admin_controls(self):
        self.speed_up_btn.config(state="normal")
        self.slow_down_btn.config(state="normal")
        self.turn_left_btn.config(state="normal")
        self.turn_right_btn.config(state="normal")
        self.list_clients_btn.config(state="normal")

    def disable_admin_controls(self):
        self.speed_up_btn.config(state="disabled")
        self.slow_down_btn.config(state="disabled")
        self.turn_left_btn.config(state="disabled")
        self.turn_right_btn.config(state="disabled")
        self.list_clients_btn.config(state="disabled")

    def send_command(self, command):
        if self.connected and self.socket:
            try:
                message = f"CMD:{command}\n"
                self.socket.send(message.encode())
                self.log_message(f"Comando enviado: {command}")
            except Exception as e:
                self.log_message(f"Error enviando comando: {str(e)}")

    def receive_messages(self):
        while self.running and self.connected:
            try:
                data = self.socket.recv(1024).decode().strip()
                if not data:
                    break

                self.process_message(data)

            except Exception as e:
                if self.running:
                    self.log_message(f"Error recibiendo datos: {str(e)}")
                break

        self.root.after(0, self.cleanup_connection)

    def process_message(self, message):
        self.log_message(f"Recibido: {message}")

        if message.startswith("TELEMETRY:"):
            self.process_telemetry(message[10:])
        elif message.startswith("CMD:OK:"):
            self.log_message(f"Comando ejecutado: {message[7:]}")
        elif message.startswith("CMD:FAIL:"):
            self.log_message(f"Error en comando: {message[9:]}")
        elif message.startswith("CLIENTS:"):
            self.process_client_list(message[8:])
        elif message.startswith("ERROR:"):
            self.log_message(f"Error del servidor: {message[6:]}")

    def process_telemetry(self, data):
        try:
            parts = data.split(',')
            if len(parts) == 4:
                self.speed = float(parts[0])
                self.battery = int(parts[1])
                self.temperature = float(parts[2])
                self.direction = parts[3]

                # Actualizar UI en hilo principal
                self.root.after(0, self.update_telemetry_display)

        except Exception as e:
            self.log_message(f"Error procesando telemetría: {str(e)}")

    def process_client_list(self, data):
        parts = data.split(',')
        if len(parts) > 0:
            count = parts[0]
            clients = parts[1:] if len(parts) > 1 else []

            client_info = f"Clientes conectados: {count}\n"
            for client in clients:
                if client:
                    client_info += f"  - {client}\n"

            messagebox.showinfo("Lista de Clientes", client_info)

    def update_telemetry_display(self):
        self.speed_label.config(text=f"{self.speed:.1f} km/h")
        self.battery_label.config(text=f"{self.battery}%")
        self.battery_progress['value'] = self.battery
        self.temp_label.config(text=f"{self.temperature:.1f}°C")
        self.direction_label.config(text=self.direction)

        # Cambiar color según valores
        if self.battery < 20:
            self.battery_label.config(foreground="red")
        elif self.battery < 50:
            self.battery_label.config(foreground="orange")
        else:
            self.battery_label.config(foreground="green")

        if self.speed > 80:
            self.speed_label.config(foreground="red")
        elif self.speed > 60:
            self.speed_label.config(foreground="orange")
        else:
            self.speed_label.config(foreground="black")

    def log_message(self, message):
        timestamp = datetime.now().strftime("%H:%M:%S")
        log_entry = f"[{timestamp}] {message}\n"

        self.root.after(0, lambda: self._append_log(log_entry))

    def _append_log(self, log_entry):
        self.log_text.insert(tk.END, log_entry)
        self.log_text.see(tk.END)

    def on_closing(self):
        if self.connected:
            self.disconnect()
        self.root.destroy()

    def run(self):
        self.root.mainloop()

if __name__ == "__main__":
    client = VehicleClient()
    client.run()