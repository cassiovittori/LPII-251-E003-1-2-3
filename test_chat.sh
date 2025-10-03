#!/bin/bash

# Configurações
PORT=8888
HOST="127.0.0.1"
CLIENT_COUNT=3
LOG_FILE="chat_server.log"
SERVER_OUTPUT="server_output.txt"

# Função para limpar processos em background
cleanup() {
    echo "Encerrando o servidor e clientes..."
    # A '|| true' evita erros se o processo já não existir
    kill $SERVER_PID 2>/dev/null || true
    pkill -P $$ client 2>/dev/null || true # Mata todos os processos 'client' filhos deste script
    rm -f $SERVER_OUTPUT
    echo "Limpeza concluída."
}

# Registra a função de limpeza para ser executada na saída do script
trap cleanup EXIT

# Limpa o arquivo de log anterior
> "$LOG_FILE"

# Compila o projeto
echo "Compilando o projeto..."
make clean
make
if [ $? -ne 0 ]; then
    echo "Falha na compilação."
    exit 1
fi

# Inicia o servidor em background
echo "Iniciando o servidor na porta $PORT..."
./server $PORT > $SERVER_OUTPUT 2>&1 &
SERVER_PID=$!
sleep 1 # Dá um tempo para o servidor iniciar

# Verifica se o servidor iniciou corretamente
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "Falha ao iniciar o servidor. Verifique o arquivo $SERVER_OUTPUT para detalhes."
    cat $SERVER_OUTPUT
    exit 1
fi

echo "Servidor iniciado com PID $SERVER_PID."
echo "Iniciando $CLIENT_COUNT clientes para o teste..."

# Inicia clientes em background
for i in $(seq 1 $CLIENT_COUNT); do
    (
        echo "Olá, mundo, sou o cliente $i"
        sleep 1
        echo "Este é um teste automático."
        sleep 1
    ) | ./client $HOST $PORT &
done

# Aguarda um tempo para as mensagens serem trocadas
echo "Aguardando 5 segundos para a troca de mensagens..."
sleep 5

# A limpeza será chamada automaticamente na saída
echo "Teste concluído."
echo "Verifique o arquivo de log '$LOG_FILE' para ver a atividade."