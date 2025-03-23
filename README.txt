A C++ library for development of real time Apps and for network Apps.


Build Requirements
------------------
The following development packages are required to build the code in this project :

lua-devel
protobuf-devel
liburing-devel


Build requirements relating to LUA
----------------------------------
There are two Makefiles that may need to be customised. The makefiles are : msglib/src/LuaConfig/Makefile , and , msglib/test/LuaConfig/Makefile.
The makefile variables LUA_INCLUDE and LUA_LIB may need to be customised, the current settings are shown below :

LUA_INCLUDE=-I/usr/include/lua5.4
LUA_LIB=lua5.4

An alternative setting could be as follows :

LUA_INCLUDE=-I/usr/include/lua
LUA_LIB=lua


Build Instructions
------------------

make clean
make all
make testall
