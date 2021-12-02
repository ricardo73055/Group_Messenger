CC = gcc
#CC = ~/buildroot/buildroot-2021.08/output/host/bin/./aarch64-buildroot-linux-gnu-gcc

IP = 10.42.0.150
PORT = 5001
ADDR = 0.0.0.0  


# Compile Server
server: 
	gcc tcpserver_group.c -o tcpserver_group.elf -lpthread
	
#Run Server	
runserver:	
	./tcpserver_group.elf $(PORT)
#Compile Client
client: 
	$(CC) tcpclient_group.c -o tcpclient_group.elf -lpthread -lrt
	
#Run Client
runclient:
	./tcpclient_group.elf $(ADDR) $(PORT)
	
	
#Compile send (will send via Posix Message Queue to client service)
clientsend: 
	$(CC) client_send.c -o send.out -lpthread -lrt

send:
	./send.out "Predefined message"



clean: 
	rm -f  *.elf *.out
	
stat: 
	netstat -ta
	
transfer: 
	scp tcpclient_group.elf send.out root@$(IP):/etc
	
open: 
	ssh root@$(IP)
