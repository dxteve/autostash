CC = gcc
CFLAGS = -pthread -Wall -Wextra
TARGET = autostash
OBJS = main.o ui.o scheduler.o copy_engine.o utilities.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c config.h scheduler.h ui.h
	$(CC) $(CFLAGS) -c main.c

ui.o: ui.c config.h ui.h
	$(CC) $(CFLAGS) -c ui.c

scheduler.o: scheduler.c config.h scheduler.h ui.h copy_engine.h utilities.h
	$(CC) $(CFLAGS) -c scheduler.c

copy_engine.o: copy_engine.c config.h copy_engine.h
	$(CC) $(CFLAGS) -c copy_engine.c

utilities.o: utilities.c config.h utilities.h
	$(CC) $(CFLAGS) -c utilities.c

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean