appname := cypher

srcdir := ./src
bindir := ./bin

CC := gcc
CFLAGS := -Wall -I$(srcdir)

srcs := $(shell find $(srcdir) -name "*.c")
objs := $(patsubst %.c, %.o, $(srcs))

# ================ main targets ================

all: app

app: $(appname)

# ================ output targets ================

$(appname): $(objs)
	$(CC) $(CFLAGS) $(LDFLAGS) -O2 -o $(appname) $(objs)
	chmod +x $(appname)
	mv $(appname) $(bindir)

### ================ object targets ================

depend: .depend

.depend: $(srcs)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend;

clean:
	rm -f $(objs)

include .depend
