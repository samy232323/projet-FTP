/*
* echoserveri.c - A concurrent echo server using process pool
*/

#include "csapp.h"
#include "ftp.h"

#define MAX_NAME_LEN 256
#define NPROC 4  // Nombre fixe de processus exécutants
#define ftp_port 2323 // Port FTP 

// Gestionnaire signal SIGINT pour arrêter proprement les fils
void sigint_handler(int sig) {
    printf("Arrêt du serveur : envoi du signal à tous les processus du groupe...\n");
    Kill(0, SIGINT); // Envoie SIGINT à tous les processus du groupe (père + fils)
    exit(0);
}


void echo(int connfd);


// Fonction exécutée par chaque processus exécutant
void traite_client(int listenfd) {
    int connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    clientlen = sizeof(clientaddr);

    while (1) {
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // Obtenir infos du client connecté
        Getnameinfo((SA *)&clientaddr, clientlen,
                    client_hostname, MAX_NAME_LEN, 0, 0, 0);
        Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);
        printf("server connected to %s (%s)\n", client_hostname, client_ip_string);

        echo(connfd);  // Traitement de la connexion
    }
}

int main(int argc, char **argv) {
    int listenfd, i;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);  //  Lis le port passé dans la commande
        listenfd = Open_listenfd(port);

    // Installer le gestionnaire de signal pour SIGINT
    Signal(SIGINT, sigint_handler);
    Signal(SIGPIPE, SIG_IGN); // Ignorer le signal SIGPIPE

    // Créer à l'avance NPROC processus exécutants
    for (i = 0; i < NPROC; i++) {
        if (Fork() == 0) { // processus exécutant (fils)
            traite_client(listenfd);
            exit(0);
        }
    }

    // Serveur veilleur reste actif sans rien faire
    while (1)
        pause();

    exit(0);
}
