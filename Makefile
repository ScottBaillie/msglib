
all:
	cd src/Uring; make
	cd src/LuaConfig; make
	cd src/String; make
	cd src/Memory; make
	cd src/Time; make
	cd src/Network/Buffer; make
	cd src/Network/IpAddress; make
	cd src/Network/ConnectionManager; make
	cd src/Network/UdpConnectionManager; make
	cd src/Network/ConnectionHandler; make
	cd src/Network/Command; make
	rm -f msglib.a
	ar rc msglib.a `find src | grep [.]o`


testall:
	cd test/Uring; make
	cd test/String; make
	cd test/Memory; make
	cd test/Time; make
	cd test/FastQueue; make
	cd test/LinkedList; make
	cd test/Network/Buffer; make
	cd test/Network/IpAddress; make
	cd test/Network/ConnectionManager; make
	cd test/Network/UdpConnectionManager; make
	cd test/Network/ConnectionHandler; make
	cd test/Network/Command; make
	cd test/LuaConfig; make

clean:
	cd src/Uring; make clean
	cd src/String; make clean
	cd src/LuaConfig; make clean
	cd src/Memory; make clean
	cd src/Time; make clean
	cd src/Network/Buffer; make clean
	cd src/Network/IpAddress; make clean
	cd src/Network/ConnectionManager; make clean
	cd src/Network/UdpConnectionManager; make clean
	cd src/Network/ConnectionHandler; make clean
	cd src/Network/Command; make clean
	cd test/Uring; make clean
	cd test/String; make clean
	cd test/LuaConfig; make clean
	cd test/Memory; make clean
	cd test/Time; make clean
	cd test/FastQueue; make clean
	cd test/LinkedList; make clean
	cd test/Network/Buffer; make clean
	cd test/Network/IpAddress; make clean
	cd test/Network/ConnectionManager; make clean
	cd test/Network/UdpConnectionManager; make clean
	cd test/Network/ConnectionHandler; make clean
	cd test/Network/Command; make clean
	rm -f msglib.a


 
