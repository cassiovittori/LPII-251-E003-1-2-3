# LPII-251-E003-1-2-3

# Projeto Final - Servidor de Chat Multiusuário (TCP)

## Descrição

Este projeto é a implementação de um servidor de chat TCP concorrente em C. O sistema permite que múltiplos clientes se conectem simultaneamente, troquem mensagens em tempo real e visualizem um histórico recente da conversa.

O servidor utiliza um modelo de uma thread por cliente para lidar com as conexões de forma concorrente, garantindo que a comunicação seja fluida e escalável. As estruturas de dados compartilhadas, como a lista de clientes e o histórico de mensagens, são protegidas com mecanismos de exclusão mútua para garantir a integridade em um ambiente multi-thread.

O projeto também inclui uma biblioteca de logging thread-safe (`libtslog`) para registrar eventos importantes do servidor, como conexões, desconexões e mensagens recebidas.

## Funcionalidades Implementadas

-   **Servidor Concorrente**: Aceita e gerencia múltiplos clientes simultaneamente usando `pthreads`.
-   **Broadcast de Mensagens**: Mensagens enviadas por um cliente são retransmitidas em tempo real para todos os outros clientes conectados.
-   **Identificação de Clientes**: O servidor atribui um ID numérico a cada cliente para identificação nas mensagens.
-   **Histórico de Mensagens**: Novos clientes recebem as últimas 10 mensagens do chat ao se conectarem, permitindo que se contextualizem na conversa.
-   **Sincronização Segura**: O acesso à lista de clientes e ao histórico é protegido por mutexes para evitar condições de corrida.
-   **Logging Thread-Safe**: Todos os eventos importantes do servidor são registrados em um arquivo `chat_server.log` de forma segura por múltiplas threads.
-   **Cliente Interativo**: Um cliente de linha de comando (CLI) que permite enviar e receber mensagens de forma não-bloqueante (usando duas threads).

## Pré-requisitos

Para compilar e executar o projeto, você precisará de um ambiente Linux ou compatível com as seguintes ferramentas:
-   `gcc` (compilador C)
-   `make` (ferramenta de build)
-   Biblioteca `pthreads`

## Como Fazer o Build do Projeto

O projeto utiliza um `Makefile` para automatizar o processo de compilação.

1.  **Clone o repositório:**
    ```sh
    git clone <url-do-repositorio>
    cd <caminho-do-repositorio>
    ```

2.  **Execute o comando `make`:**
    Este comando irá compilar todos os arquivos-fonte e gerar dois executáveis: `server` e `client`.
    ```sh
    make
    ```

3.  **Para limpar os arquivos gerados** (executáveis, arquivos objeto e logs), use o comando:
    ```sh
    make clean
    ```

## Como Executar

### 1. Iniciar o Servidor

Execute o `server` em um terminal, especificando a porta em que ele deve escutar por conexões.

```sh
./server <porta>
# Exemplo:
./server 8888
```
### 2. Iniciar Clientes 
Para cada cliente, abra um novo terminal e execute o client, especificando o endereço IP e a porta do servidor.

```sh
./client <ip_do_servidor> <porta>
# Exemplo para conexão local:
./client 127.0.0.1 8888
```
Repita o passo acima em diferentes terminais para simular múltiplos usuários.

### 3. Teste Automatizado
O projeto inclui um script de teste para simular rapidamente a conexão e o envio de mensagens de 3 clientes.

  Dê permissão de execução ao script:
  ```sh
  chmod +x test_chat.sh
  ```

  Execute o script:

  ```sh
  ./test_chat.sh
  ```

O script iniciará o servidor, os clientes, aguardará a troca de mensagens e encerrará tudo automaticamente.
