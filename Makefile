CC = g++
CFLAGS = -Wall -Wextra -g -std=c++23
# Linker flags remain largely the same, but we use g++ to link
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

TARGET = main

$(TARGET): main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o $(LDFLAGS)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -f $(TARGET) *.o