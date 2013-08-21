CC = gcc
CFLAGS = -Wall `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv` -lm

TARGET = mshcd

all: $(TARGET)

$(TARGET).o: $(TARGET).c $(TARGET).h
	$(CC) -c $< $(CFLAGS)

GetHaarCascade.o: GetHaarCascade.c
	$(CC) $(CFLAGS) -c $<

main.o: main.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(TARGET).o GetHaarCascade.o main.o
	$(CC) $+ $(LIBS) -o $@

clean:
	rm -rf $(TARGET) *.o

