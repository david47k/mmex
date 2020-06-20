CC=gcc
WIN32CC=i686-w64-mingw32-gcc
WIN64CC=x86_64-w64-mingw32-gcc
CFLAGS = -s -Wall -Wpedantic
TARGETS = mmex mmex.exe mmex.x64.exe

default: $(TARGETS)

mmex: mmex.c
	$(CC) $(CFLAGS) $^ -o $@

mmex.exe: mmex.c
	$(WIN32CC) $(CFLAGS) $^ -o $@

mmex.x64.exe: mmex.c
	$(WIN64CC) $(CFLAGS) $^ -o $@

clean:
	rm $(TARGETS)

