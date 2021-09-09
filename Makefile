all: txtdump

txtdump: apps/txtDump.c src/parser.c
	$(CC) -O2 $? -o $@ -Isrc/

clean:
	rm txtdump
