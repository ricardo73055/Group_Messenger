#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <pthread.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

char echo=0;
char connections=0;
int list_sd[10];
void panic(char *msg);

#define panic(m)	{perror(m); abort();}
#define MAX_CLIENT_NUM			10
#define MAX_MSG_LEN     100000
#define AFK 2
#define ONLINE 1
#define OFFLINE 0
typedef struct client_socket_into client_socket_info_t;
struct client_socket_into
{
	int socket;
	int state;
	int index;
	clock_t end_time;
}; 

client_socket_info_t socket_table[MAX_CLIENT_NUM];


/*Function to receive messages from each client and send to others (Broadcast)*/
void *threadfuntion(void *arg)                    
{
	char buffer[MAX_MSG_LEN ];
	char senda[MAX_MSG_LEN ];
	client_socket_info_t *info = (client_socket_info_t *)arg;   /* get & convert the socket */	
	while(recv(info->socket,buffer,sizeof(buffer),0)>0){  /* Verify if still connected and receive*/
		  snprintf(senda, MAX_MSG_LEN, "S%d%s",info->index,buffer); /* Add SX to say the sender ex: S1 (Client 1)*/
		printf("Received from Client %d: %s\n",info->index,buffer);
		socket_table[info->index].end_time=clock(); /*End time 1 min*/
		
		for(int i=0;i<MAX_CLIENT_NUM;i++){                     /* Send to all clients*/
			if(socket_table[i].state && info->index!=i){    /* Verify IF ative AND not send to the same customer  */
				printf("Resend to: %d\n",socket_table[i].index);
				send(socket_table[i].socket,senda,sizeof(senda),0);  /* Send message*/
    			}
	    	}
    	
	}
	    
	
	printf("Close connection: Client %d\n", info->index);
	shutdown(info->socket,SHUT_RD);
	shutdown(info->socket,SHUT_WR);
	shutdown(info->socket,SHUT_RDWR);
	socket_table[info->index].state = OFFLINE;
	/* terminate the thread */
}
	                          

/*Function to scan terminal and send the message to all clients  */
void *threadfuntion_scan(void *arg){	
	while(1){
		int sd = *(int*)arg;               /* get & convert the socket */
		char buffer[MAX_MSG_LEN ];
		char senda[MAX_MSG_LEN ];
		scanf("%s",buffer);
		if(buffer[0]=='S' &&buffer[1]=='T'){
			printf("Status: 0-offline || 1-online || 2-AFK \n");
			printf("Client %c has the status: %d\n",buffer[2],socket_table[buffer[2]-48].state);
		}else{
		snprintf(senda, MAX_MSG_LEN, "SS%s",buffer); /* Add SX to say the sender ex: S1 (Client 1)*/	
		for(int i=0;i<connections;i++){    /* Send to all clients*/
			if(socket_table[i].state){ /* Verify IF ative*/
				send(socket_table[i].socket,senda,sizeof(senda),0); /* Send message*/
			}
	    	}
	    	}
	}		                                
}

/*Function to send a predefined message (used to send this message every 1 second)  */
static void sendPeriodicUpdate(int signo){		
	if(signo==SIGALRM){	/* handle of SIGNAL Alarm*/
		char buffer[MAX_MSG_LEN]="P"; /* send simple message*/
    		for(int i=0;i<connections;i++){    /* Send to all clients*/
    			if(socket_table[i].state== ONLINE){ /* Verify IF ative*/
    				if(send(socket_table[i].socket,buffer,sizeof(buffer),0)<0 ){ /* Send message*/
    						printf("Close connection timeout: Client %d\n",socket_table[i].index);
						shutdown(socket_table[i].socket,SHUT_RD);
						shutdown(socket_table[i].socket,SHUT_WR);
						shutdown(socket_table[i].socket,SHUT_RDWR);
						socket_table[i].state = OFFLINE;
    				}else if( clock() > socket_table[i].end_time){
    				
    				printf("Put AFK: Client %d\n",socket_table[i].index);
						socket_table[i].state = AFK;
    				}
    				
    			}
    		}
	}
	else if(signo==SIGINT){	/* handler of SIGNAL Run Interruption*/
		for(int i=0;i<connections;i++){    /* Close sockets*/
    			if(socket_table[i].state){ /* Verify IF ative*/
    				
				printf("Close connection: Client %d\n",socket_table[i].index);
				shutdown(socket_table[i].socket,SHUT_RD);
				shutdown(socket_table[i].socket,SHUT_WR);
				shutdown(socket_table[i].socket,SHUT_RDWR);
				socket_table[i].state = OFFLINE;
				}
			}		
		exit(1);
		
	}	                      
}

int main(int count, char *args[])
{	struct sockaddr_in addr;
	int listen_sd, port;
	if ( count != 2 )
	{
		printf("usage: %c <protocol or portnum>\n", args[0]);
		exit(0);
	}

	/*---Get server's IP and standard service connection--*/
	if ( !isdigit(args[1][0]) )
	{	
		struct servent *srv = getservbyname(args[1], "tcp");
		if ( srv == NULL )
			panic(args[1]);
		printf("%s: port=%d\n", srv->s_name, ntohs(srv->s_port));
		port = srv->s_port;
	}
	else					 //Se for um digito
		port = htons(atoi(args[1])); //Seleção da porta
		
	/*--- create socket ---*/
	listen_sd = socket(PF_INET, SOCK_STREAM, 0);  // Socket Listen
	if ( listen_sd < 0 )
		panic("socket");

	/*--- bind port/address to socket ---*/
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.s_addr = INADDR_ANY;                  //       IP 0.0.0.0 (/* any interface */)
	

	if ( bind(listen_sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
		panic("bind");

	/*--- make into listener with 10 slots ---*/
	if ( listen(listen_sd, 10) != 0 )
		panic("listen")

	/*--- begin waiting for connections ---*/
	else
	{	int st=0;
		pthread_t child4;
		pthread_create(&child4, 0, threadfuntion_scan,&st);       /* start thread */
		pthread_detach(child4);                    /* don't track it */
	
		signal(SIGALRM,sendPeriodicUpdate); // set signal (alarm)
		struct itimerval itv;
		itv.it_interval.tv_sec = 5;
		itv.it_interval.tv_usec = 0;
		itv.it_value.tv_sec = 5;
		itv.it_value.tv_usec = 0;
		setitimer (ITIMER_REAL, &itv, NULL); /* send signal to process for every 5 seconds*/
		while (1)                         /* process all incoming clients */
		{
			int n = sizeof(addr);
			int sd = accept(listen_sd, (struct sockaddr*)&addr, &n);     /* accept connection */
			if(sd!=-1)
			{
				/*Create socket*/
				pthread_t child;
				printf("New connection: Client %d\n",connections);
				socket_table[connections].socket=sd;
				socket_table[connections].state = ONLINE;		/*means connection opened*/
				socket_table[connections].index=connections;
				socket_table[connections].end_time=clock()+60000; /*End time 1 min*/
				client_socket_info_t *info;
				info = malloc (sizeof (client_socket_info_t)); /* allocate memory for socket information */
				info = &socket_table[connections++];
				pthread_create(&child, 0, threadfuntion, info);       /* start thread of braodcast */
				pthread_detach(child);                      /* don't track it */

			}
			
		}
	}
}
