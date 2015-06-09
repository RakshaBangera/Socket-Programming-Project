#
#Makefile for EE450 Socket Programming Project
#

#Create object files for healthcenterserver.c doctor.c patient1.c patient2.
all: healthcenterserver doctor patient1 patient2

#Create object file for healthcenterserver.c
healthcenterserver:healthcenterserver.c
	gcc -o healthcenterserver healthcenterserver.c -lsocket -lnsl -lresolv
	
#Create object file for doctor.c
doctor:doctor.c
	gcc -o doctor doctor.c -lsocket -lnsl -lresolv

#Create object file for patient1.c
patient1:patient1.c
	gcc -o patient1 patient1.c -lsocket -lnsl -lresolv

#Create object file for patient2.c	
patient2:patient2.c
	gcc -o patient2 patient2.c -lsocket -lnsl -lresolv
	