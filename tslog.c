#include "tslog.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

// Variáveis globais estáticas para o arquivo de log e o mutex
static FILE *log_file = NULL;
static pthread_mutex_t log_mutex;

// Converte o enum LogLevel para string
static const char* level_to_string(LogLevel level) {
    switch (level) {
        case INFO:  return "INFO";
        case ERROR: return "ERROR";
        default:    return "UNKNOWN";
    }
}

void log_init(const char *filename) {
    // Abre o arquivo no modo "append" (adicionar ao final)
    log_file = fopen(filename, "a");
    if (log_file == NULL) {
        perror("Erro ao abrir o arquivo de log");
        exit(EXIT_FAILURE);
    }

    // Inicializa o mutex
    if (pthread_mutex_init(&log_mutex, NULL) != 0) {
        perror("Erro ao inicializar o mutex");
        exit(EXIT_FAILURE);
    }
}

void log_message(LogLevel level, const char *message) {
    if (log_file == NULL) {
        fprintf(stderr, "Logger não inicializado.\n");
        return;
    }

    // Adquire o lock para garantir exclusão mútua
    pthread_mutex_lock(&log_mutex);

    // Obtém o tempo atual
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_buf[80];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", t);

    // Obtém o ID da thread
    pthread_t tid = pthread_self();

    // Escreve a mensagem formatada no arquivo
    fprintf(log_file, "[%s] [%lu] [%s] %s\n",
            time_buf,
            (unsigned long)tid,
            level_to_string(level),
            message);

    // Garante que a escrita seja feita imediatamente no disco
    fflush(log_file);

    // Libera o lock
    pthread_mutex_unlock(&log_mutex);
}

void log_close(void) {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
    // Destrói o mutex para liberar recursos
    pthread_mutex_destroy(&log_mutex);
}