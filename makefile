TARGET=pokemonClient pokemonServer
normal: $(TARGET)
pokemonClient: pokemonclient.o llist.o threadpool.o clientHelpers.o
	gcc -o pokemonClient pokemonclient.o llist.o threadpool.o clientHelpers.o -pthread
pokemonServer: pokemonserver.o llist.o threadpool.o serverHelpers.o
	gcc -o pokemonServer pokemonserver.o llist.o threadpool.o serverHelpers.o -pthread
pokemonserver.o: serverHelpers.h
	gcc -c pokemonserver.c
pokemonclient.o: clientHelpers.h
	gcc -c pokemonclient.c
serverHelpers.o: serverHelpers.h llist.h threadpool.h 
	gcc -c serverHelpers.c
clientHelpers.o: clientHelpers.h llist.h threadpool.h
	gcc -c clientHelpers.c
llist.o: llist.h
	gcc -c llist.c
threadpool.o: threadpool.h
	gcc -c threadpool.c
clean:
	rm -f *.o
removeAll:
	rm -f *.o
	$(RM) $(TARGET) 