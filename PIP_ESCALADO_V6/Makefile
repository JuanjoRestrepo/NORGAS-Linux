#
# Makefile for make
#
# Compiler: gcc
#

PROGS   = phoneFSMtrc
CFLAGST = -D_REENTRANT -Wall
LIBS    = pthread

all: $(PROGS)

phoneFSMtrc : phtrdsMsgLyr.o phoneTrc.o
	$(CC) $(CFLAGST) -o phoneFSMtrc phoneTrc.o phtrdsMsgLyr.o -l $(LIBS)

phtrdsMsgLyr.o : phtrdsMsgLyr.c phtrdsMsgLyr.h pMLusrConf.h
	$(CC) $(CFLAGST) -g -c phtrdsMsgLyr.c

phone.o : phone.c pMLusrConf.h
	$(CC) $(CFLAGST) -g -c phone.c

phoneTrc.o : phoneTrc.c pMLusrConf.h
	$(CC) $(CFLAGST) -g -c phoneTrc.c

clean:
	rm -f $(PROGS) *~ *.o
