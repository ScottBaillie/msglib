
INCDIR=../../../include

LIBS1=../../../src/Network/IpAddress/*.o
LIBS2=../../../src/Network/ConnectionManager/*.o
LIBS3=../../../src/Network/Buffer/*.o
LIBS4=../../../src/Network/ConnectionHandler/*.o
LIBS=$(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)


all:
	protoc --proto_path=src --cpp_out=gen src/test1.proto
	g++  -c gen/*.cc
	g++  *.cpp -I $(INCDIR) $(LIBS) *.o -o testapp -lpthread -lrt -lprotobuf


clean:
	rm -f *.o
	rm -f testapp
	rm -f gen/*

  
