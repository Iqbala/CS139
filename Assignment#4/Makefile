CC = gcc
CFLAGS = -Wall
LDFLAGS = 
OBJFILES = bestfitmalloc.o driverp4.o
TARGET = bestfitmalloc

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean: 
	rm -f $(OBJFILES) $(TARGET) *~ 
