CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -O2
LDLIBS = -lm
OBJS   = main.o artemis.o validaciones.o

all: artemis

artemis: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

# Cada .o se recompila si cambia su .c o algun .h
main.o: main.c artemis.h validaciones.h
artemis.o: artemis.c artemis.h validaciones.h
validaciones.o: validaciones.c validaciones.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) artemis

.PHONY: all clean
