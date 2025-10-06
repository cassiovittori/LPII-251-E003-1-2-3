#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "tslog.h"

#define INITIAL_CAPACITY 10
#define BUFFER_SIZE 2048
#define HISTORY_SIZE 10 

// Estrutura para representar um cliente conectado
typedef struct {
    int socket;
    int id;
    struct sockaddr_in address;
} client_t;

// Globais para a lista de clientes (recurso compartilhado)
static client_t *clients;
static int client_count = 0;
static int client_capacity = INITIAL_CAPACITY;
static int next_client_id = 1;
static pthread_mutex_t clients_mutex;

// GLOBAIS PARA O HISTÓRICO DE MENSAGENS
static char *message_history[HISTORY_SIZE];
static int history_count = 0;
static int history_head = 0;
static pthread_mutex_t history_mutex;

// Adiciona um cliente à lista. A lista já deve estar travada com o mutex.
void add_client(client_t client) {
    if (client_count == client_capacity) {
        client_capacity *= 2;
        clients = realloc(clients, client_capacity * sizeof(client_t));
        if (!clients) {
            log_message(ERROR, "Falha ao realocar memória para clientes.");
            exit(EXIT_FAILURE);
        }
    }
    clients[client_count++] = client;
}

// Remove um cliente da lista. A lista já deve estar travada com o mutex.
void remove_client(int id) {
    int i;
    for (i = 0; i < client_count; i++) {
        if (clients[i].id == id) {
            memmove(&clients[i], &clients[i + 1], (client_count - i - 1) * sizeof(client_t));
            client_count--;
            break;
        }
    }
}

void init_history() {
    pthread_mutex_init(&history_mutex, NULL);
    for (int i = 0; i < HISTORY_SIZE; i++) {
        message_history[i] = NULL;
    }
}

void add_to_history(const char *message) {
    pthread_mutex_lock(&history_mutex);
    
    // Libera a memória da mensagem mais antiga se o buffer estiver cheio
    if (message_history[history_head] != NULL) {
        free(message_history[history_head]);
    }

    // strdup aloca memória e copia a string
    message_history[history_head] = strdup(message);

    history_head = (history_head + 1) % HISTORY_SIZE;
    if (history_count < HISTORY_SIZE) {
        history_count++;
    }

    pthread_mutex_unlock(&history_mutex);
}

void send_history_to_client(int client_socket) {
    char welcome_msg[] = "--- Bem-vindo ao chat! Vendo histórico de mensagens... ---\n";
    write(client_socket, welcome_msg, strlen(welcome_msg));

    pthread_mutex_lock(&history_mutex);
    
    // Calcula o índice da mensagem mais antiga para começar a ler
    int start_index = (history_head - history_count + HISTORY_SIZE) % HISTORY_SIZE;

    for (int i = 0; i < history_count; i++) {
        int current_index = (start_index + i) % HISTORY_SIZE;
        if (message_history[current_index] != NULL) {
            write(client_socket, message_history[current_index], strlen(message_history[current_index]));
        }
    }
    pthread_mutex_unlock(&history_mutex);

    char end_msg[] = "--- Fim do histórico ---\n";
    write(client_socket, end_msg, strlen(end_msg));
}

// Retransmite uma mensagem para todos os clientes, exceto o remetente.
void broadcast_message(const char *message, int sender_id) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].id != sender_id) {
            if (write(clients[i].socket, message, strlen(message)) < 0) {
                log_message(ERROR, "Falha ao enviar mensagem broadcast.");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Função executada por cada thread de cliente
void *handle_client(void *arg) {
    client_t client = *(client_t *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    char message_with_id[BUFFER_SIZE + 50];
    char log_buffer[BUFFER_SIZE + 100];

    // Envia o histórico ao novo cliente 
    send_history_to_client(client.socket);

    int read_size;
    while ((read_size = read(client.socket, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[read_size] = '\0';
        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';

        if (strlen(buffer) == 0) continue; // Ignora mensagens vazias

        snprintf(message_with_id, sizeof(message_with_id), "[Cliente %d]: %s\n", client.id, buffer);
        snprintf(log_buffer, sizeof(log_buffer), "Mensagem recebida do Cliente %d: '%s'", client.id, buffer);
        log_message(INFO, log_buffer);

        // Adiciona ao histórico antes de retransmitir
        add_to_history(message_with_id);

        broadcast_message(message_with_id, client.id);
        memset(buffer, 0, BUFFER_SIZE);
    }

    close(client.socket);
    pthread_mutex_lock(&clients_mutex);
    remove_client(client.id);
    pthread_mutex_unlock(&clients_mutex);

    snprintf(log_buffer, sizeof(log_buffer), "Cliente %d desconectado.", client.id);
    log_message(INFO, log_buffer);

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    log_init("chat_server.log");
    log_message(INFO, "Iniciando servidor de chat...");

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Inicializa o histórico
    init_history();

    clients = malloc(INITIAL_CAPACITY * sizeof(client_t));
    if (!clients) {
        log_message(ERROR, "Falha ao alocar memória inicial para clientes.");
        return 1;
    }
    pthread_mutex_init(&clients_mutex, NULL);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        log_message(ERROR, "Falha no bind do socket.");
        return 1;
    }

    listen(server_socket, 5);
    char log_msg[100];
    snprintf(log_msg, sizeof(log_msg), "Servidor escutando na porta %s", argv[1]);
    log_message(INFO, log_msg);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            log_message(ERROR, "Falha ao aceitar conexão.");
            continue;
        }

        client_t *new_client = malloc(sizeof(client_t));
        new_client->socket = client_socket;
        new_client->address = client_addr;

        pthread_mutex_lock(&clients_mutex);
        new_client->id = next_client_id++;
        add_client(*new_client);
        pthread_mutex_unlock(&clients_mutex);

        snprintf(log_msg, sizeof(log_msg), "Cliente %d conectado de %s:%d", new_client->id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        log_message(INFO, log_msg);
        
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void *)new_client) != 0) {
            log_message(ERROR, "Falha ao criar thread para cliente.");
        }
        pthread_detach(tid);
    }

    // Limpeza (código inalcançável)
    free(clients);
    close(server_socket);
    pthread_mutex_destroy(&clients_mutex);
    pthread_mutex_destroy(&history_mutex);
    for(int i = 0; i < HISTORY_SIZE; i++) if(message_history[i]) free(message_history[i]);
    log_close();

    return 0;
}