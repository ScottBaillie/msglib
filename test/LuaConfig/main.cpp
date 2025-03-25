//////////////////////////////////////////////////////////////////////////////

#include <LuaConfig/LuaConfig.h>

using namespace msglib;

#include <iostream>

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	int ret;

	try {
		LuaConfig cfg;
		std::string error;
		std::vector<std::string> names;

		bool ok = cfg.load(argv[1], error);
		if (!ok) {
			std::cout << "Error from load\n";
			return 0;
		}

		names = cfg.getNames();

		for (auto n : names) {
			std::cout << "name=" << n << "\n";
		}

		std::string str;

		str = cfg.getString("table1.tt.MyTable1.MyTable1a.inner_name");

		std::cout << "table1.tt.MyTable1.MyTable1a.inner_name=" << str << "\n";

		std::vector<std::string> stra = cfg.getStringArray("table2.tt.strlist");

		for (auto s : stra) {
			std::cout << "table2.tt.strlist=" << s << "\n";
		}
	}
	catch (const std::exception & e) {
		std::cout << "Exception : " << e.what() << "\n";
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 
