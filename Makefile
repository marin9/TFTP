CC=gcc
FLAGS=-Wall -Wextra -c
OBJ=net.o file.o tftp.o

all: stftp ctftp


stftp : stftp.o $(OBJ)
	$(CC) stftp.o $(OBJ) -o stftp
	
ctftp : ctftp.o $(OBJ)
	$(CC) ctftp.o $(OBJ) -o ctftp


ctftp.o : ctftp.c
	$(CC) $(FLAGS) ctftp.c
	
stftp.o : stftp.c
	$(CC) $(FLAGS) stftp.c	
	
tftp.o : tftp.c
	$(CC) $(FLAGS) tftp.c
	
net.o : net.c
	$(CC) $(FLAGS) net.c
	
file.o : file.c
	$(CC) $(FLAGS) file.c


clean:
	rm *.o
	rm stftp
	rm ctftp
