#include "csapp.h"
#include "ftp.h"
#include <unistd.h> // pour usleep

void echo(int connfd) {
    request_t req;            // Structure pour stocker la requête du client
    response_t resp;          // Structure pour envoyer une réponse au client
    char filepath[MAXLINE];   // Contiendra le chemin complet du fichier demandé
    int fd;                   // Descripteur du fichier à envoyer
    struct stat st;           // Structure pour stocker les infos sur le fichier
    ssize_t n;                // Nombre d'octets lus/écrits

    while (1) {
        // Lire la requête du client (de type request_t)
        n = Rio_readn(connfd, &req, sizeof(req));
        if (n == 0) {
            // Connexion fermée proprement
            Close(connfd);
            return;
        }
        if (n != sizeof(req)) {
            // Requête incomplète ou corrompue
            Close(connfd);
            return;
        }

        

        if (req.type == REQ_BYE) {
            printf("Client a demandé la déconnexion.\n");
            break;
        }

        if (req.type != REQ_GET) {
            resp.status = 1;
            resp.filesize = 0;
            Rio_writen(connfd, &resp, sizeof(resp));
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", SERVER_DIRECTORY, req.filename);

        if ((fd = open(filepath, O_RDONLY)) < 0) {
            resp.status = 1;
            resp.filesize = 0;
            Rio_writen(connfd, &resp, sizeof(resp));
            continue;
        }

        if (fstat(fd, &st) < 0) {
            Close(fd);
            resp.status = 1;
            resp.filesize = 0;
            Rio_writen(connfd, &resp, sizeof(resp));
            continue;
        }

        if (req.offset > 0) {
            if (lseek(fd, req.offset, SEEK_SET) < 0) {
                fprintf(stderr, "Erreur lseek\n");
                Close(fd);
                resp.status = 1;
                resp.filesize = 0;
                Rio_writen(connfd, &resp, sizeof(resp));
                continue;
            }
        }

        resp.status = 0;
        resp.filesize = st.st_size - req.offset;
        Rio_writen(connfd, &resp, sizeof(resp));

        char buffer[BLOCK_SIZE];
        while ((n = Read(fd, buffer, BLOCK_SIZE)) > 0) {
            printf("Envoi d un bloc de %ld octets...\n", n);
            Rio_writen(connfd, buffer, n);
            //usleep(1000); // Pour simuler un délai de transmission
        }

        Close(fd);
        printf("Fichier %s envoyé avec succès (reste : %d octets)\n", req.filename, resp.filesize);
    }

    Close(connfd);
}
