all:	server client

CC=cc
CFLAGS=-I/usr/include/db1 -pedantic -Wall

# For debugging un-comment the next line
# DFLAGS=-DDEBUG_TRACE=1 -g


# Include for systems with dbm, but only as part of the BSD licensed version,
# and not in the standard locations. This is the default...
DBM_INC_PATH=/usr/include/db1
DBM_LIB_PATH=/usr/lib
DBM_LIB_FILE=db

# For systems with dbm in the standard places, comment out the previous
# definitions, and uncomment these
# 
#DBM_INC_PATH=/usr/include
#DBM_LIB_PATH=/usr/lib
#DBM_LIB_FILE=ndbm

# For systems where the gdbm libraries have been fetched and installed
# separately in the default locations under /usr/local
# comment out the previous definitions, and uncomment these
#DBM_INC_PATH=/usr/local/include
#DBM_LIB_PATH=/usr/local/lib
#DBM_LIB_FILE=gdbm


.c.o:
	$(CC) $(CFLAGS) -I$(DBM_INC_PATH) $(DFLAGS) -c $<

app_ui.o: app_ui.c cd_data.h
cd_dbm.o: cd_dbm.c cd_data.h
client_f.o: clientif.c cd_data.h cliserv.h
pipe_imp.o: pipe_imp.c cd_data.h cliserv.h
server.o: server.c cd_data.h cliserv.h


client: app_ui.o clientif.o pipe_imp.o
	$(CC) -o client  $(DFLAGS) app_ui.o clientif.o pipe_imp.o

server:	server.o cd_dbm.o pipe_imp.o
	$(CC) -o server -L$(DBM_LIB_PATH) $(DFLAGS) server.o cd_dbm.o pipe_imp.o -l$(DBM_LIB_FILE)

clean:
	rm -f server client_app *.o *~

