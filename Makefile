CFLAGS = `pkg-config --cflags --libs libnotify`

all:	rotomatic

rotomatic:	rotomatic.o findpowermate.o
	$(CC) $(CFLAGS) findpowermate.o rotomatic.o -o rotomatic

clean:
	rm -f *.o *~ rotomatic

%.o:    %.c
	$(CC) $(CFLAGS) -O2 -Wall -llibboost_program_options.so.1.53.0 -c $< -o $@
