CC = clang
CFLAGS = -Wall -Wextra -O2 -I/usr/src/minix
LDLIBS = -lsys -ltimers

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

TARGET = rsstat
OBJS = rsstat.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

rsstat.o: rsstat.c
	$(CC) $(CFLAGS) -c rsstat.c

install: $(TARGET)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	chmod 755 $(DESTDIR)$(BINDIR)/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all install uninstall clean
