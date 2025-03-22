
INCDIR=../../../include


all:
	mkdir -p gen
	protoc --proto_path=src --cpp_out=gen src/command.proto
	g++ -c gen/*.cc
	rm -rf $(INCDIR)/Network/ConnectionHandler/gen
	mkdir $(INCDIR)/Network/ConnectionHandler/gen
	cp gen/*.h $(INCDIR)/Network/ConnectionHandler/gen
	g++ -I $(INCDIR) -c *.cpp


clean:
	rm -f *.o
	rm -f gen/*

  
