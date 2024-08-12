PREFIX ?= /usr/local

all:
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -o SimpleMidiGuitar -ljack

clean:
	rm -f *.o *.gch
	rm -f SimpleMidiGuitar
	
install:
	install MidiGuitarOnLinux $(DESTDIR)/$(PREFIX)/bin/SimpleMidiGuitar

