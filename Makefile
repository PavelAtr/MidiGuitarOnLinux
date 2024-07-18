PREFIX ?= /usr/local

all:
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -o MidiGuitarOnLinux -ljack -lm

clean:
	rm -f *.o *.gch
	rm -f MidiGuitarOnLinux
	
install:
	install MidiGuitarOnLinux $(PREFIX)/bin/MidiGuitarOnLinux

