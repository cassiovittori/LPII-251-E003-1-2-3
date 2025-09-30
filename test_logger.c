#include "tslog.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // Para sleep

// Estrutura para passar argumentos para as threads
typedef struct {
    int thread_id;
    int num_messages;
} thread_args_t;

// Função executada por cada thread
void* worker(void* args) {
    thread_args_t* t_args = (thread_args_t*)args;
    char message_buffer[100];

    for (int i = 0; i < t_args->num_messages; ++i) {
        // Cria uma mensagem única para cada log
        sprintf(message_buffer, "Mensagem %d da Thread %d", i + 1, t_args->thread_id);

        // Alterna o nível de log para teste
        if (i % 5 == 0) {
            log_message(ERROR, message_buffer);
        } else {
            log_message(INFO, message_buffer);
        }
        // Pequena pausa para simular trabalho e aumentar a chance de contenção
        usleep(1000); // 1ms
    }

    free(t_args); // Libera a memória alocada para os argumentos
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <numero_de_threads> <mensagens_por_thread>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    int num_messages = atoi(argv[2]);

    if (num_threads <= 0 || num_messages <= 0) {
        fprintf(stderr, "Os argumentos devem ser números positivos.\n");
        return 1;
    }

    // Inicializa o logger para salvar em "chat_server.log"
    log_init("chat_server.log");

    pthread_t threads[num_threads];

    printf("Iniciando teste com %d threads, cada uma enviando %d mensagens.\n", num_threads, num_messages);

    // Cria as threads
    for (int i = 0; i < num_threads; ++i) {
        thread_args_t* args = malloc(sizeof(thread_args_t));
        args->thread_id = i + 1;
        args->num_messages = num_messages;
        if (pthread_create(&threads[i], NULL, worker, args) != 0) {
            perror("Erro ao criar thread");
            return 1;
        }
    }

    // Espera todas as threads terminarem
    for (int i = 0; i < num_threads; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Erro ao aguardar thread");
            return 1;
        }
    }

    // Fecha o logger
    log_close();

    printf("Teste concluído. Verifique o arquivo 'chat_server.log'.\n");

    return 0;
}