#ifndef TSLOG_H
#define TSLOG_H

// Enum para os níveis de log
typedef enum {
    INFO,
    ERROR
} LogLevel;

/**
 * @brief Inicializa a biblioteca de logging.
 *
 * Abre o arquivo de log e inicializa o mutex para controle de concorrência.
 * Deve ser chamada uma única vez no início do programa.
 *
 * @param filename O nome do arquivo onde os logs serão salvos.
 */
void log_init(const char *filename);

/**
 * @brief Grava uma mensagem de log de forma thread-safe.
 *
 * Adiciona informações de timestamp, nível de log e ID da thread à mensagem
 * antes de escrevê-la no arquivo.
 *
 * @param level O nível da mensagem (INFO ou ERROR).
 * @param message A mensagem de log a ser gravada.
 */
void log_message(LogLevel level, const char *message);

/**
 * @brief Finaliza a biblioteca de logging.
 *
 * Fecha o arquivo de log e destrói o mutex.
 * Deve ser chamada no final do programa para liberar os recursos.
 */
void log_close(void);

#endif // TSLOG_H