all:	application

app_ui.o: app_ui.c cd_data.h
	gcc -pedantic -Wall -ansi -g -c app_ui.c

access.o: access.c cd_data.h
	gcc -pedantic -Wall -ansi -g -c access.c

application:	app_ui.o access.o
	gcc -o application -pedantic -Wall -ansi -g app_ui.o access.o -ldbm

