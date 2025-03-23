////////////////////////////////////////////////////////////////

#ifndef MSGLIB_TOKENIZER_H
#define MSGLIB_TOKENIZER_H

////////////////////////////////////////////////////////////////

#include <vector>
#include <map>
#include <set>
#include <string>

////////////////////////////////////////////////////////////////

namespace msglib {

////////////////////////////////////////////////////////////////

class TokenArgs
{
public:
	bool	getParam(const std::string & switchName, std::string & switchParam)
	{
		if (switches.find(switchName) == switches.end()) return false;
		switchParam = switches[switchName];
		return true;
	}

	bool	hasSwitch(const std::string & switchName)
	{
		if (switches.find(switchName) == switches.end()) return false;
		return true;
	}

	bool	getArgs();

	bool	checkSwitches(const std::set<std::string> & allowedSwitches); // returns false if unknown switch exists.

public:
	std::vector<std::string> tokens;		// Input : Result from tokenize().
	std::set<std::string> singularSwitches;		// Input : A list of singular switches ( i.e. -f instead of -f <file> ) (eg. singularSwitches.insert("f");).

	std::map<std::string,std::string> switches;	// Output : switch name is key (eg. key="f") , data is switch parameter (eg. <file> ).
	std::vector<std::string> args;			// Output : tokens that were not switches or switch parameters.
};

////////////////////////////////////////////////////////////////

class Tokenizer
{
public:
	Tokenizer()
	{
		m_delim.push_back(' ');
		m_delim.push_back('\t');
	}

	Tokenizer(const std::vector<char> & delim) : m_delim(delim) {}

	// qindex map key is start index and map data is end index.
	// The start index points to the start quote char and the end index points to the end quote char.
	static bool findQuotes(const std::string & s, std::map<unsigned,unsigned> & qindex);

	static std::string removeQuotes(const std::string & s);

	bool tokenize(const std::string & s, std::vector<std::string> & tokens);

	static bool getTokens(int argc, char* argv[], std::vector<std::string> & tokens);

private:
	std::vector<char>	m_delim;
};

////////////////////////////////////////////////////////////////

}

#endif
