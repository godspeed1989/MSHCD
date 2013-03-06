CC=g++
TARGET1=raw
TARGET2=draw
TARGET3=show
LIBS=`pkg-config opencv --libs`
CPPFLAGS=`pkg-config opencv --cflags` -g -Wall


all: $(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET1): $(TARGET1).cpp
	$(CC) $(CPPFLAGS) -o $@ $< $(LIBS)

$(TARGET2): $(TARGET2).cpp
	$(CC) $(CPPFLAGS) -o $@ $< $(LIBS)

$(TARGET3): $(TARGET3).cpp ../GetHaarCascade.cpp
	$(CC) $(CPPFLAGS) -o $@ $+ $(LIBS)

clean:
	rm -rf $(TARGET1) $(TARGET2) $(TARGET3)

