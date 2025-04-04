#include "csapp.h"
#include "ftp.h"

#define ftp_port 2323 // Port du serveur maître (load balancer)

int main(int argc, char **argv)
{
    int clientfd;
    char *host, buf[MAXLINE];
    rio_t rio;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }

    host = argv[1];
    int masterfd = Open_clientfd(host, ftp_port);
    if (masterfd < 0) {
        fprintf(stderr, "Erreur : impossible de se connecter au serveur maître\n");
        exit(1);
    }

    printf("Connecté au serveur maître sur %s:%d\n", host, ftp_port);
    Rio_readinitb(&rio, masterfd);
    request_t req;
    req.type = REQ_REDIR;
    Rio_writen(masterfd, &req, sizeof(req));

    slave_info_t chosen;
    if (Rio_readn(masterfd, &chosen, sizeof(chosen)) != sizeof(chosen)) {
        fprintf(stderr, "Erreur : réception des infos échouée\n");
        Close(masterfd);
        exit(1);
    }

    char slave_ip[INET_ADDRSTRLEN];
    strcpy(slave_ip, chosen.ip);
    int slave_port = chosen.port;
    Close(masterfd);
    printf("Redirigé vers le serveur esclave %s:%d\n", slave_ip, slave_port);

    clientfd = Open_clientfd(slave_ip, slave_port);
    if (clientfd < 0) {
        fprintf(stderr, " Échec de connexion à l'esclave %s:%d\n", slave_ip, slave_port);
        exit(1);
    }

    printf("Connexion réussie au serveur esclave.\n");
    Rio_readinitb(&rio, clientfd);

    while (1) {
        char commande[MAXLINE];
        printf("ftp> ");
        if (Fgets(commande, MAXLINE, stdin) == NULL)
            break;

        char cmd[16], filename[MAXLINE];
        if (sscanf(commande, "%s %s", cmd, filename) < 1) {
            fprintf(stderr, "Commande invalide.\n");
            continue;
        }

        if (strcmp(cmd, "bye") == 0) {
            request_t req;
            req.type = REQ_BYE;
            Rio_writen(clientfd, &req, sizeof(req));
            break;
        }

        if (strcmp(cmd, "get") == 0) {
            request_t req;
            req.type = REQ_GET;
            strncpy(req.filename, filename, MAX_file_name_len);

            char filepath[MAXLINE];
            snprintf(filepath, sizeof(filepath), "%s/%s", CLIENT_DIRECTORY, req.filename);

            req.offset = 0;
            struct stat st;
            if (stat(filepath, &st) == 0) {
                req.offset = st.st_size;
                printf("Reprise du téléchargement à partir de l'octet %d\n", req.offset);
            }

            Rio_writen(clientfd, &req, sizeof(req));

            response_t resp;
            if (Rio_readn(clientfd, &resp, sizeof(resp)) != sizeof(resp)) {
                fprintf(stderr, "Erreur réception réponse serveur\n");
                break;
            }

            if (resp.status != 0) {
                printf("Taille restante reçue du serveur : %d octets\n", resp.filesize);
                fprintf(stderr, "Erreur : fichier introuvable\n");
                continue;
            }

            if (resp.filesize == 0) {
                printf("Le fichier \"%s\" est déjà entièrement téléchargé. Rien à faire.\n", filename);
                continue;
            }

            int output_fd;
            if (req.offset > 0)
                output_fd = open(filepath, O_WRONLY | O_APPEND);
            else
                output_fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (output_fd < 0) {
                perror("open");
                fprintf(stderr, "Erreur création fichier %s\n", filepath);
                break;
            }

            struct timeval start, end;
            gettimeofday(&start, NULL);

            char buffer[BLOCK_SIZE];
            int bytes_remaining = resp.filesize;
            int total_received = 0;

            while (bytes_remaining > 0) {
                int n = Rio_readn(clientfd, buffer, (bytes_remaining < BLOCK_SIZE) ? bytes_remaining : BLOCK_SIZE);
                if (n <= 0) {
                    fprintf(stderr, "Erreur : réception interrompue après %d octets\n", total_received);
                    break;
                }
                Write(output_fd, buffer, n);
                bytes_remaining -= n;
                total_received += n;
            }

            gettimeofday(&end, NULL);
            Close(output_fd);

            printf(" Écriture de %d octets dans %s...\n", total_received, filepath);

            double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
            double kbytes = total_received / 1024.0;
            double speed = kbytes / elapsed;

            printf("Transfer successfully complete.\n");
            printf("%d bytes received in %.2f seconds (%.2f Kbytes/s)\n", total_received, elapsed, speed);
        } else {
            fprintf(stderr, "Commande non reconnue. Utilisez : get <fichier> ou bye\n");
        }
    }

    Close(clientfd);
    exit(0);
}
