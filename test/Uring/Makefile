
INCDIR=../../include

LIBS=../../src/Uring/*.o


all:
	g++ *.cpp -I $(INCDIR) $(LIBS) -o testapp -lpthread -lrt -luring


clean:
	rm -f testapp
	rm -f *.o

  
