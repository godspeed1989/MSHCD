CC=g++
CPPFLAGS= -g -Wall
TARGET=main

all: $(TARGET)

main.o: main.cpp Detector.hpp Stage.hpp Tree.hpp Feature.hpp Rect.hpp \
 Point.hpp Image.hpp


$(TARGET): $(TARGET).o
#	$(CC) $(CPPFLAGS) $< -o $@

clean:
	rm -rf $(TARGET) $(TARGET).o

