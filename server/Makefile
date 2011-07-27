server: main.o server.o store_msg.o get_msg.o  wrap.o admin.o
	gcc -o server server.o store_msg.o get_msg.o main.o wrap.o admin.o -lpthread
main.o:main.c server.h
	gcc -c -g main.c
admin.o:admin.c server.h wrap.h
	gcc -c -g admin.c
wrap.o:wrap.c wrap.h
	gcc -c -g wrap.c
server.o:server.c server.h
	gcc -c -g server.c
store_msg.o:store_msg.c server.h
	gcc -c -g store_msg.c
get_msg.o:get_msg.c server.h
	gcc -c -g get_msg.c
clean:
	rm -f *.o
