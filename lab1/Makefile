FLAGS = -Wall -g
CC = gcc
TARGET = desafio1

OBJS = desafio1.o \
	funciones.o

all: $(TARGET)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET):
	gcc -o $(TARGET) $(OBJS)

clean:
	rm -f *.o $(TARGET)
