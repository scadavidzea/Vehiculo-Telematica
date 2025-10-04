#include "server.h"
#include <stdarg.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define close closesocket
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

// Global variables
client_info_t clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
telemetry_data_t vehicle_data;
pthread_mutex_t vehicle_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *log_file = NULL;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_running = 1;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <puerto> <archivo_logs>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    char *log_filename = argv[2];

#ifdef _WIN32
    // Inicializar Winsock en Windows
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        exit(EXIT_FAILURE);
    }
#endif

    // Abrir archivo de logs
    log_file = fopen(log_filename, "a");
    if (!log_file) {
        perror("Error abriendo archivo de logs");
        exit(EXIT_FAILURE);
    }

    // Configurar handler para señales
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Inicializar datos del vehículo
    init_vehicle_data();

    // Crear socket del servidor
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error creando socket");
        exit(EXIT_FAILURE);
    }

    // Configurar reutilización de dirección
#ifdef _WIN32
    char opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
#else
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
#endif
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección del servidor
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }

    log_message("INFO", "Servidor iniciado en puerto %d", port);
    printf("Servidor escuchando en puerto %d\n", port);

    // Crear hilo para envío de telemetría
    pthread_t telemetry_thread;
    if (pthread_create(&telemetry_thread, NULL, telemetry_sender, NULL) != 0) {
        perror("Error creando hilo de telemetría");
        exit(EXIT_FAILURE);
    }

    // Loop principal para aceptar conexiones
    while (server_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR && !server_running) {
                break;
            }
            perror("Error en accept");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (client_count >= MAX_CLIENTS) {
            pthread_mutex_unlock(&clients_mutex);
            log_message("WARNING", "Máximo número de clientes alcanzado");
            close(client_fd);
            continue;
        }

        // Configurar nueva conexión
        int client_index = client_count++;
        clients[client_index].socket_fd = client_fd;
        clients[client_index].address = client_addr;
        clients[client_index].active = 1;
        inet_ntop(AF_INET, &client_addr.sin_addr, clients[client_index].ip, INET_ADDRSTRLEN);
        clients[client_index].port = ntohs(client_addr.sin_port);

        pthread_mutex_unlock(&clients_mutex);

        // Crear hilo para manejar cliente
        if (pthread_create(&clients[client_index].thread_id, NULL, client_handler, &client_index) != 0) {
            perror("Error creando hilo para cliente");
            remove_client(client_index);
            continue;
        }

        log_message("INFO", "Nuevo cliente conectado desde %s:%d",
                   clients[client_index].ip, clients[client_index].port);
    }

    // Cleanup
    log_message("INFO", "Cerrando servidor...");
    close(server_fd);

    // Cerrar todas las conexiones de clientes
    for (int i = 0; i < client_count; i++) {
        if (clients[i].active) {
            close(clients[i].socket_fd);
            pthread_cancel(clients[i].thread_id);
        }
    }

    pthread_cancel(telemetry_thread);

    if (log_file) {
        fclose(log_file);
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}

void *client_handler(void *arg) {
    int client_index = *(int*)arg;
    char buffer[BUFFER_SIZE];

    while (server_running && clients[client_index].active) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(clients[client_index].socket_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0) {
            log_message("INFO", "Cliente %s:%d desconectado",
                       clients[client_index].ip, clients[client_index].port);
            break;
        }

        buffer[bytes_received] = '\0';

        // Eliminar salto de línea y carriage return
        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        char *carriage = strchr(buffer, '\r');
        if (carriage) *carriage = '\0';

        // Eliminar espacios al final
        int len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == ' ' || buffer[len-1] == '\t')) {
            buffer[--len] = '\0';
        }

        log_message("DEBUG", "Mensaje recibido de %s:%d: %s",
                   clients[client_index].ip, clients[client_index].port, buffer);

        // Procesar mensaje
        if (strncmp(buffer, "AUTH:", 5) == 0) {
            handle_auth(client_index, buffer + 5);
        } else if (strncmp(buffer, "CMD:", 4) == 0) {
            handle_command(client_index, buffer + 4);
        } else if (strncmp(buffer, "PING:", 5) == 0) {
            send_to_client(client_index, "PONG:");
        } else if (strncmp(buffer, "DISCONNECT:", 11) == 0) {
            log_message("INFO", "Cliente %s:%d se desconectó voluntariamente",
                       clients[client_index].ip, clients[client_index].port);
            break;
        } else {
            send_to_client(client_index, "ERROR:INVALID_FORMAT");
        }
    }

    remove_client(client_index);
    return NULL;
}

void *telemetry_sender(void *arg) {
    (void)arg; // Suprimir warning de parámetro no usado
    char telemetry_msg[BUFFER_SIZE];

    while (server_running) {
#ifdef _WIN32
        Sleep(TELEMETRY_INTERVAL * 1000); // Sleep en Windows usa milisegundos
#else
        sleep(TELEMETRY_INTERVAL);
#endif

        pthread_mutex_lock(&vehicle_mutex);

        // Consumo de batería basado en velocidad
        if (vehicle_data.speed > 0 && vehicle_data.battery > 0) {
            // Consumo: 0.5% base + 0.1% por cada 10 km/h de velocidad
            float consumption = 0.5 + (vehicle_data.speed / 10.0) * 0.1;
            vehicle_data.battery -= (int)consumption;

            if (vehicle_data.battery < 0) {
                vehicle_data.battery = 0;
                vehicle_data.speed = 0; // Detener vehículo si batería agotada
                log_message("WARNING", "Batería agotada - Vehículo detenido");
            } else if (vehicle_data.battery <= 10) {
                log_message("WARNING", "Batería crítica: %d%%", vehicle_data.battery);
            } else if (vehicle_data.battery <= 20) {
                log_message("WARNING", "Batería baja: %d%%", vehicle_data.battery);
            }
        }

        snprintf(telemetry_msg, BUFFER_SIZE, "TELEMETRY:%.1f,%d,%.1f,%s",
                vehicle_data.speed, vehicle_data.battery,
                vehicle_data.temperature, vehicle_data.direction);
        pthread_mutex_unlock(&vehicle_mutex);

        send_to_all_clients(telemetry_msg);
        log_message("DEBUG", "Telemetría enviada: %s", telemetry_msg);
    }

    return NULL;
}

void send_to_all_clients(const char *message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].active) {
            send_to_client(i, message);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_to_client(int client_index, const char *message) {
    char msg_with_newline[BUFFER_SIZE];
    snprintf(msg_with_newline, BUFFER_SIZE, "%s\n", message);

    if (send(clients[client_index].socket_fd, msg_with_newline, strlen(msg_with_newline), 0) < 0) {
        log_message("ERROR", "Error enviando mensaje a cliente %s:%d",
                   clients[client_index].ip, clients[client_index].port);
        clients[client_index].active = 0;
    }
}

void log_message(const char *level, const char *format, ...) {
    pthread_mutex_lock(&log_mutex);

    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    va_list args;
    va_start(args, format);

    // Log a consola
    printf("[%s] [%s] ", time_str, level);
    vprintf(format, args);
    printf("\n");

    // Log a archivo
    if (log_file) {
        fprintf(log_file, "[%s] [%s] ", time_str, level);
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }

    va_end(args);
    pthread_mutex_unlock(&log_mutex);
}

void handle_auth(int client_index, const char *auth_data) {
    // Crear copia local para usar con strtok
    char auth_copy[BUFFER_SIZE];
    strncpy(auth_copy, auth_data, BUFFER_SIZE - 1);
    auth_copy[BUFFER_SIZE - 1] = '\0';

    char *type = strtok(auth_copy, ":");
    char *password = strtok(NULL, ":");

    if (!type) {
        send_to_client(client_index, "AUTH:FAIL:invalid_format");
        return;
    }

    if (strcmp(type, "ADMIN") == 0) {
        log_message("DEBUG", "Intento de auth ADMIN desde %s:%d, password='%s'",
                   clients[client_index].ip, clients[client_index].port,
                   password ? password : "(null)");

        if (password && strcmp(password, ADMIN_PASSWORD) == 0) {
            clients[client_index].type = CLIENT_ADMIN;
            send_to_client(client_index, "AUTH:OK:ADMIN");
            log_message("INFO", "Cliente %s:%d autenticado como ADMIN",
                       clients[client_index].ip, clients[client_index].port);
        } else {
            send_to_client(client_index, "AUTH:FAIL:invalid_password");
            log_message("WARNING", "Intento de autenticación fallido desde %s:%d - Password esperado: '%s', recibido: '%s'",
                       clients[client_index].ip, clients[client_index].port,
                       ADMIN_PASSWORD, password ? password : "(null)");
        }
    } else if (strcmp(type, "OBSERVER") == 0) {
        clients[client_index].type = CLIENT_OBSERVER;
        send_to_client(client_index, "AUTH:OK:OBSERVER");
        log_message("INFO", "Cliente %s:%d autenticado como OBSERVER",
                   clients[client_index].ip, clients[client_index].port);
    } else {
        send_to_client(client_index, "AUTH:FAIL:invalid_type");
    }
}

void handle_command(int client_index, const char *command) {
    if (clients[client_index].type != CLIENT_ADMIN) {
        send_to_client(client_index, "ERROR:UNAUTHORIZED");
        return;
    }

    pthread_mutex_lock(&vehicle_mutex);

    if (strcmp(command, "SPEED_UP") == 0) {
        // Verificar batería antes de acelerar
        if (vehicle_data.battery <= 0) {
            send_to_client(client_index, "CMD:FAIL:battery_depleted");
            log_message("WARNING", "SPEED_UP rechazado - Batería agotada");
        } else if (vehicle_data.battery <= 15) {
            send_to_client(client_index, "CMD:FAIL:battery_too_low");
            log_message("WARNING", "SPEED_UP rechazado - Batería crítica (%d%%)", vehicle_data.battery);
        } else if (vehicle_data.speed < 120) {
            vehicle_data.speed += 5.0;
            send_to_client(client_index, "CMD:OK:speed_increased");
            log_message("INFO", "Velocidad aumentada a %.1f por admin %s:%d (Batería: %d%%)",
                       vehicle_data.speed, clients[client_index].ip, clients[client_index].port,
                       vehicle_data.battery);
        } else {
            send_to_client(client_index, "CMD:FAIL:max_speed_reached");
        }
    } else if (strcmp(command, "SLOW_DOWN") == 0) {
        if (vehicle_data.speed > 0) {
            vehicle_data.speed -= 5.0;
            if (vehicle_data.speed < 0) vehicle_data.speed = 0;
            send_to_client(client_index, "CMD:OK:speed_decreased");
            log_message("INFO", "Velocidad reducida a %.1f por admin %s:%d",
                       vehicle_data.speed, clients[client_index].ip, clients[client_index].port);
        } else {
            send_to_client(client_index, "CMD:FAIL:already_stopped");
        }
    } else if (strcmp(command, "TURN_LEFT") == 0) {
        strcpy(vehicle_data.direction, "LEFT");
        send_to_client(client_index, "CMD:OK:turned_left");
        log_message("INFO", "Dirección cambiada a LEFT por admin %s:%d",
                   clients[client_index].ip, clients[client_index].port);
    } else if (strcmp(command, "TURN_RIGHT") == 0) {
        strcpy(vehicle_data.direction, "RIGHT");
        send_to_client(client_index, "CMD:OK:turned_right");
        log_message("INFO", "Dirección cambiada a RIGHT por admin %s:%d",
                   clients[client_index].ip, clients[client_index].port);
    } else if (strcmp(command, "RECHARGE") == 0) {
        // Recargar batería (solo si vehículo está detenido)
        if (vehicle_data.speed == 0) {
            vehicle_data.battery = 100;
            send_to_client(client_index, "CMD:OK:battery_recharged");
            log_message("INFO", "Batería recargada al 100%% por admin %s:%d",
                       clients[client_index].ip, clients[client_index].port);
        } else {
            send_to_client(client_index, "CMD:FAIL:vehicle_must_be_stopped");
            log_message("WARNING", "RECHARGE rechazado - Vehículo en movimiento (%.1f km/h)",
                       vehicle_data.speed);
        }
    } else if (strcmp(command, "LIST_CLIENTS") == 0) {
        pthread_mutex_unlock(&vehicle_mutex);
        list_clients(client_index);
        return;
    } else {
        send_to_client(client_index, "ERROR:UNKNOWN_COMMAND");
    }

    pthread_mutex_unlock(&vehicle_mutex);
}

void list_clients(int admin_client_index) {
    char client_list[BUFFER_SIZE * 2];
    char temp[256];

    pthread_mutex_lock(&clients_mutex);

    int active_clients = 0;
    for (int i = 0; i < client_count; i++) {
        if (clients[i].active) active_clients++;
    }

    snprintf(client_list, sizeof(client_list), "CLIENTS:%d", active_clients);

    for (int i = 0; i < client_count; i++) {
        if (clients[i].active) {
            snprintf(temp, sizeof(temp), ",%s:%d:%s",
                    clients[i].ip, clients[i].port,
                    clients[i].type == CLIENT_ADMIN ? "ADMIN" : "OBSERVER");
            strcat(client_list, temp);
        }
    }

    pthread_mutex_unlock(&clients_mutex);

    send_to_client(admin_client_index, client_list);
}

void remove_client(int client_index) {
    pthread_mutex_lock(&clients_mutex);

    if (clients[client_index].active) {
        close(clients[client_index].socket_fd);
        clients[client_index].active = 0;
        log_message("INFO", "Cliente %s:%d removido",
                   clients[client_index].ip, clients[client_index].port);
    }

    pthread_mutex_unlock(&clients_mutex);
}

void init_vehicle_data() {
    pthread_mutex_lock(&vehicle_mutex);
    vehicle_data.speed = 0.0;
    vehicle_data.battery = 100;
    vehicle_data.temperature = 22.5;
    strcpy(vehicle_data.direction, "FORWARD");
    pthread_mutex_unlock(&vehicle_mutex);
}

void signal_handler(int sig) {
    log_message("INFO", "Señal %d recibida, cerrando servidor...", sig);
    server_running = 0;
}

int find_client_by_socket(int socket_fd) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket_fd == socket_fd && clients[i].active) {
            return i;
        }
    }
    return -1;
}