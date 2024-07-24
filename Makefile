PREFIX ?= /usr/local

all:
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -o MidiGuitarOnLinux -ljack

clean:
	rm -f *.o *.gch
	rm -f MidiGuitarOnLinux
	
install:
	install MidiGuitarOnLinux $(DESTDIR)/$(PREFIX)/bin/MidiGuitarOnLinux

