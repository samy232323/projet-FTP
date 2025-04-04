# Projet FTP - Client/Serveur en C

📂 Ce projet consiste à implémenter un serveur FTP avec :
- Un serveur maître qui fait de la répartition de charge (Round Robin)
- Plusieurs serveurs esclaves (concurrents avec pool de processus)
- Un client FTP capable de :
  - se connecter au maître
  - être redirigé vers un esclave
  - télécharger des fichiers (`GET`)
  - reprendre un téléchargement interrompu
  - gérer une commande `bye`

## 🎯 Fonctionnalités

✅ Reprise de téléchargement  
✅ Traitement de fichiers binaires  
✅ Transfert par blocs  
✅ Résistance aux crashs clients/serveurs  
✅ Redirection dynamique via maître  

## 🚀 Compilation

Dans le terminal, compile les exécutables avec :

```bash
gcc -o ftpserver ftpserver.c file_transfer.c csapp.c -lpthread
gcc -o ftpclient ftpclient.c csapp.c -lpthread
gcc -o ftp_master ftp_master.c csapp.c -lpthread

Ligne pour initialiser les servers :
./ftpserver 2525
./ftpserver 2526
./ftpserver 2527

Lancer le serveur maître avec cette ligne  :

./ftp_master

Lancer le cote client  avec cette ligne :

./ftpclient localhost
