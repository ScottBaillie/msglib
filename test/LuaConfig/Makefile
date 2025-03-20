
INCDIR=../../include

LIBS=../../src/LuaConfig/*.o


all:
	g++ *.cpp -I $(INCDIR) -I/usr/include/lua5.4 $(LIBS) -o testapp -lpthread -lrt -llua5.4


clean:
	rm -f testapp
	rm -f *.o

  
