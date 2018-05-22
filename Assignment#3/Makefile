CC = gcc
SRC1 = demo.c
SRC2 = timer.c
SRC3 = functions.c
TRGT1 = demo
TRGT2 = timer
TRGT3 = functions
CFLAGS = -lm

all: T1 T2 T3 

T1:
	$(CC)  $(SRC1) -o $(TRGT1)

T2:
	$(CC)  $(SRC2) -o $(TRGT2)

T3:
	$(CC)  $(SRC3) $(CFLAGS) -o $(TRGT3)

clean:
	@rm $(TRGT1) $(TRGT2) $(TRGT3)
