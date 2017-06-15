FLAGS=-Wall -Wextra -c

all: stftp ctftp


stftp : stftp.o net.o file.o tftp.o
	cc stftp.o net.o file.o tftp.o -o stftp
	
ctftp : ctftp.o net.o file.o tftp.o
	cc ctftp.o net.o file.o tftp.o -o ctftp


ctftp.o : ctftp.c
	cc $(FLAGS) ctftp.c
	
stftp.o : stftp.c
	cc $(FLAGS) stftp.c	
	
tftp.o : tftp.c
	cc $(FLAGS) tftp.c
	
net.o : net.c
	cc $(FLAGS) net.c
	
file.o : file.c
	cc $(FLAGS) file.c

clean:
	rm *.o
	rm stftp
	rm ctftp
