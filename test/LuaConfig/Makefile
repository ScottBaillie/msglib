
INCDIR=../../include

LIBS=../../src/LuaConfig/*.o

LUA_INCLUDE=-I/usr/include/lua5.4
LUA_LIB=lua5.4

all:
	g++ *.cpp -I $(INCDIR) $(LUA_INCLUDE) $(LIBS) -o testapp -lpthread -lrt -l$(LUA_LIB)


clean:
	rm -f testapp
	rm -f *.o

  
