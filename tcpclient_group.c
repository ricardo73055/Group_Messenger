
/*****************************************************************************/
/*** tcpclient.c                                                           ***/
/***                                                                       ***/
/*** Demonstrate an TCP client.                                            ***/
/*****************************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <mqueue.h>	/* mq_* functions */
#include <sys/types.h>
#include <resolv.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

/* name of the POSIX object referencing the queue */
#define MSGQOBJ_NAME    "/send_queue"
/* max length of a message (just for this process) */
#define MAX_MSG_LEN     100000
mqd_t msgq_id;
unsigned int sender;
int sd;
void panic(char *msg);
#define panic(m)	{perror(m); abort();}
/****************************************************************************/
/*** This program opens a connection to a server using either a port or a ***/
/*** service.  Once open, it sends the message from the command line.     ***/
/*** some protocols (like HTTP) require a couple newlines at the end of   ***/
/*** the message.                                                         ***/
/*** Compile and try 'tcpclient lwn.net http "GET / HTTP/1.0" '.          ***/
/****************************************************************************/

/*Function to receive from the sever*/
void *threadfuntion(void *arg)                    
{	
while(1){
	int sd = *(int*)arg;            /* get & convert the socket */
	char buffer[MAX_MSG_LEN ];
	char client=' ';
	
	

	recv(sd,buffer,sizeof(buffer),0);
	switch(buffer[0]){
	case 'S':
		switch(buffer[1]){
			case 'S':
				printf("Server send: %s\n",buffer+2);
			break;
			default:
				printf("Client %c send: %s\n",buffer[1],buffer+2);
				break;
		}
		break;
	case 'T':
		break;
	}
	
}
	                          /* terminate the thread */
}
/*Function to scan terminal and send the message to sever*/
void *threadfuntionsend(void *arg)                    
{	
while(1){
	int sd = *(int*)arg;            /* get & convert the socket */
	char msgcontent[MAX_MSG_LEN];
	int msgsz;
	
	//scanf("%s",buffer);
	 msgsz = mq_receive(msgq_id, msgcontent, MAX_MSG_LEN, &sender); /* Receive message by ICP message queue*/
    	if (msgsz == -1) {
        	perror("In mq_receive()");
        	exit(1);
    	}
    	
    	printf("You send: %s\n",msgcontent);
	send(sd,msgcontent,msgsz,0);
	
	}
}
static void int_handler(int signo){		
	if(signo==SIGINT){	/* handler of SIGNAL*/
		shutdown(sd,SHUT_RD);
		shutdown(sd,SHUT_WR);
		shutdown(sd,SHUT_RDWR);
		mq_close(msgq_id); /*Close queue*/
		exit(1);
		
	}
}
int main(int count, char *args[])
{	
	
    	
    	
    	 unsigned int msgprio = 1;
    	pid_t my_pid = getpid();
    	struct mq_attr msgq_attr;
    	
   	signal(SIGINT,  int_handler);
    	/* forcing specification of "-i" argument */
    	if (msgprio == 0) {
        	printf("Usage: %s [-q] -p msg_prio\n", args[0]);
        	exit(1);
    	}
    	/* opening the queue using default attributes  --  mq_open() */
  	msgq_id = mq_open(MSGQOBJ_NAME, O_RDWR | O_CREAT , S_IRWXU | S_IRWXG, NULL);
    	if (msgq_id == (mqd_t)-1) {
        	perror("In mq_open()");
        	exit(1);
    	}
    	 /* getting the attributes from the queue   --  mq_getattr() */
    		mq_getattr(msgq_id, &msgq_attr);
    		printf("Queue \"%s\":\n\t- stores at most %ld messages\n\t- large at most %ld bytes each\n\t- currently holds %ld messages\n",
                                                      MSGQOBJ_NAME,msgq_attr.mq_maxmsg, msgq_attr.mq_msgsize, msgq_attr.mq_curmsgs);


	struct hostent* host;
	struct sockaddr_in addr;
	int port;
	


	if ( count != 3 )
	{
		printf("usage: %s <servername> <protocol or portnum>\n", args[0]);
		exit(0);
	}

	/*---Get server's IP and standard service connection--*/
	host = gethostbyname(args[1]);
	//printf("Server %s has IP address = %s\n", args[1],inet_ntoa(*(long*)host->h_addr_list[0]));
	if ( !isdigit(args[2][0]) )
	{
		struct servent *srv = getservbyname(args[2], "tcp");
		if ( srv == NULL )
			panic(args[2]);
		printf("%s: port=%d\n", srv->s_name, ntohs(srv->s_port));
		port = srv->s_port;
	}
	else
		port = htons(atoi(args[2]));

	/*---Create socket and connect to server---*/
	sd = socket(PF_INET, SOCK_STREAM, 0);        /* create socket */
	if ( sd < 0 )
		panic("socket");
	memset(&addr, 0, sizeof(addr));       /* create & zero struct */
	addr.sin_family = AF_INET;        /* select internet protocol */
	addr.sin_port = port;                       /* set the port # */
	addr.sin_addr.s_addr = *(long*)(host->h_addr_list[0]);  /* set the addr */

	/*---If connection successful, send the message and read results---*/
	if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) == 0)
	{	
		pthread_t child,child1;
		
		pthread_create(&child, 0, threadfuntion, &sd);       /* start thread */
		pthread_detach(child);                      /* don't track it */
		pthread_create(&child1, 0, threadfuntionsend, &sd);       /* start thread */
		pthread_detach(child1);                      /* don't track it */
		while(1){}
		
	}
	else
		panic("connect");
}
