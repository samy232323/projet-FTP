#ifndef FTP_H
#define FTP_H
#define MAX_file_name_len 256 // Taille maximale du nom de fichier
#define BLOCK_SIZE 4096 // Taille du bloc de transfert



#define SERVER_DIRECTORY "serveur_dir" // Répertoire du serveur
#define CLIENT_DIRECTORY "client_dir"  // Répertoire du client





//Définition du type enum pour les requêtes
typedef enum {
    REQ_GET,  // demande de telech de fichier
    REQ_PUT,  // demande d'envoi d'un fichier
    REQ_LS, // demande de liste de fichiers
    REQ_REDIR,   // demande de redirection 
    REQ_BYE // demande de terminaison de connexion
} typereq_t;



typedef struct {
    char filename [MAX_file_name_len];  // nom  du fichier
    typereq_t type;  
    int offset; //    offset du fichier
} request_t;

typedef struct {
    int status;     // 0 = succès, 1 = erreur
    int filesize;   // taille du fichier (en octets, si succès)
} response_t;

// Structure pour stocker les infos des esclaves
typedef struct {
    int port;                    // Port du serveur esclave
    char ip[INET_ADDRSTRLEN];   // Adresse IP du serveur esclave
} slave_info_t;

#endif