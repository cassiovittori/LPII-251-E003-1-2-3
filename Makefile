# Nome do compilador
CC = gcc
# Flags de compilação
CFLAGS = -Wall -Werror -pthread -std=c99 -D_DEFAULT_SOURCE

# Alvos
all: server client

# Regra para compilar o servidor
server: server.c tslog.c tslog.h
	$(CC) $(CFLAGS) -o server server.c tslog.c

# Regra para compilar o cliente
client: client.c
	$(CC) $(CFLAGS) -o client client.c

# Regra para limpar os arquivos gerados
clean:
	rm -f server client *.o chat_server.log server_output.txt

# Phony target para evitar conflitos com nomes de arquivos
.PHONY: all clean