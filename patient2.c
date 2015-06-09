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
#define  HEALTH_CENTER_SERVER_PORT  "21736"
#define  MAXLINE 				  	80
#define  MAXDATASIZE				200 //max number of bytes server can receive at once
//#define  VERBOSE					0
int main()
{
	FILE 	*fd = NULL;
	char 	name[MAXLINE];
	char	passwd[MAXLINE];
	char 	p2_name[MAXLINE];
    char 	p2_password[MAXLINE];
	char 	buffer[MAXLINE];
	struct 	addrinfo hints;
	struct 	addrinfo *servinfo, *p;
	struct 	sockaddr_in	clientinfo;
	int 	rv;
	int 	sockfd;
	char 	*authenticate_msg=NULL;
	int 	msg_len = strlen("authenticate");
	char 	buf[MAXDATASIZE];
	int 	numbytes;
	char 	index[4];
	char 	index_options[6][4];
	char 	* ptr_ch=NULL;
	int 	i,count;
	char 	port[6];
	int 	len;
	unsigned short int doctor_port;
	struct 	sockaddr_in  *doctor_ipv4;
	struct 	sockaddr_in6 *doctor_ipv6;
	void 	*doctor_ip_addr;
	char 	doctor_ip[INET6_ADDRSTRLEN];
	struct 	sockaddr_storage their_addr;
	socklen_t addr_len;
	char 	doctor_name[6];
	struct 	hostent *udp_ip;
	struct 	in_addr **addr_list;
	
	fd = fopen("patient2.txt", "r");
	if( NULL == fd )
	{
		printf("Error in opening patient2.txt\n\n");
		return 0;
	}
	if( fgets(buffer, MAXLINE, fd)!=NULL)
	{
		if(sscanf(buffer, "%s %s", name, passwd) == 2)
		{
			strcpy(p2_name,name);
			strcpy(p2_password,passwd);
		}
	}	
#ifdef VERBOSE
	printf("Patient2: %s %s\n\n",p2_name, p2_password);
#endif
	fclose(fd);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	/* 
	*	Code from Beej Socket Programming Tutorial: Reference#3 Starts here 
	*/ 
	if ((rv = getaddrinfo("nunki.usc.edu", HEALTH_CENTER_SERVER_PORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n\n", gai_strerror(rv));
		exit(1);
	}
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		p->ai_protocol)) == -1) 
		{
			perror("client: socket\n\n");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("client: connect\n\n");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "client: failed to connect\n\n");
		return 2;
	}
	
	/* 
	*	Code from Beej Socket Programming Tutorial: Reference#3 Ends here 
	*/ 
	
	freeaddrinfo(servinfo); // all done with this structure
	len = sizeof(clientinfo);
	rv = getsockname(sockfd, (struct sockaddr *)&clientinfo, &len);

	if	(rv == -1)	
	{
		perror("getsockname\n\n");
		exit(1);
	}
	printf("Phase 1: Patient 2 has TCP port number %hu and IP address %s\n\n",clientinfo.sin_port, inet_ntoa(clientinfo.sin_addr));
	
	msg_len +=strlen(p2_name) + strlen(p2_password);
	authenticate_msg=(char *)malloc(msg_len+3);
	strcat(authenticate_msg,"authenticate");
	strcat(authenticate_msg," ");
	strcat(authenticate_msg,p2_name);
	strcat(authenticate_msg," ");
	strcat(authenticate_msg,p2_password);
#ifdef VERBOSE
	printf("%s\n", authenticate_msg);
#endif
	/*Patient 2 sends authentication request to health center server */
	if ((send(sockfd, authenticate_msg, strlen(authenticate_msg)+1, 0)) == -1) 
	{
		perror("send\n\n");
		close(sockfd);
		exit(1);
	}
	printf("Phase 1: Authentication request from Patient 2 with username %s and password %s has been sent to the Health Center Server\n\n", p2_name, p2_password);
	
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) 
	{
		perror("recv");
		close(sockfd);
		exit(1);
	}
	else if ( numbytes == 0)
	{
		printf("Connection closed by remote host\n\n");
		close(sockfd);
		exit(1);
	}
	buf[numbytes] = '\0';
	
	printf("Phase 1: Patient 2 authentication result: %s\n\n", buf);
	printf("Phase 1: End of Phase1 for Patient2\n\n");
	
	if( (strcmp(buf,"failure"))==0)
	{	
		close(sockfd);
		return 0;	
	}
	else
	{	/* Patient 2 requests for available appointments */
		if ((send(sockfd, "available", 10, 0)) == -1) 
		{
			perror("send");
			close(sockfd);
			exit(1);
		}
		
		memset(buf, 0, sizeof(buf));
		
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) 
		{
			perror("recv");
			close(sockfd);
			exit(1);
		}
		else if ( numbytes == 0)
		{
			printf("Connection closed by remote host\n\n");
			close(sockfd);
			exit(1);
		}
		buf[numbytes] = '\0';
		if( buf[0] == '\0')
		{
			close(sockfd);
			exit(1);
		}
		printf("Phase 2: The following appointments are available for Patient2:\n\n");
		printf("%s\n\n",buf);
		
		ptr_ch = strtok(buf, " ");
		strcpy(index_options[0], ptr_ch);
		count=1;
		while((ptr_ch != NULL)&& (count<6))
		{	
			ptr_ch=strtok(NULL, "\n");
			if(ptr_ch != NULL)
			{
				ptr_ch=strtok(NULL, " ");
				if( ptr_ch!=NULL)
				{
					strcpy(index_options[count],ptr_ch);
					count++;
				}
			}
		}
#ifdef VERBOSE
		printf("Options: %s %s %s %s %s %s\n\n", index_options[0], index_options[1], index_options[2],index_options[3], index_options[4], index_options[5]);
#endif
		while(1)
		{
			printf("Please enter the preferred appointment index and press enter:\n\n");
			scanf("%s",index);
			for(i=0; i<count; i++)
			{
				if( (strcmp(index_options[i], index))== 0)
					break;
			}
			if( i == count)
				printf("Incorrect choice\n\n");
			else
				break;
		}
		strcpy(buf, "selection ");
		strcat(buf, index);
#ifdef VERBOSE
		printf("\n\nPatient2 appointment choice message: %s\n\n", buf);
#endif
		/* Patient2 sends the preferred appointment */
		if ((send(sockfd, buf, strlen(buf)+1, 0)) == -1) 
		{
			perror("send");
			close(sockfd);
			exit(1);
		}
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) 
		{
			perror("recv");
			close(sockfd);
			exit(1);
		}
		else if ( numbytes == 0)
		{
			printf("Connection closed by remote host\n\n");
			close(sockfd);
			exit(1);
		}
		buf[numbytes] = '\0';
		if( strcmp(buf, "notavailable")==0)
		{
#ifdef VERBOSE
			printf("Reservation not available\n\n");
#endif			
			printf("Phase 2: The requested appointment from Patient2 is not available, Exiting...\n\n");
			close(sockfd);
			return 0;
		}
		else
		{	
			/*Extract doctor's port number if appointment is available */
			ptr_ch=strtok(buf, " ");
			if(ptr_ch !=NULL)
			{
				ptr_ch=strtok(NULL, "\n");
				strcpy(port, ptr_ch);
				doctor_port=(unsigned int)atoi(port);
#ifdef VERBOSE
				printf("Doctor port: %d\n\n", doctor_port);
#endif
				printf("Phase 2: The requested appointment is available and reserved to Patient2. The assigned doctor port number is %u\n\n", doctor_port);
			}
			close(sockfd);
			fd = fopen("patient1insurance.txt", "r");
			if( fd == NULL )
			{
				printf("Error in opening patient2insurance.txt\n\n");
				return 0;
			}
			if( fgets(buffer, MAXLINE, fd)!=NULL)
			{
				if(sscanf(buffer, "%s", buf) == 1)
					;
			}	
			//UDP socket 
			memset(&hints, 0, sizeof hints);
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_DGRAM;
			if ((rv = getaddrinfo("nunki.usc.edu", port, &hints, &servinfo)) != 0) 
			{
				fprintf(stderr, "getaddrinfo: %s\n\n", gai_strerror(rv));
				return 1;
			}
			// loop through all the results and make a socket
			/* 
			*	Code from Beej Socket Programming Tutorial: Reference#5 Starts here 
			*/ 
			for(p = servinfo; p != NULL; p = p->ai_next) 
			{
				if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
				{
					perror("Client: socket\n\n");
					continue;
				}
				break;
			}
			if (p == NULL) 
			{
				fprintf(stderr, "Client: failed to bind socket\n\n");
				return 2;
			}
			/* 
			*	Code from Beej Socket Programming Tutorial: Reference#5 Ends here 
			*/ 
			if (p->ai_family == AF_INET) 
			{ 	// IPv4
				doctor_ipv4 =(struct sockaddr_in *)p->ai_addr;
				doctor_ip_addr =  &(doctor_ipv4->sin_addr);
				doctor_port = doctor_ipv4->sin_port;
			}
			else
			{
				// IPv6
				doctor_ipv6 = (struct sockaddr_in6 *)p->ai_addr;
				doctor_ip_addr = &(doctor_ipv6 ->sin6_addr);
				doctor_port = doctor_ipv6->sin6_port;
			}
			inet_ntop(p->ai_family, doctor_ip_addr, doctor_ip, sizeof(doctor_ip));
			/*Patient 2 sends insurance information to doctor */
			if ((numbytes = sendto(sockfd, buf, strlen(buf)+1, 0, p->ai_addr, p->ai_addrlen)) == -1) 
			{
				perror("Client: sendto\n\n");
				close(sockfd);
				exit(1);
			}
			len = sizeof(clientinfo);
			rv = getsockname(sockfd, (struct sockaddr *)&clientinfo, &len);
			if	(rv == -1)	
			{
				perror("getsockname\n\n");
				close(sockfd);
				exit(1);
			}
			if ((udp_ip = gethostbyname("nunki.usc.edu")) == NULL) {  // get the host info
				herror("gethostbyname");
				close(sockfd);
				return 2;
			}
			addr_list = (struct in_addr **)udp_ip->h_addr_list;
			printf("Phase 3: Patient 2 has a dynamic UDP port number %hu and IP address %s\n\n",clientinfo.sin_port, inet_ntoa(*addr_list[0]));
			printf("Phase 3: The cost estimation request from Patient 2 with insurance plan %s has been sent to the doctor with port number %hu and IP address %s\n\n",buf, doctor_port, doctor_ip);
			addr_len = sizeof(their_addr);
			/* Patient 2 receives the insurance cost from the corresponding doctor */
			if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) 
			{
				perror("recvfrom\n\n");
				close(sockfd);
				exit(1);
			}
			buf[numbytes]='\0';
			printf("Phase 3: Patient1 receives $%s estimation cost from doctor with port number %hu and name %s\n\n", buf, doctor_port, doctor_name);
			
			printf("Phase 3: End of Phase 3 for Patient1\n\n");
			close(sockfd);
		}	
		
	}		
	return 0;	
}
