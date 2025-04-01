
INCDIR=../../include

LIBS=../../msglib.a


all:
	g++ main.cpp -I $(INCDIR) $(LIBS) -o testapp -lpthread -lrt -luring
	g++  test1.cpp -I $(INCDIR) $(LIBS) -o test1 -lpthread -lrt -luring


clean:
	rm -f testapp
	rm -f test1

  
