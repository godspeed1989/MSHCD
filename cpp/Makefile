CC=g++
CPPFLAGS= -g -Wall
TARGET=main

all: $(TARGET)

main.o: main.cpp Detector.hpp Stage.hpp Tree.hpp Feature.hpp Rectangle.hpp \
 Point.hpp Image.hpp


$(TARGET): $(TARGET).o GetHaarCascade.cpp
#	$(CC) $(CPPFLAGS) $+ -o $@

clean:
	rm -rf $(TARGET) $(TARGET).o

