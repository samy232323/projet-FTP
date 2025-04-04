#include "csapp.h"
#include "ftp.h"

#define NB_SLAVES 3
#define MASTER_PORT 2323  // Port d'écoute du serveur maître 

slave_info_t slaves[NB_SLAVES]; // Tableau des esclaves
int next_slave = 0;             // Index pour round-robin

int main() {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    int slave_ports[NB_SLAVES] = {2525, 2526, 2527};

    // Initialisation des esclaves
    int esclaves_ok = 0;

    for (int i = 0; i < NB_SLAVES; i++) {
        slaves[i].port = slave_ports[i];
        strcpy(slaves[i].ip, "127.0.0.1");

        int testfd = Open_clientfd(slaves[i].ip, slaves[i].port);
        if (testfd < 0) {
            fprintf(stderr, "Esclave %d (%d) injoignable, il sera ignoré\n", i, slaves[i].port);
            slaves[i].port = -1; // Marque l'esclave comme indisponible
            continue;
        }
        esclaves_ok++;
        Close(testfd);
        printf("Esclave %d prêt sur %s:%d\n", i, slaves[i].ip, slaves[i].port);
    }
    
    if (esclaves_ok == 0) {
        fprintf(stderr, " Aucun esclave n'est disponible, arrêt du maître.\n");
        exit(1);
    }
    printf("Tous les esclaves sont connectés et prêts.\n");

    listenfd = Open_listenfd(MASTER_PORT);
    printf("Serveur maître à l'écoute sur le port %d\n", MASTER_PORT);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        rio_t rio; 
        Rio_readinitb(&rio, connfd);

        request_t req;
        ssize_t n = Rio_readn(connfd, &req, sizeof(req));

        if (n != sizeof(req)) {
            // Ignorer les connexions incomplètes ou fermées silencieusement
            Close(connfd);
            continue;
        }

        // Sélection d’un esclave disponible (round robin)
        int tried = 0;
        slave_info_t chosen;
        do {
            chosen = slaves[next_slave];
            next_slave = (next_slave + 1) % NB_SLAVES;
            tried++;
        } while (chosen.port == -1 && tried < NB_SLAVES);

        if (chosen.port == -1) {
            fprintf(stderr, "Aucun esclave disponible actuellement\n");
            Close(connfd);
            continue;
        }

        // Envoi de la redirection au client
        Rio_writen(connfd, &chosen, sizeof(chosen));

        if (req.type == REQ_REDIR)
            printf("Redirection demandée : vers %s:%d\n", chosen.ip, chosen.port);
        else
            printf("Client redirigé vers l'esclave %s:%d\n", chosen.ip, chosen.port);

        Close(connfd);
    }

    return 0;
}
