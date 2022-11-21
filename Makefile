all:
	gcc -Wall -g -c src/commom.c
	gcc -Wall -g src/equipment.c commom.o -o equipment
	gcc -Wall -g src/server.c commom.o -o server