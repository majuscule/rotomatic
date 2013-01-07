CFLAGS = `pkg-config --cflags --libs libnotify`

all:	rotomatic pulseled

rotomatic:	rotomatic.o findpowermate.o
	$(CC) $(CFLAGS) findpowermate.o rotomatic.o -o rotomatic

pulseled:	pulseled.o findpowermate.o
	$(CC) findpowermate.o pulseled.o -o pulseled

clean:
	rm -f *.o *~ rotomatic pulseled

%.o:    %.c
	$(CC) $(CFLAGS) -O2 -Wall -c $< -o $@
