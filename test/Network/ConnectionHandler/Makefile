
INCDIR=../../../include

LIBS=../../../msglib.a


all:
	mkdir -p gen
	protoc --proto_path=src --cpp_out=gen src/test1.proto
	g++  -c gen/*.cc
	g++  *.cpp -I $(INCDIR) $(LIBS) *.o -o testapp -lpthread -lrt -lprotobuf


clean:
	rm -f testapp
	rm -f gen/*
	rm -f *.o

  
