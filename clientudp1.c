#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX 1024 //max len of buff
#define PORT 43454 //the port for transfering data
#define SA struct sockaddr

char buff[MAX], option, username[21], message[980], numUsers;
int sockfd, num=0, i, j;
struct sockaddr_in servaddr;
int len=sizeof(servaddr);

//receive thread
void *recvFunc(void * args)
{
	while(1)
	{
		bzero(buff,sizeof(buff));
		//receive some data from server 
		if(recvfrom(sockfd,buff,sizeof(buff),0,(SA *)&servaddr,&len)==-1)//receive data
		{
			perror("recvfrom(): ");
			close(sockfd);
			exit(1);
		}
		//printf("%s\n",buff);
		bcopy(buff, &option, 1);
		switch(option)
		{

			case '2':
				printf("Online users: ");
				bcopy(&buff[1],&numUsers,1);
				printf("%c\n",numUsers);
				num = numUsers - '0';
				j=2;
				for(i=0;i<num;i++)
				{
					bzero(username,21);
					bcopy(&buff[j],username,20);
					j+=20;
					printf("%s\n",username);
				}
				break;
			
			case '3':
				bzero(username,21);
				bcopy(&buff[1], username, 20);
				bzero(message,980);
				bcopy(&buff[21],message,979);
				printf("%s: %s\n",username, message);
				break;	
			
		}
	}
}

//send thread
void *sendFunc(void * args)
{	
	int n;
	//repeate while ctrl+z
	while(1)
	{	
		scanf("%c",&option);
		bzero(buff,MAX);
		bcopy(&option, buff,1);
		switch(option)
		{
			case '0':
				if(sendto(sockfd,buff,sizeof(buff),0,(SA *)&servaddr,len)==-1)//send data
				{
					perror("sendto(): ");
					close(sockfd);
					exit(0);
				}
				close(sockfd);
				exit(0);

			/*case '1':
				bzero(username,21);
				n=0;
				printf("Enter your username: ");
				scanf("%s", username);
				bcopy(username, &buff[1], 20);
				if(sendto(sockfd,buff,sizeof(buff),0,(SA *)&servaddr,len)==-1)//sent read data 
				{
					perror("sendto(): ");
					return 0;
				}
				//printf("%s\n",username);
				//printf("%s\n",buff);
				break;
				*/
			case '2':
				//printf("%s\n",buff);
				if(sendto(sockfd,buff,sizeof(buff),0,(SA *)&servaddr,len)==-1)//sent read data 
				{
					perror("sendto(): ");
					return 0;
				}
				//printf("Information sent!\n");
				break;
			
			case '3':
				bzero(username,21);
				printf("Enter username: ");
				scanf("%s", username);
				bcopy(username, &buff[1], 20);
				bzero(message,980);
				printf("Enter your message: ");
				scanf("%s", message);
				bcopy(message, &buff[21], 979);
				if(sendto(sockfd,buff,sizeof(buff),0,(SA *)&servaddr,len)==-1)//sent read data 
				{
					perror("sendto(): ");
					return 0;
				}
				break;	
			
		}	
		
	}	
}

int main(int argc, char *argv[])
{
	pthread_t recv_thread, send_thread;
	int n,i,res;
	sockfd=socket(AF_INET,SOCK_DGRAM,0); //open socket
	if(sockfd==-1) //check is socket correctly opened
	{
		printf("socket creation failed...\n");
		exit(0);
	}	
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr,sizeof(len));
	//set options to sockaddr_in structure
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	servaddr.sin_port=htons(PORT);
	//create thread for recvFunc
	if (pthread_create(&recv_thread, NULL, &recvFunc, NULL) != 0) 	
	{
        	printf("error\n");
		close(sockfd);
        	exit(1);
    	}
	//create thread for sendFunk
	if (pthread_create(&send_thread, NULL, &sendFunc, NULL) != 0) 	
	{
        	printf("error\n");
		close(sockfd);
        	exit(2);
    	}

	printf("Menu:\n");
	printf("0.Exit.\n");
	printf("2.Show me all users.\n");
	printf("3.Send message.\n");
	option='1';
	bzero(buff,MAX);
	bcopy(&option, buff,1);
	bzero(username,21);
	printf("Enter your username: ");
	scanf("%s", username);
	bcopy(username, &buff[1], 20);
	if(sendto(sockfd,buff,sizeof(buff),0,(SA *)&servaddr,len)==-1)//sent read data 
	{
		perror("sendto(): ");
		return 0;
	}
	//join send_thread	
	pthread_join(send_thread,NULL);
	//join recv_thread		
	pthread_join(recv_thread,NULL);
		
		
	close(sockfd);
}
