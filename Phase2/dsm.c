#include "dsm.h"
#define _GNU_SOURCE

int DSM_NODE_NUM; /* nombre de processus dsm */
int DSM_NODE_ID;
int sock_ecoute ; /* rang (= numero) du processus */
int sock_initialisation;

void info_dsmwrap_init(infos_dsm_t *infos_init[], int DSM_NODE_NUM){
  for (int i = 0; i < DSM_NODE_NUM; i++)
  infos_init[i] = malloc(sizeof(infos_dsm_t));
}

int creer_socket_serv(int *serv_port,struct sockaddr_in *serv_addr)
{
  int fd;
  int port;

  /* fonction de creation et d'attachement d'une nouvelle socket */
  fd = do_socket();
  init_serv_addr(serv_addr, *serv_port); // init avec port choisi par la machine
  do_bind(fd, *serv_addr);

  // Récupération du n° de port de la socket
  socklen_t len = sizeof(struct sockaddr_in);
  getsockname(fd, (struct sockaddr *) serv_addr, &len);
  port = ntohs(serv_addr->sin_port);
  *serv_port = port;

  /* renvoie le numero de descripteur */
  /* et modifie le parametre serv_port */
  return fd;
}
void do_bind(int socket, struct sockaddr_in addr_in)
{
  int bind_result = bind(socket, (struct sockaddr *) &addr_in, sizeof(addr_in));
  if (-1 == bind_result)
  error("bind");
}

int do_socket(){
  int file_des;
  do {
    file_des = socket(AF_INET, SOCK_STREAM, 0);
  } while ((file_des == -1) && (errno == EAGAIN || errno == EINTR));

  if (file_des == -1)
  error("socket");

  return file_des;
}


void init_serv_addr(struct sockaddr_in *serv_addr, int port)
{
  memset(serv_addr, 0, sizeof(struct sockaddr_in)); // clean structure
  serv_addr->sin_family = AF_INET; // IP V4
  serv_addr->sin_port = port;
  serv_addr->sin_addr.s_addr = INADDR_ANY;
}

int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen)
{
  printf("[do_accept] début\n");
  int file_des_new = accept(socket, addr, addrlen);
  printf("[do_accept] %d\n",file_des_new );
  if(-1 == file_des_new)
  error("accept");
  return file_des_new;
}

void do_connect(int sock, struct sockaddr_in host_addr) {
  int connect_result;

  do {
    connect_result = connect(sock, (struct sockaddr *) &host_addr, sizeof(host_addr));
  } while ((connect_result == -1) && (errno == EAGAIN || errno == EINTR));

  if (connect_result == -1)
  error("connect");
}

void do_listen(int socket, int nb_max)
{
  int listen_result = listen(socket, nb_max);
  if (-1 == listen_result)
  error("listen");
}
void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port) {
  // clean structure
  memset(serv_addr, '\0', sizeof(*serv_addr));
  serv_addr->sin_family = AF_INET; // IP V4
  serv_addr->sin_port = htons(port); // specified port in args
  serv_addr->sin_addr.s_addr = inet_addr(ip); // specified server IP in args
}
/* indique l'adresse de debut de la page de numero numpage */
static char *num2address( int numpage )
{
   char *pointer = (char *)(BASE_ADDR+(numpage*(PAGE_SIZE)));

   if( pointer >= (char *)TOP_ADDR ){
      fprintf(stderr,"[%i] Invalid address !\n", DSM_NODE_ID);
      return NULL;
   }
   else return pointer;
}

int address2num(char * addr){
    return (((long int)(addr-BASE_ADDR))/(PAGE_SIZE));
}

/* fonctions pouvant etre utiles */
static void dsm_change_info( int numpage, dsm_page_state_t state, dsm_page_owner_t owner)
{
   if ((numpage >= 0) && (numpage < PAGE_NUMBER)) {
	if (state != NO_CHANGE )
	table_page[numpage].status = state;
      if (owner >= 0 )
	table_page[numpage].owner = owner;
      return;
   }
   else {
	fprintf(stderr,"[%i] Invalid page number !\n", DSM_NODE_ID);
      return;
   }
}

static dsm_page_owner_t get_owner( int numpage)
{
   return table_page[numpage].owner;
}

static dsm_page_state_t get_status( int numpage)
{
   return table_page[numpage].status;
}

/* Allocation d'une nouvelle page */
static void dsm_alloc_page( int numpage )
{
   char *page_addr = num2address( numpage );
   mmap(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   return ;
}

/* Changement de la protection d'une page */
static void dsm_protect_page( int numpage , int prot)
{
   char *page_addr = num2address( numpage );
   mprotect(page_addr, PAGE_SIZE, prot);
   return;
}

static void dsm_free_page( int numpage )
{
   char *page_addr = num2address( numpage );
   munmap(page_addr, PAGE_SIZE);
   return;
}

static void *dsm_comm_daemon( void *arg)
{
   while(1)
     {
	/* a modifier */
	printf("[%i] Waiting for incoming reqs \n", DSM_NODE_ID);
	sleep(2);
     }
   //return;
}
send_request(int owner,int numpage,int fd)
{
  char requete[100];
  memset(requete,'\0',46);
  sprintf(requete,"le processus %d demande d'acceder à la page : %d",DSM_NODE_ID,numpage);
  int wr=write(fd,requete,46);
  if (wr < 0) {
    error("error sending request\n");
  }
}

static int dsm_send(int dest,void *buf,size_t size)
{
   /* a completer */
}

static int dsm_recv(int from,void *buf,size_t size)
{
   /* a completer */
}

static void dsm_handler( void *page_addr,int sock_ecoute)
{
   /* A modifier */
   printf("[%i] FAULTY  ACCESS !!! \n",DSM_NODE_ID);
   int numpage = address2num((char *)(page_addr));
   int owner = get_owner(numpage);
   int fd= sock_ecoute;
   send_request(owner,numpage,fd);
   abort();
}

/* traitant de signal adequat */
static void segv_handler(int sig, siginfo_t *info, void *context)
{
   /* A completer*/
  if (info->si_code==SEGV_MAPERR){                                                                         //--> adresse sans objet
   /* adresse qui a provoque une erreur */
   void  *addr = info->si_addr;
  /* Si ceci ne fonctionne pas, utiliser a la place :*/
  /*
   #ifdef __x86_64__
   void *addr = (void *)(context->uc_mcontext.gregs[REG_CR2]);
   #elif __i386__
   void *addr = (void *)(context->uc_mcontext.cr2);
   #else
   void  addr = info->si_addr;
   #endif
   */
   /*
   pour plus tard (question ++):
   if (info->si_code==SEGV_ACCERR)--> permissions invalide
   dsm_access_t access  = (((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR] & 2) ? WRITE_ACCESS : READ_ACCESS;
  */
   /* adresse de la page dont fait partie l'adresse qui a provoque la faute */
   void  *page_addr  = (void *)(((unsigned long) addr) & ~(PAGE_SIZE-1));

   if ((addr >= (void *)BASE_ADDR) && (addr < (void *)TOP_ADDR))
     {
	dsm_handler(page_addr,sock_ecoute);
     }
   else
     {
	/* SIGSEGV normal : ne rien faire*/
     }
    }
}

/* Seules ces deux dernieres fonctions sont visibles et utilisables */
/* dans les programmes utilisateurs de la DSM                       */
char *dsm_init(int argc, char **argv)
{
   struct sigaction act;
   int index;

   int sock_initialisation=atoi(argv[1]);
   int sock_ecoute=atoi(argv[2]);
   /* Lecture des infos (port + IP) nécessaires aux connexions aux tres processus dsm */
   infos_dsm_t * infos_init[DSM_NODE_NUM];
   info_dsmwrap_init(infos_init, DSM_NODE_NUM);
   for (int i = 0; i < DSM_NODE_NUM; i++) {
     int test_info_init_dsmwrap = read(sock_initialisation, infos_init[i], sizeof(infos_dsm_t));
     if (test_info_init_dsmwrap < 0)
       error("read info_init_dsmwrap");
   }

   /* initialisation des connexions */
   /* avec les autres processus : connect/accept */
   /*definition des variables nécessaires au poll*/

   struct sockaddr_in *serv_addr_ecoute=malloc(sizeof(struct sockaddr_in));
   socklen_t addrlen = sizeof(struct sockaddr);
   int *serv_port = malloc(sizeof(int));

   sock_ecoute = creer_socket_serv(serv_port,serv_addr_ecoute);
   do_listen(sock_ecoute, NB_MAX_PROC);
   /* SOCKET de communication avec les autres processus DMS : SET-UP declarations */
   struct sockaddr_in serv_addr_connexion;
   int sock;
   for (int j = 0; j <DSM_NODE_NUM; j++) {
     printf("====>%d\n",infos_init[j]->rank);
     if (infos_init[j]->rank > DSM_NODE_ID) {
       sock = do_socket();
       init_client_addr(&serv_addr_connexion, infos_init[j]->IP, infos_init[j]->port);
       do_connect(sock, serv_addr_connexion);
       printf(">>>>>>>[dsm] connexion ok : %d\n", sock);
       fflush(stdout);
     }
     else if (infos_init[j]->rank != DSM_NODE_ID){
       printf(">>>>>>>[dsm%d] accept début %d\n",DSM_NODE_ID, infos_init[j]->rank );
       fflush(stdout);
       sock = do_accept(sock_ecoute, (struct sockaddr*)serv_addr_ecoute, &addrlen);
       printf(">>>>>>>[dsm] accept fin de  la fin : %d\n", sock);
       fflush(stdout);
     }
   }



   /* Allocation des pages en tourniquet */
   for(index = 0; index < PAGE_NUMBER; index ++){
     if ((index % DSM_NODE_NUM) == DSM_NODE_ID)
       dsm_alloc_page(index);
     dsm_change_info( index, WRITE, index % DSM_NODE_NUM);
   }

   /* mise en place du traitant de SIGSEGV */
   act.sa_flags = SA_SIGINFO; // traitant étendendu
   act.sa_sigaction = segv_handler;
   sigaction(SIGSEGV, &act, NULL);

   /* creation du thread de communication */
   /* ce thread va attendre et traiter les requetes */
   /* des autres processus */
   pthread_create(&comm_daemon, NULL, dsm_comm_daemon, NULL);

   /* Adresse de début de la zone de mémoire partagée */
   return ((char *)BASE_ADDR);
}

void dsm_finalize(void)
{
   /* fermer proprement les connexions avec les autres processus */
   /* Libération des ressources */



   /* terminer correctement le thread de communication */
   /* pour le moment, on peut faire : */
   pthread_cancel(comm_daemon);

  return;
}
