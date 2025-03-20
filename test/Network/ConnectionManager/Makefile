
INCDIR=../../../include

LIBS1=../../../src/Network/IpAddress/*.o
LIBS2=../../../src/Network/ConnectionManager/*.o
LIBS3=../../../src/Network/Buffer/*.o
LIBS=$(LIBS1) $(LIBS2) $(LIBS3)

all:
	g++  *.cpp -I $(INCDIR) $(LIBS) -o testapp -lpthread -lrt


clean:
	rm -f testapp
	rm -f *.o


