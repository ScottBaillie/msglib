//////////////////////////////////////////////////////////////////////////////

#include <LuaConfig/LuaConfig.h>

#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <random>
#include <vector>
#include <set>

#include <cstdlib>

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

		str = cfg.getString("tt.MyTable1.name");

		std::cout << "tt.MyTable1.name=" << str << "\n";

		std::vector<std::string> stra = cfg.getStringArray("tt.strlist");

		for (auto s : stra) {
			std::cout << "strlist=" << s << "\n";
		}
	}
	catch (const std::exception & e) {
		std::cout << "Exception : " << e.what() << "\n";
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////// 
