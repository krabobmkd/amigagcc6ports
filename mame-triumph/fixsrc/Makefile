CC     = gcc
LD	   = gcc

OUT    = fixsrc

CFLAGS = -g -I/gg/include -I/include -I/myinclude -m68000 -Wall -noixemul

LDFLAGS= -g

LIBS   = -noixemul

OBJS   = fixsrc.o scandir.o

all: $(OUT)

$(OUT): $(OBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

%.o: %.c scandir.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm	*.o
	-rm	$(OUT)
