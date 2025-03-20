
INCDIR=../../../include

LIBS1=../../../src/Network/IpAddress/*.o
LIBS2=../../../src/Network/ConnectionManager/*.o
LIBS3=../../../src/Network/Buffer/*.o
LIBS4=../../../src/Network/ConnectionHandler/*.o
LIBS5=../../../src/Network/Command/*.o
LIBS6=../../../src/String/*.o
LIBS=$(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4) $(LIBS5) $(LIBS6)



all:
	g++  main.cpp -I $(INCDIR) $(LIBS) -o testapp -lpthread -lrt -lprotobuf
	g++  cmdclient.cpp -I $(INCDIR) $(LIBS) -o cmdclient -lpthread -lrt -lprotobuf


clean:
	rm -f testapp
	rm -f cmdclient

  
