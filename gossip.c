/*
    Arquivo: main.c
    Autor: Daniel Garcia
    Matricula: 09/0018362

    Trabalho de implementacao
    Transmissão de dados - Turma A - 01/2015
    Dept. Ciencia da Computacao - UnB
*/

#include <stdio.h>      /* para printf() e fprintf() */
#include <string.h>     /* para memset() */
#include <stdlib.h>     /* para atoi() e exit() */
#include <unistd.h>     /* para close() */
#include <sys/socket.h> /* para socket(), bind(), e connect() */
#include <arpa/inet.h>  /* para sockaddr_in e inet_ntoa() */

void SairComErro(char *mensagem);  /* Sai do programa com erro */
void LidarComCliente(int socketCliente);   /* TCP client handling function */
void IniciarServidor();
void IniciarCliente();
void LidarComCliente(int socketServidor);
void LidarComServidor(int socketCliente);
char * LerEntrada(void);

#define TAMBUFFER 255 // Em bytes

int main(int argc, char *argv[])
{
    char * tipo;

    // Menu de opcoes

    while(1) {
        printf("Escolha uma opcao:\n 1. Servidor \n 2. Cliente \n 3. Sair\n\nEscolha: ");
        tipo = LerEntrada();

        if(atoi(tipo) == 1 || atoi(tipo) == 2) break;
        printf("Tipo incorreto!\n\n");
    }

    if(atoi(tipo) == 1) IniciarServidor();
    else if (atoi(tipo) == 2) IniciarCliente();
    else if (atoi(tipo) == 3) exit(1);

    return 1;
}

void IniciarServidor() {

    int servSock;                    /* ID do socket do Servidor */
    int clntSock;                    /* ID do socket do Cliente */
    struct sockaddr_in ServSocketStruct; /* Endereco local */
    struct sockaddr_in echoClntAddr; /* Endereco cliente */
    unsigned short portaServidor;    /* Porta do servidor */
    unsigned int clntLen;            /* Tamanho da estrutura de dados do cliente */
    char * porta;

    printf("Escolha uma porta para atribuir ao servidor [10101]: ");
    porta = LerEntrada();
    if(strlen(porta) == 1)
        strcpy (porta,"10101");

    portaServidor = atoi(porta); // Atribui a porta ao servidor

    printf("Porta escolhida %d\n", portaServidor);

    /* Cria um socket TCP para lidar com os clientes */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        SairComErro("socket() falhou");
      
    /* Constroi a estrutura TCP para o socket */
    memset(&ServSocketStruct, 0, sizeof(ServSocketStruct));   /* Zera a estrutura para limpar o lixo de memoria */
    ServSocketStruct.sin_family = AF_INET;                /* Familia endereco */
    ServSocketStruct.sin_addr.s_addr = htonl(INADDR_ANY); /* Vindo de qualquer interface */
    ServSocketStruct.sin_port = htons(portaServidor);      /* Porta do socket (Servidor) */

    /* Atribui a porta local */
    if (bind(servSock, (struct sockaddr *) &ServSocketStruct, sizeof(ServSocketStruct)) < 0)
        SairComErro("bind() falhou");

    /* Coloca o socket em estado de escuta para futuras conexoes do cliente */
    if (listen(servSock, 5) < 0)
        SairComErro("listen() falhou");

    while(1)
    {
        clntLen = sizeof(echoClntAddr);

        /* Aguarda o cliente se conectar */
        if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0) // TODO: thread
            SairComErro("accept() falhou");

        printf("Cliente conectado %s\n", inet_ntoa(echoClntAddr.sin_addr));

        LidarComCliente(clntSock);
    }
}

void LidarComCliente(int socketServidor)
{
    char bufferMensagem[TAMBUFFER];     /* Buffer da mensagem */
    int tamMensagem;                    /* Tamanho da mensagem recebida */

    /* Aguarda receber mensagem do cliente "HELLO SVR" */
    if ((tamMensagem = recv(socketServidor, bufferMensagem, TAMBUFFER, 0)) < 0)
        SairComErro("recv() falhou");

    /* Envia ao cliente "HELLO CLT" */

    const char * hello = "HELLO CLT";
    int tamanhoMsg = strlen(hello);

    if (send(socketServidor, hello, tamanhoMsg, 0) != tamanhoMsg)
            SairComErro("send() falhou");

    /* Funcao de chat enquanto nao receber "BYE SRV" */
    do {

        printf("Cliente [ %d ] > %s\n", socketServidor, bufferMensagem);
        memset(&bufferMensagem, 0, TAMBUFFER); // Limpa a memoria

        /* Recebe mais mensagens do cliente */
        if ((tamMensagem = recv(socketServidor, bufferMensagem, TAMBUFFER, 0)) < 0)
            SairComErro("recv() falhou");

    } while (strcmp("BYE SRV", bufferMensagem));

    printf("Cliente [ %d ] > %s\n", socketServidor, bufferMensagem);
    printf("Enviando \"BYE CLT\" e desconectando cliente [ %d ] ...\n", socketServidor);
    
    /* Envia ao cliente "BYE CLT" */
    const char * bye = "BYE CLT";
    tamanhoMsg = strlen(bye);

    if (send(socketServidor, bye, tamanhoMsg, 0) != tamanhoMsg)
            SairComErro("send() falhou");

    close(socketServidor);    /* Fecha a conexao com o cliente */

    printf("Cliente [ %d ] desconectado!\n\n", socketServidor);
}

void IniciarCliente() {

    int clienteSock;                     /* ID do socket */
    struct sockaddr_in ServSocketStruct; /* Endereco do servidor [host] */
    unsigned short portaServidor;        /* Porta do servidor */
    char * IPserv;                       /* IP do servidor */
    char * porta;                        /* Porta do servidor char */

    printf("Digite o endereco IP do servidor [127.0.0.1]: ");
    IPserv = LerEntrada();
    if(strlen(IPserv) == 1)
        strcpy (IPserv, "127.0.0.1");
    printf("IP escolhido: %s\n", IPserv);

    printf("Digite a porta do servidor [10101]: ");
    porta = LerEntrada();
    if(strlen(porta) == 1)
        strcpy (porta, "10101");
    portaServidor = atoi(porta);
    printf("Porta escolhida: %d\n", portaServidor);

    /* Cria um socket TCP para o cliente */
    if ((clienteSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        SairComErro("socket() falhou");

    /* Constroi a estrutura de dados do socket */
    memset(&ServSocketStruct, 0, sizeof(ServSocketStruct)); /* Zera a estrutura para limpar o lixo de memoria */
    ServSocketStruct.sin_family      = AF_INET;             /* Familia endereco */
    ServSocketStruct.sin_addr.s_addr = inet_addr(IPserv);   /* IP do servidor */
    ServSocketStruct.sin_port        = htons(portaServidor); /* Porta do servidor */

    /* Estabelece conexao com o servidor */
    if (connect(clienteSock, (struct sockaddr *) &ServSocketStruct, sizeof(ServSocketStruct)) < 0)
        SairComErro("connect() falhou");

    LidarComServidor(clienteSock);
}

void LidarComServidor(int socketCliente) {

    char bufferMensagem[TAMBUFFER];     /* Buffer da mensagem */
    int tamMensagem;                    /* Tamanho da mensagem recebida */
    char * enviar;

    /* Envia "HELLO SVR" no inicio */
    const char * hello = "HELLO SVR";
    int tamanhoMsg = strlen(hello);

    if(send(socketCliente, hello, tamanhoMsg, 0) != tamanhoMsg)
        SairComErro("send() falhou");

    memset(&bufferMensagem, 0, TAMBUFFER); // Limpa a memoria

    /* Aguarda mensagem de resposta do servidor "HELLO CLT" */
    if ((tamMensagem = recv(socketCliente, bufferMensagem, TAMBUFFER, 0)) < 0)
        SairComErro("recv() falhou");

    printf("Servidor [ %d ] > %s\n", socketCliente, bufferMensagem);

    /* Funcao de chat enquanto nao enviar "BYE SRV" */
    do {

        printf("Enviar ao servidor [ %d ] > ", socketCliente);
        enviar = LerEntrada();
        enviar[strlen(enviar) - 1] = '\0';

        /* Envia mais mensagens ao servidor */
        if(send(socketCliente, enviar, strlen(enviar), 0) != strlen(enviar))
            SairComErro("send() falhou");

    } while (strcmp("BYE SRV", enviar));

    memset(&bufferMensagem, 0, TAMBUFFER); // Limpa a memoria

    /* Aguarda mensagem de resposta do servidor "BYE CLT" */
    if (recv(socketCliente, bufferMensagem, TAMBUFFER, 0) < 0)
        SairComErro("recv() falhou");

    printf("Servidor [ %d ] > %s\n", socketCliente, bufferMensagem);

    close(socketCliente);

    printf("Servidor [ %d ] desconectado!\n", socketCliente);

}


void SairComErro(char *mensagem)
{
    perror(mensagem);
    exit(1);
}

char * LerEntrada(void) {
    char * line = malloc(TAMBUFFER), * linep = line;
    size_t lenmax = TAMBUFFER, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}