
txtdump: apps/txtDump.c src/CraveFile.c
	$(CC) -O2 $? -o $@ -Isrc/
