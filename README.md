# Socket-Programming-Project

a. === Full Name ===
   Raksha Madhava Bangera

b. === Student ID ===
   1177-5517-36

c. === Project Summary ===
   This project demonstrates the communication using TCP and UDP sockets by implementing an online medical system. The online medical system comprises of:
   1. HealthCenter Server
   2. 2 Patients
   3. 3 Doctors.
   The program establishes a TCP connection between a patient and the Health Center Server. The authentication of server and the list of available appointments 
   are sent over this channel. Once the patient is authenticated and receives an appointment with one of the doctors, the program then establishes a UDP
   connection between the corresponding doctor and the patient. The patient receives the etimated insurance cost via this communication channel.
   
d. === Code Files ===
   1. HealthCenterServer.c	:Authenticates the patient that connects to it via TCP socket by validating the username and password sent by the patient through 
							the socket. On successful authentication and request for appointments for doctors, the healthcenter server sends out the list of		
							available appointments via the same TCP socket. 
	
   2. Patient1.c 		  	:Opens a TCP connection to the healthcenter server and sends its username and password for the server to validate. After succesful 
							validation, it requests the server to send out the list of available appointments via the same TCP connection. The Patient selects
							one of the appointment index and obtains the doctor port it has to connect to from the health center server.
							Patient then opens an UDP connection to the obtained doctor port. The patient sends its insurance type and obtains the cost of 
							insurance from the doctor on the UDP channel.
							
   3. Patient2.c			:Opens a TCP connection to the healthcenter server and sends its username and password for the server to validate. After succesful 
							validation, it requests the server to send out the list of available appointments via the same TCP connection. The Patient selects
							one of the appointment index and obtains the doctor port it has to connect to from the health center server.
							Patient then opens an UDP connection to the obtained doctor port. The patient sends its insurance type and obtains the cost of 
							insurance from the doctor on the UDP channel.
							
   4. Doctor.center			:Sends the insurance cost to the Patient for the insurance type specified via UDP socket.	
   
e. === Program Execution Steps ===
	1. Run the make file 
	   $>make	 
	   This generates the .o files for healthcenterserver, doctor, patient1, patient2
	   
	2. $>./healthcenterserver
	   This launches the health center server
	   
	3. Open a new putty session and hit
	   $>./doctor
	   This launches the doctors concurrently
	   
	4. Open a new putty session and hit
	   $> ./patient1
	   
	5. Open a new putty session and hit
	   $>./patient2

f. === Format of the messages ===
	Phase 1: Patient sends "authenticate username password" to health center server		
			 Health center responds with "success" or "failure" after authenticating the patient	
	
	Phase 2: Authenticated patient sends "available" to health center server 
			 The Health Center server responds with the "Index Day Time" of the available appointments
			 The patient sends "selection index" indicating the preferred appointment'
			 The Health Center server responds with "doc# port number" or "notavailable" based on whether or not the appointment is available.
			 
	Phase 3: The patient sends "insurance#" to the Doctor
			The doctor responds with "estimated cost" value to the patient.
			
g. === Idiosyncracy ===
	Not found

h. === Reused code ===
	Used code from beej socket programming tutorial: http://beej.us/guide/bgnet/
	The code used from the above link is indicated in the source code with the comment blocks:
	/* 
	 *	Code from Beej Socket Programming Tutorial: Reference# Starts here 
	 */ 
		Code block from beej
	/* 
	 *	Code from Beej Socket Programming Tutorial: Reference# ends here 
	 */ 
	 
Note:
   1.Used fork() system call in doctor.c to create concurrent processes - doctor1 and doctor2
   2.Created make file for easy program execution
   3.When the available appointments are empty and the patient requests for an appointment, the patient program exits and the health center server continues
   to handle the next request.
