all:
	$(CC) *.c -o MidiGuitarOnLinux -ljack -lm

clean:
	rm -f *.o *.gch
	rm -f MidiGuitarOnLinux

