appname := cypher

srcdir := ./src
bindir := ./bin

CXX := gcc
CXXFLAGS := -Wall -I$(srcdir) -O2

srcs := $(shell find $(srcdir) -name "*.c")
objs := $(patsubst %.c, %.o, $(srcs))

# ================ main targets ================

all: app

app: $(appname)

# ================ output targets ================

$(appname): $(objs)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(appname) $(objs)
	mv $(appname) $(bindir)

### ================ object targets ================

depend: .depend

.depend: $(srcs)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	rm -f $(objs)

include .depend
