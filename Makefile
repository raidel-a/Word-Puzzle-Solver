CC=gcc
CFLAGS=-Wall -pthread
TARGET=wordpuzzle

$(TARGET): wordpuzzle.c
	$(CC) $(CFLAGS) -o $(TARGET) wordpuzzle.c

clean:
	rm -f $(TARGET) 