
INCDIR=../../../include

LIBS=../../../msglib.a


all:
	mkdir -p gen
	protoc --proto_path=src --cpp_out=gen src/test2.proto
	g++  -c gen/*.cc
	g++  main.cpp -I $(INCDIR) $(LIBS) *.o -o testapp -lpthread -lrt -lprotobuf
	g++  test1.cpp -I $(INCDIR) $(LIBS) -o test1 -lpthread -lrt
	g++  test2.cpp -I $(INCDIR) $(LIBS) -o test2 -lpthread -lrt -lprotobuf


clean:
	rm -f testapp
	rm -f test1
	rm -f test2
	rm -f gen/*
	rm -f *.o


