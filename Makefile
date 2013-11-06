all:
	gcc -c ipcMessaging/ipcMessaging.c
	ar rcs ipcMessaging.a ipcMessaging.o
	rm -rf *.o
