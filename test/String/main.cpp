//////////////////////////////////////////////////////////////////////////////

#include <String/Tokenizer.h>

#include <random>
#include <iostream>

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, char * argv[])
{
	int ret = 0;
	std::string s;
	Tokenizer t;
	TokenArgs tokenArgs;

	s = "  command  -x  -f filename.txt  -s 1000  -a  -l 50 -A ' a1 a2 a3 '  arg1 arg2 arg3  ";

	t.tokenize(s, tokenArgs.tokens);

	tokenArgs.singularSwitches.insert("x");
	tokenArgs.singularSwitches.insert("a");

	bool ok = tokenArgs.getArgs();

	for (auto i : tokenArgs.switches) {
		std::cout << "switchName : " << i.first << " : data : " << i.second << "\n";
	}

	for (auto i : tokenArgs.args) {
		std::cout << "args : " << i << "\n";
	}

	if (tokenArgs.hasSwitch("x")) std::cout << "hasSwitch : x\n";
	if (tokenArgs.hasSwitch("f")) std::cout << "hasSwitch : f\n";
	if (tokenArgs.hasSwitch("s")) std::cout << "hasSwitch : s\n";
	if (!tokenArgs.hasSwitch("S")) std::cout << "hasSwitch not : S\n";

	std::string param;
	ok = tokenArgs.getParam("A", param);

	std::cout << "param : " << param << " : param qr : " << t.removeQuotes(param) << "\n";

	return ret;
}

////////////////////////////////////////////////////////////////////////////// 
