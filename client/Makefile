client:main.o client.o wrap.o store_msg.o  print_fmt.o  get_msg.o admin.o
	gcc -o client main.o client.o wrap.o store_msg.o print_fmt.o  get_msg.o  admin.o -lpthread
main.o:main.c client.h
	gcc -c -g main.c
admin.o:admin.c client.h
	gcc -c -g admin.c
get_msg.o:get_msg.c client.h
	gcc -c -g get_msg.c
print_fmt.o:print_fmt.c
	gcc -c -g print_fmt.c
store_msg.o:store_msg.c client.h
	gcc -c -g store_msg.c
wrap.o:wrap.c wrap.h
	gcc -c -g wrap.c
client.o:client.c client.h
	gcc -c -g client.c
clean:
	rm -f *.o
