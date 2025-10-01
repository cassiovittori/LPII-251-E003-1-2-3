#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 2048

void *receive_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char server_reply[BUFFER_SIZE];
    int read_size;

    while ((read_size = recv(sock, server_reply, BUFFER_SIZE - 1, 0)) > 0) {
        server_reply[read_size] = '\0';
        printf("%s", server_reply);
        memset(server_reply, 0, BUFFER_SIZE);
    }

    if (read_size == 0) {
        puts("Servidor desconectado.");
    } else if (read_size == -1) {
        perror("recv falhou");
    }
    
    // Encerra o programa se o servidor cair
    exit(0);
    return NULL;
}

void *send_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char message[BUFFER_SIZE];

    while (1) {
        if (fgets(message, BUFFER_SIZE, stdin) != NULL) {
            if (write(sock, message, strlen(message)) < 0) {
                perror("Falha ao enviar");
                break;
            }
        } else {
            break; // EOF
        }
    }
    
    // Encerra o programa se a entrada padrão fechar
    exit(0);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <ip_servidor> <porta>\n", argv[0]);
        return 1;
    }

    int sock;
    struct sockaddr_in server;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Não foi possível criar o socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Falha na conexão");
        return 1;
    }

    puts("Conectado ao servidor. Você pode começar a digitar.");

    pthread_t recv_thread, send_thread;

    if (pthread_create(&recv_thread, NULL, receive_handler, (void *)&sock) < 0) {
        perror("Não foi possível criar a thread de recebimento");
        return 1;
    }

    if (pthread_create(&send_thread, NULL, send_handler, (void *)&sock) < 0) {
        perror("Não foi possível criar a thread de envio");
        return 1;
    }

    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);

    close(sock);
    return 0;
}