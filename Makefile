CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu99 -g

SRCS    = main.c refseq.c memory.c pagetable.c tlb.c replace.c
OBJS    = $(SRCS:.c=.o)
TARGET  = memsimulator

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c memsim.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET) tlb.o replace.o 

.PHONY: all clean