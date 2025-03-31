
INCDIR=../../../include

LIBS=../../../msglib.a


all:
	g++  main.cpp -I $(INCDIR) $(LIBS) -o testapp -lpthread -lrt
	g++  test1.cpp -I $(INCDIR) $(LIBS) -o test1 -lpthread -lrt


clean:
	rm -f testapp
	rm -f test1


