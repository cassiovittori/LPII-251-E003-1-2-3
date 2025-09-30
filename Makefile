# Nome do compilador
CC = gcc
# Flags de compilação
CFLAGS = -Wall -Werror -pthread -std=c99 -D_DEFAULT_SOURCE

# Nome do executável de teste
TARGET = test_logger
# Arquivos de código fonte da biblioteca
SOURCES_LIB = tslog.c
# Código fonte do programa de teste
SOURCE_TEST = test_logger.c

# Regra padrão: compila tudo
all: $(TARGET)

# Regra para criar o executável de teste
$(TARGET): $(SOURCE_TEST) $(SOURCES_LIB)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE_TEST) $(SOURCES_LIB)

# Regra para limpar os arquivos gerados
clean:
	rm -f $(TARGET) *.o chat_server.log

# Phony target para evitar conflitos com nomes de arquivos
.PHONY: all clean