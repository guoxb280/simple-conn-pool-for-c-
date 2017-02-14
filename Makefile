SRC = connpool.cc
OBJ = $(SRC:.cc=.o)
OUT = connpool.a
INCLUDES = #-I../include
CCFLAGS = -g -O2 -fpic -pedantic
CCC = g++

.SUFFIXES: .cc

all: static

static: $(OUT)

.c.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) $(EXTRACCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs connpool.a $(OBJ)

clean:
	rm -f $(OBJ) $(OUT)

install:
	echo "do nothing"
