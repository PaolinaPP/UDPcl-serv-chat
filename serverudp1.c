#include<stdio.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>

#define MAX 1024 //max len of buff
#define PORT 43454 //the port for transfering data
#define SA struct sockaddr 

struct client
{
	struct sockaddr_in cl;
	char username[21];
	struct client * next;
};

struct client *head=NULL;
struct client *tail=NULL;
struct client *newd;
char buff[MAX];

void add_user(struct sockaddr_in cl, char username[20])
{	
	int i;
	newd=(struct client*)malloc(sizeof(struct client));
	if(head==NULL)
	{
		head=newd;
   	}
	else
	{
	   	tail->next=newd;
	}
	
	newd->cl=cl;
	for(i=0;i<20;i++)
	{
		newd->username[i]=username[i];
	}
	newd->next=NULL;
	tail=newd;
	printf("User added!\n");
	return ;	
}

void all_users(int allUsers)
{
	char n;
	n=allUsers + '0';
	int i=2;
	bcopy(&n, &buff[1], 1);
	struct client *p=head;
	while(p!=NULL)
	{
		printf("%s\n",p->username);
		bcopy(p->username, &buff[i], 20);
		i+=20;
		p=p->next;
	}
	//printf("\n%s",buff);
}



//returns port
struct sockaddr_in searchList(char username[20])
{
	struct client *p=head;
	while(p!=NULL)
	{
        	if(strcmp(p->username, username)==0)
		{
          		printf("Find it!\n");
          		return p->cl;
        	}
        	else
		{
          		p=p->next;
        	}
	}
}

char * searchClient(int port)
{
	struct client *p=head;
	while(p!=NULL)
	{
        	if(ntohs(p->cl.sin_port)==port)
		{
          		printf("Find it!\n");
          		return p->username;
        	}
        	else
		{
          		p=p->next;
        	}
	}
	return "error";
}

int clientExist(int port)
{
	struct client *p=head;
	while(p!=NULL)
	{
        	if(ntohs(p->cl.sin_port)==port)
		{
          		printf("Find it!\n");
          		return 1;
        	}
        	else
		{
          		p=p->next;
        	}
	}
	return 0;
}

int send_recv(int sockfd)
{
	//printf("in send and recv");
	char message[980];	
	char username[21], comm; 
	int allUsers=0, clen, i, flag=0, toClientPort=0, clientPort, num, res, command;
	struct sockaddr_in cli, tocli;
	clen=sizeof(cli);
	//repeate while ctrl+z
	while(1)
	{
		flag=0; //flag to check is this client already exist in the array of clients 0-no 1-yes
		
 		//clean buff for the new msg
		bzero(buff,MAX); 
		//receive data from client and save it in buff
		if(recvfrom(sockfd,buff,sizeof(buff),0,(SA *)&cli,&clen)==-1) //receive data
		{
			perror("recvfrom(): ");
			return 0;
		} 
		bcopy(buff, &comm, 1);
		command=comm-'0';
		
		if(command==0)
		{
			close(sockfd);
			exit(0);
		}	
		else if(command==1)
		{
			//printf("in option 1\n");
			clientPort=clientExist(ntohs(cli.sin_port));
			if(clientPort==1) continue;
			bzero(username,21);
			bcopy(&buff[1], username, 20);
			//printf("%s\n", username);
			//printf("after printf\n");
			add_user(cli,username);
			allUsers++;
			
		}
		else if(command==2)
		{
			bzero(buff,MAX);
			bcopy(&comm, buff, 1);
			all_users(allUsers);
			if(sendto(sockfd,buff,sizeof(buff),0,(SA *)&cli,clen)==-1)//sent read data 
			{
				perror("sendto(): ");
				return 0;
			}
			
			
		}
		else if(command==3)
		{
			
			bzero(username,21);
			bcopy(&buff[1], username, 20);
			tocli=searchList(username);
			bzero(message,980);
			bcopy(&buff[21],message,979);
			bzero(buff,MAX);
			bcopy(&comm,buff,1);
			bzero(username,21);
			char * uname=searchClient(ntohs(cli.sin_port));
			strncpy(username, uname,20);
			printf("%s\n",username);
			bcopy(username,&buff[1],20);
			bcopy(message,&buff[21],979);
			if(sendto(sockfd,buff,sizeof(buff),0,(SA *)&tocli,clen)==-1)//sent read data 
			{
				perror("sendto(): ");
				return 0;
			}
			
		}
	}
	return 1;
}



int main()
{
	int sockfd; //socket file descriptor
	struct sockaddr_in servaddr;
	sockfd=socket(AF_INET,SOCK_DGRAM,0); //open socket
	if(sockfd==-1) //check is socket correctly opened
	{
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr,sizeof(servaddr)); //same as memset()
	//set options to sockaddr_in structure
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(PORT);
	//assign sockfd to servaddr (bind)
	if((bind(sockfd,(SA *)&servaddr,sizeof(servaddr)))!=0)
	{
		printf("socket bind failed...\n");
		close(sockfd);
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");
	//go to send_recv function
	if(send_recv(sockfd)==0)
	{
		printf("Can't send or receive data!");
		close(sockfd);
		exit(1);
	}
	close(sockfd);
}

