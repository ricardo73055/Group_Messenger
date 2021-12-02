#include <stdio.h>
#include <mqueue.h>   /* mq_* functions */
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

/* name of the POSIX object referencing the queue */
#define MSGQOBJ_NAME    "/send_queue"
/* max length of a message (just for this process) */
#define MAX_MSG_LEN     70

int main(int argc, char *argv[]) {
    mqd_t msgq_id;
    unsigned int msgprio = 1;
    pid_t my_pid = getpid();
    if(argc!=2){
    	printf("Usage: %s \"Message to send\"\n", argv[0]);
    	exit(1);
    }
    time_t currtime;
   
    /* opening the queue using default attributes  --  mq_open() */
    msgq_id = mq_open(MSGQOBJ_NAME, O_RDWR | O_CREAT , S_IRWXU | S_IRWXG, NULL);
    if (msgq_id == (mqd_t)-1) {
        perror("In mq_open()");
        exit(1);
    }
   
   
    /* sending the message      --  mq_send() */
    mq_send(msgq_id,argv[1], strlen(argv[1])+1, msgprio);
    /* closing the queue        -- mq_close() */
    mq_close(msgq_id);     
    return 0;
}
