CC = gcc
CPPFLAGS = -Wall `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv` -lm

TARGET = mshcd

all: $(TARGET)

$(TARGET).o: $(TARGET).cpp $(TARGET).hpp
	$(CC) -c $<

GetHaarCascade.o: GetHaarCascade.cpp
	$(CC) -c $<

main.o: main.cpp
	$(CC) -c $<

$(TARGET): $(TARGET).o GetHaarCascade.o main.o
	$(CC) $+ $(LIBS) $(CPPFLAGS) -o $@

clean:
	rm -rf $(TARGET) *.o

