#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define close closesocket
    typedef int socklen_t;
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024
#define ADMIN_PASSWORD "admin123"
#define TELEMETRY_INTERVAL 10

typedef enum {
    CLIENT_OBSERVER,
    CLIENT_ADMIN
} client_type_t;

typedef struct {
    int socket_fd;
    struct sockaddr_in address;
    client_type_t type;
    char ip[INET_ADDRSTRLEN];
    int port;
    int active;
    pthread_t thread_id;
} client_info_t;

typedef struct {
    float speed;
    int battery;
    float temperature;
    char direction[10];
} telemetry_data_t;

// Global variables
extern client_info_t clients[MAX_CLIENTS];
extern int client_count;
extern pthread_mutex_t clients_mutex;
extern telemetry_data_t vehicle_data;
extern pthread_mutex_t vehicle_mutex;
extern FILE *log_file;
extern pthread_mutex_t log_mutex;
extern int server_running;

// Function prototypes
void *client_handler(void *arg);
void *telemetry_sender(void *arg);
void send_to_all_clients(const char *message);
void send_to_client(int client_index, const char *message);
void log_message(const char *level, const char *format, ...);
void handle_auth(int client_index, const char *message);
void handle_command(int client_index, const char *message);
void list_clients(int admin_client_index);
void remove_client(int client_index);
void init_vehicle_data();
void signal_handler(int sig);
int find_client_by_socket(int socket_fd);

#endif