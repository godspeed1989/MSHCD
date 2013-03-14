CC=g++
CPPFLAGS= -g -Wall
TARGET=mshcd

all: $(TARGET)

main.o: main.cpp Detector.hpp Stage.hpp Tree.hpp Feature.hpp Rectangle.hpp \
 Point.hpp Image.hpp CannyPruner.hpp


$(TARGET): main.o GetHaarCascade.cpp
	$(CC) $(CPPFLAGS) $+ -o $@


clean:
	rm -rf $(TARGET) $(TARGET).o main.o

