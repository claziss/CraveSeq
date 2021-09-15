all: txtdump

parser.o: src/parser.c
	$(CC) -c -O2 $< -o $@ -Isrc/

txtdump.o: apps/txtDump.c
	$(CC) -c -O2 $< -o $@ -Isrc/

txtdump: txtdump.o parser.o
	$(CC) -O2 $^ -o $@ -Isrc/

clean:
	rm -rf txtdump *.o
