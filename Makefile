


CC= gcc
CFLAGS= -g -Wall
OBJS= main.o
TARGET= app.out

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)


main.o: main.c



clean:
	rm -f *.o
	rm -f $(TARGET)
