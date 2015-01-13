all:	application

GNOMEapp_ui.o: GNOMEapp_ui.c cd_data.h
	gcc -pedantic -Wall -ansi -g -c GNOMEapp_ui.c

cd_access.o: cd_access.c cd_data.h
	gcc -pedantic -Wall -ansi -g -c cd_access.c

application:	GNOMEapp_ui.o cd_access.o
	gcc -o application -pedantic -Wall -ansi -g GNOMEapp_ui.o cd_access.o -lgdbm `gnome-config --cflags --libs gnomeui`

