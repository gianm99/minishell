CC=gcc
CFLAGS=-c -g -Wall -std=c99
LDFLAGS=-lreadline

SOURCES= my_shell.c nivel7.c nivel6.c nivel5.c nivel4.c nivel3.c nivel2.c nivel1.c
LIBRARIES= #.o
INCLUDES= my_shell.h nivel7.h nivel6.h nivel5.h nivel4.h nivel3.h nivel2.h nivel1.h 
PROGRAMS=my_shell nivel7 nivel6 nivel5 nivel4 nivel3 nivel2 nivel1
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#   $(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

my_shell: my_shell.o $(LIBRARIES) $(INCLUDES)
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

nivel7: nivel7.o $(LIBRARIES) $(INCLUDES) 	
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

nivel6: nivel6.o $(LIBRARIES) $(INCLUDES) 	
	$(CC) $@.o -o $@ $(LIBRARIES)

nivel5: nivel5.o $(LIBRARIES) $(INCLUDES) 	
	$(CC) $@.o -o $@ $(LIBRARIES)

nivel4: nivel4.o $(LIBRARIES) $(INCLUDES) 	
	$(CC) $@.o -o $@ $(LIBRARIES)

nivel3: nivel3.o $(LIBRARIES) $(INCLUDES) 	
	$(CC) $@.o -o $@ $(LIBRARIES)

nivel2: nivel2.o $(LIBRARIES) $(INCLUDES) 	
	$(CC) $@.o -o $@ $(LIBRARIES)

nivel1: nivel1.o $(LIBRARIES) $(INCLUDES) 	
	$(CC) $@.o -o $@ $(LIBRARIES)

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<
    
.PHONY: clean
clean:
	rm -rf *.o *~ *.tmp $(PROGRAMS)
