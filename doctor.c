#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#define	 MAXLINE					80
#define  MAXBUFLEN 				  	100 
#define  MAXDATASIZE				200 //max number of bytes server can receive at once
#define	 DOC1_PORT					"41736"
#define  DOC2_PORT					"42736"
#define  NO_OF_INSURANCE			3
//#define  VERBOSE					0

int main()
{
	pid_t		pid;
	FILE 		*fd;
	unsigned short int doctor_port, udp_port, patient_port;
	int 		sockfd;
	struct 		addrinfo hints, *servinfo, *p;
	int 		rv;
	int 		numbytes;
	struct 		sockaddr_storage patient_addr;
	char 		buf[MAXBUFLEN];
	socklen_t 	addr_len;
	char 		s[INET6_ADDRSTRLEN];
	char 		server_port[6];
	char 		buffer[MAXLINE];
	int 		count=0;
	int 		i;
	char 		doctor_name[9];
	struct 		sockaddr_in  *doctor_ipv4,*patient_ipv4;
	struct 		sockaddr_in6 *doctor_ipv6,*patient_ipv6;
	void 		*doctor_ip_addr;
	char 		doctor_ip[INET6_ADDRSTRLEN];
	char 		patient_ip[INET6_ADDRSTRLEN];
	struct insurance
	{
		char  insurance_name[12];
		char  insurance_cost[10];
	}insurance_info[NO_OF_INSURANCE];

	
	pid = fork();
	if( pid < 0)
	{
		printf("Fork() error\n");
		return 0;
	}
	else if( pid == 0 )
	{
		//Child Process - Doctor2
		strcpy(server_port,DOC2_PORT);
		strcpy(doctor_name, "Doctor 2");
		fd = fopen("doc2.txt", "r");
	}
	else
	{
		//Parent Process - Doctor1
		strcpy(server_port,DOC1_PORT);
		strcpy(doctor_name, "Doctor 1");
		fd = fopen("doc1.txt", "r");
	}
	count=0;
	while( fgets(buffer, MAXLINE, fd) != NULL )
	{
		if( count < NO_OF_INSURANCE)
		{
			if(sscanf(buffer, "%s %s", insurance_info[count].insurance_name, insurance_info[count].insurance_cost) == 2)
			{
				count++;
			}
		}
	}
	fclose(fd);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; 		// set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	
	/* 
	*	Code from Beej Socket Programming Tutorial: Reference#6 Starts here 
	*/ 
	if ((rv = getaddrinfo("nunki.usc.edu", server_port, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		p->ai_protocol)) == -1) 
		{
			perror("listener: socket\n\n");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("listener: bind\n\n");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "listener: failed to bind socket\n\n");
		return 2;
	}
	if (p->ai_family == AF_INET) 
	{ 	// IPv4
		doctor_ipv4 =(struct sockaddr_in *)p->ai_addr;
		doctor_ip_addr =  &(doctor_ipv4->sin_addr);
		udp_port = doctor_ipv4->sin_port;
	}
	else
	{
		// IPv6
		doctor_ipv6 = (struct sockaddr_in6 *)p->ai_addr;
		doctor_ip_addr = &(doctor_ipv6 ->sin6_addr);
		udp_port = doctor_ipv6->sin6_port;
	}
	inet_ntop(p->ai_family, doctor_ip_addr, doctor_ip, sizeof(doctor_ip));
	printf("%s has a static UDP port %hu and IP address %s\n\n",doctor_name, udp_port, doctor_ip);
	freeaddrinfo(servinfo);
	
	while(1)
	{
		addr_len = sizeof(patient_addr);
		memset(buf, 0, sizeof(buf));
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&patient_addr, &addr_len)) == -1) 
		{
			perror("recvfrom\n\n");
			continue;
		}
		buf[numbytes]='\0';
		//printf("Doctor:%s\n\n", buf);
		
		/* Extract Patient IP and port number */
		if (patient_addr.ss_family == AF_INET) 
		{
			patient_ipv4 = (struct sockaddr_in *)&patient_addr;
			patient_port = ntohs(patient_ipv4->sin_port);
			inet_ntop(AF_INET, &patient_ipv4->sin_addr, patient_ip, sizeof(patient_ip));
		} 
		else 
		{ // AF_INET6
			patient_ipv6 = (struct sockaddr_in6 *)&patient_addr;
			patient_port = ntohs(patient_ipv6->sin6_port);
			inet_ntop(AF_INET6, &patient_ipv6->sin6_addr, patient_ip, sizeof(patient_ip));
		}
		
		printf("%s receives the request from the patient with port number %hu and the insurance plan %s\n\n",doctor_name, patient_port, buf);
		
		/* Fetch insurance cost for the insurance the Patient has requested */
		for(i=0; i< NO_OF_INSURANCE; i++)
		{
			if( (strcmp(buf,insurance_info[i].insurance_name))== 0 )
			{
				memset(buf, 0, sizeof(buf));
				strcpy(buf,insurance_info[i].insurance_cost);
				break;
			}
		}
		/*if( i == NO_OF_INSURANCE)
		{
			strcpy(buf, "Invalid Insurance");
		}*/
		
		memset(buf, 0, sizeof(buf));
		if ((numbytes = sendto(sockfd, buf, strlen(buf), 0,
		(struct sockaddr *)&patient_addr, addr_len)) == -1) 
		{
			perror("talker: sendto\n\n");
			exit(1);
		}	
		
		printf("%s has sent estimated price $%s to patient with port number %hu\n\n",doctor_name, buf, patient_port);
	}
	close(sockfd);
	return 0;
}	
