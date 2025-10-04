#include <stdio.h>
#include <string.h>

#define ADMIN_PASSWORD "admin123"

void test_auth(const char *auth_data) {
    printf("=== PRUEBA DE AUTENTICACIÓN ===\n");
    printf("Datos recibidos: '%s'\n", auth_data);

    char auth_copy[1024];
    strncpy(auth_copy, auth_data, 1023);
    auth_copy[1023] = '\0';

    char *type = strtok(auth_copy, ":");
    char *password = strtok(NULL, ":");

    printf("Tipo parseado: '%s'\n", type ? type : "(null)");
    printf("Password parseado: '%s'\n", password ? password : "(null)");
    printf("Password esperado: '%s'\n", ADMIN_PASSWORD);

    if (type && strcmp(type, "ADMIN") == 0) {
        printf("Tipo es ADMIN - OK\n");
        if (password) {
            printf("Comparando: '%s' vs '%s'\n", password, ADMIN_PASSWORD);
            int result = strcmp(password, ADMIN_PASSWORD);
            printf("strcmp result: %d\n", result);
            if (result == 0) {
                printf("✅ AUTENTICACIÓN EXITOSA\n");
            } else {
                printf("❌ PASSWORD NO COINCIDE\n");
            }
        } else {
            printf("❌ PASSWORD ES NULL\n");
        }
    } else {
        printf("❌ TIPO NO ES ADMIN\n");
    }
    printf("===============================\n\n");
}

int main() {
    // Probar exactamente lo que Java está enviando
    test_auth("ADMIN:admin123");

    // Probar con espacios
    test_auth("ADMIN: admin123");
    test_auth("ADMIN:admin123 ");

    return 0;
}