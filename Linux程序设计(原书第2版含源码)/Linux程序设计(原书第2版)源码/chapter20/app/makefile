all:	cdhtml


.c.o:
	gcc -g -c $?

html.o:	html.c html.h
	gcc -g -c html.c

app_html.o: app_html.c cd_data.h html.h
	gcc -g -c app_html.c

access.o: access.c cd_data.h
	gcc -I/usr/include/db1 -g -c access.c

cdhtml:	app_html.o access.o html.o
	gcc -o cdhtml -pedantic -g app_html.o access.o \
html.o -ldb

install: cdhtml
	-echo Depending on your setup, you need to do something like...
	-echo cp cdhtml /usr/local/apache/cgi-bin/cddb
	-echo cp cdc_data.dir cdc_data.pag /usr/local/apache/cgi-bin/cddb
	-echo cp cdt_data.dir cdt_data.pag /usr/local/apache/cgi-bin/cddb








