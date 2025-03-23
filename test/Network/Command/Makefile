
INCDIR=../../../include

LIBS=../../../msglib.a


all:
	g++  main.cpp -I $(INCDIR) $(LIBS) -o testapp -lpthread -lrt -lprotobuf
	g++  cmdclient.cpp -I $(INCDIR) $(LIBS) -o cmdclient -lpthread -lrt -lprotobuf


clean:
	rm -f testapp
	rm -f cmdclient

  
