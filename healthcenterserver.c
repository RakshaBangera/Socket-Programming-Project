// THIS IS AN COMPLETE AUTHENTIC CODE. ANY RESEMBLENCE TO OTHER CODE IS COINCIDENCE..... bANGUDE (BIGGEST JERK)
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
#define  NO_OF_PATIENTS 		  	2
#define  MAXLINE 				  	80
#define  HEALTH_CENTER_SERVER_PORT "21736"
#define	 BACKLOG 				  	10
#define  MAXDATASIZE				200 //max number of bytes server can receive at once
#define  NO_OF_APPOINTMENTS			6
//#define  VERBOSE					0

int main()
{
	FILE 		*fd = NULL;
	char 		name[MAXLINE], passwd[MAXLINE];
	char 		username[NO_OF_PATIENTS][MAXLINE];
	char 		password[NO_OF_PATIENTS][MAXLINE];
	char 		buffer[MAXLINE];
	int 		count=0;
	int 		i;
	struct 		hostent *hostip;
	struct 		in_addr **addr_list;
	char 		health_center_ip[INET6_ADDRSTRLEN];
	struct 		addrinfo hints, *servinfo, *p;
	struct 		sockaddr_in *health_center_addr_ipv4, *patient_ipv4;
	struct 		sockaddr_in6 *health_center_addr_ipv6, *patient_ipv6;
	void 		*health_center_ip_addr;
	int 		rv;
	int 		sockfd, new_fd;
	struct 		sockaddr_storage patient_addr; // Patient's(client's) address information
	socklen_t 	sin_size;
	char 		patient_ip[INET6_ADDRSTRLEN];
	char 		buf[MAXDATASIZE];
	int 		numbytes;
	char 		*client_msg=NULL;
	char 		*client_username=NULL, *client_password=NULL;
	char 		received_index[4];
	char 		*ptr_ch;
	char 		doc_port[33];
	int 		index;
	int 		temp_port;
	int 		len;
	unsigned short server_port, patient_port;
	struct availability
	{
	char time_index[4];
	char day[4];
	char time[5];
	char doctor[4];
	unsigned short int port;
	int reserved;
	}appointments[NO_OF_APPOINTMENTS];

	fd = fopen("users.txt", "r");
	if( NULL == fd )
	{
		printf("Error in opening users.txt\n\n");
		return 0;
	}

	/* Read each line of users.txt and fetch username and passwords */
	while( fgets(buffer, MAXLINE, fd) != NULL )
	{
	if( count < NO_OF_PATIENTS)
	{
		if(sscanf(buffer, "%s %s", name, passwd) == 2)
		{
				strcpy(username[count],name);
				strcpy(password[count],passwd);
		}
		else
			break;
	}
	else
		break;
	count++;
	}
	fclose(fd);

	count=0;
	fd = fopen("availabilities.txt", "r");
	/*Fetch the availabilities and store the parsed values in struct appointments */
	while( fgets(buffer, MAXLINE, fd) != NULL )
	{
	if( count < NO_OF_APPOINTMENTS )
	{
		if(sscanf(buffer, "%s %s %s %s %hu", appointments[count].time_index, appointments[count].day, appointments[count].time, appointments[count].doctor, &appointments[count].port)==5)
		{
			appointments[count].reserved=0; //Initially all appointments are available
		}
		else
			break;
	}
	else	
		break;
	count++;
	}
	fclose(fd);
	#ifdef VERBOSE
	printf("Appointments:\n\n");
	for( count=0; count < NO_OF_APPOINTMENTS; count++)
	{
	printf("%s %s %s %s %hu %d\n", appointments[count].time_index, appointments[count].day, appointments[count].time, appointments[count].doctor, appointments[count].port, appointments[count].reserved);
	}
	#endif
	memset(&hints, 0, sizeof hints);        //Clear struct hints empty		
	hints.ai_family 	= AF_UNSPEC; 	   // Either IPv4 or IPv6
	hints.ai_socktype   = SOCK_STREAM;    // TCP stream sockets

	if ((rv = getaddrinfo("nunki.usc.edu", HEALTH_CENTER_SERVER_PORT, &hints, &servinfo)) != 0) 
	{
	fprintf(stderr, "getaddrinfo: %s\n\n", gai_strerror(rv));
	exit(1);
	}

	/* 
	*	Code Beej Networking Tutorial: Reference#1 Starts here 
	*/
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
	if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) 
	{
		perror("server: socket\n\n");
		continue;
	}

	if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
	{
		 close(sockfd);
		 perror("server: bind\n\n");
		 continue;
	}
	/* if we get here, we must have connected successfully */
	break; 
	}

	/* looped off the end of the list with no connection */
	if (p == NULL) 
	{
	fprintf(stderr, "failed to bind\n\n");
	exit(2);
	}
	/* 
	*	Code from Beej Socket Programming Tutorial: Reference#1 Ends here 
	*/
	freeaddrinfo(servinfo); // all done with this structure

	/* IPV4 */
	if (p->ai_family == AF_INET) 
	{ 	
	health_center_addr_ipv4 =(struct sockaddr_in *)p->ai_addr;
	health_center_ip_addr =  &(health_center_addr_ipv4->sin_addr);
	server_port = ntohs(health_center_addr_ipv4->sin_port);
	}
	/* IPV6 */
	else
	{
	health_center_addr_ipv6 = (struct sockaddr_in6 *)p->ai_addr;
	health_center_ip_addr = &(health_center_addr_ipv6 ->sin6_addr);
	server_port = ntohs(health_center_addr_ipv6->sin6_port);
	}
	inet_ntop(p->ai_family, health_center_ip_addr, health_center_ip, sizeof(health_center_ip));

	/*Phase1 Message */
	printf("Phase 1: The Health Center Server has port number %hu and IP address %s\n\n", server_port, health_center_ip);

	if ((listen(sockfd, BACKLOG)) == -1) 
	{
	 perror("listen\n\n");
	 close(sockfd);
	 exit(1);
	}

	/* main accept loop*/
	while(1)  
	{ 
	 sin_size = sizeof(patient_addr);
	 new_fd = accept(sockfd, (struct sockaddr *)&patient_addr, &sin_size);
	#ifdef VERBOSE
	 printf("Connection accepted\n\n");
	#endif
	 if (new_fd == -1) 
	 {
		 perror("accept\n\n");
		 continue;
	 }
	 
	/* Fetch the address of the patient which made a connection to the Health Center Server */
	len=sizeof(patient_addr);
	getpeername(new_fd, (struct sockaddr*)&patient_addr, &len);
	if (patient_addr.ss_family == AF_INET) 
	{
		patient_ipv4 = (struct sockaddr_in *)&patient_addr;
		patient_port = ntohs(patient_ipv4->sin_port);
		inet_ntop(AF_INET, &patient_ipv4->sin_addr, patient_ip, sizeof(patient_ip));
	} 
	else 
	{ 
		patient_ipv6 = (struct sockaddr_in6 *)&patient_addr;
		patient_port = ntohs(patient_ipv6->sin6_port);
		inet_ntop(AF_INET6, &patient_ipv6->sin6_addr, patient_ip, sizeof(patient_ip));
	}

	/* Authentication Request from Patient */
	if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) 
	{
		perror("recv");
		close(new_fd);
		continue;
	}
	else if ( numbytes == 0)
	{
		printf("Connection closed by remote host\n\n"); 
		close(new_fd);
		continue;
	}
	buf[numbytes] = '\0';

	client_msg = strtok(buf, " ");
	client_msg = strtok(NULL, " ");
	client_username = (char *)malloc(strlen(client_msg)+1);
	strcpy(client_username, client_msg);
	client_msg = strtok(NULL, " ");
	client_password = (char *)malloc(strlen(client_msg)+1);
	strcpy(client_password , client_msg);

	printf("Phase 1:The Health Center Server has received request from a patient with username %s and password %s\n\n", client_username, client_password);

	/* Patient Authentication Success */
	if( ((strcmp(client_username,username[0])== 0) && (strcmp(client_password,password[0])== 0)) ||
		((strcmp(client_username,username[1])== 0) && (strcmp(client_password,password[1])== 0)))
	{
		printf("Phase 1:The Health Center Server sends the response success to patient with username %s\n\n",client_username);
		if ((send(new_fd, "success", 8, 0)) == -1) 
		{
			perror("send\n\n");
			close(new_fd);
			continue;
		}
	}
	else /* Authentication Failed */
	{
		printf("Phase 1:The Health Center Server sends the response failure to patient with username %s\n\n",client_username);
		if ((send(new_fd, "failure", 8 , 0)) == -1) 
		{
			perror("send\n\n");
			close(new_fd);
			continue;
		}
		continue;
	}

	memset(buf, 0, MAXDATASIZE);
	/* Patient requests for available appointments */
	if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) 
	{
		perror("recv\n\n");
		close(new_fd);
		continue;
	}
	else if ( numbytes == 0)
	{
		printf("Connection closed by remote host\n\n");
		close(new_fd);
		continue;
	}
	buf[numbytes] = '\0';
		
	if( (strcmp(buf,"available"))==0)
	{
		printf("Phase 2: The Health Center Server, receives a request for available time slots from patients with port number %hu and IP address %s\n\n",patient_port,patient_ip);
		memset(buf, 0, sizeof(buf));
		for( i=0; i < NO_OF_APPOINTMENTS; i++)
		{
			if(appointments[i].reserved == 0)
			{
				strcat(buf, appointments[i].time_index);
				strcat(buf, " ");
				strcat(buf, appointments[i].day);
				strcat(buf, " ");
				strcat(buf, appointments[i].time);
				strcat(buf, "\n");
			}
		}
	#ifdef VERBOSE
		printf("Available Appointments:\n%s\n\n", buf);
	#endif		
		/* Send available appointments */
		if ((send(new_fd, buf, strlen(buf)+1 , 0)) == -1) 
		{
			perror("send\n\n");
			close(new_fd);
			continue;
		}	
		
		printf("Phase 2: The Health Center Server sends available time slots to patient with username %s\n\n", client_username);
		
		/* Patient's preferred appointment */
		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) 
		{
			perror("recv\n\n");
			close(new_fd);
			continue;
		}
		else if ( numbytes == 0)
		{
			printf("Connection closed by remote host\n\n");
			close(new_fd);
			continue;
		}
		buf[numbytes] = '\0';

		if( buf!=NULL)
		{
			ptr_ch = strtok(buf, " ");
			if(ptr_ch !=NULL)
			{
				ptr_ch = strtok(NULL, "\n");
				strcpy(received_index, ptr_ch);
			}
		}
	#ifdef VERBOSE
		printf("Received index: %s\n\n", received_index);
	#endif
		index = atoi(received_index);
		printf("Phase 2: The Health Center Server receives a request for appointment %d from patient with port number %hu and username %s\n\n", index, patient_port, client_username);
		if( appointments[index-1].reserved == 0)
		{ 
			appointments[index-1].reserved=1;   //Mark appointment as reserved
			memset(buf, 0, sizeof(buf));
			strcat(buf, appointments[index-1].doctor);
			strcat(buf, " ");
			//temp_port = (int)appointments[index-1].port;
			//itoa(temp_port, doc_port, 10);
			sprintf(doc_port,"%u",appointments[index-1].port);
			strcat(buf, doc_port );
			if ((send(new_fd, buf, strlen(buf)+1 , 0)) == -1) 
			{
				perror("send\n\n");
				close(new_fd);
				continue;
			}	
			printf("Phase 2: The Health Center Server confirms the following appointment %d to patient with username %s\n\n", index, client_username);
		}
		else
		{	
			memset(buf, 0, sizeof(buf));
			strcat(buf, "notavailable");
			if ((send(new_fd, buf, strlen(buf)+1 , 0)) == -1) 
			{
				perror("send\n\n");
				close(new_fd);
				continue;
			}
			printf("Phase 2: The Health Center Server rejects the following appointment %d to patient with username %s\n\n", index, client_username);
		}
	#ifdef VERBOSE
		printf("Server doctor info: %s\n\n", buf);
	#endif							
	}
	else
	{	
		printf("Invalid command received\n\n");
		break;
	}
	close(new_fd); // parent doesn't need this
	}
	close(sockfd);
	return 0;		
}
