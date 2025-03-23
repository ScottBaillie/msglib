//////////////////////////////////////////////////////////////////////////////

#include <String/Tokenizer.h>

using namespace msglib;

#include <iostream>

//////////////////////////////////////////////////////////////////////////////

bool
isDelim(const std::vector<char> & delim, const char & c)
{
	for (auto i : delim) if (c==i) return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////////

bool
Tokenizer::findQuotes(const std::string & s, std::map<unsigned,unsigned> & qindex)
{
	unsigned startIndex = 0;
	bool squote = false;
	bool dquote = false;

	for (unsigned u0=0; u0<s.size(); u0++) {
		const char & c = s[u0];

		if (dquote) {
			if (c == '"') {
				dquote = false;
				qindex[startIndex] = u0;
				continue;
			}
		}

		if (squote) {
			if (c == '\'') {
				squote = false;
				qindex[startIndex] = u0;
				continue;
			}
		}

		if (c == '"') {
			dquote = true;
			startIndex = u0;
		}

		if (c == '\'') {
			squote = true;
			startIndex = u0;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////

std::string
Tokenizer::removeQuotes(const std::string & s)
{
	size_t sz = s.size();
	if (sz<2) return s;

	if ((s[0] == '"') || (s[0] == '\'')) {
		if ((s[sz-1] == '"') || (s[sz-1] == '\'')) {
			std::string ret = s.c_str()+1;
			ret.erase(sz-2, 1);
			return ret;
		} else {
			return s;
		}
	}

	return s;
}

//////////////////////////////////////////////////////////////////////////////

bool
Tokenizer::tokenize(const std::string & s, std::vector<std::string> & tokens)
{
	tokens.clear();

	if (s.size()==0) return true;

	std::map<unsigned,unsigned> qindex;

	bool ok = findQuotes(s, qindex);
	if (!ok) return false;

	bool token = false;
	bool quote = false;
	unsigned endIndex = 0;

	for (unsigned u0=0; u0<s.size(); u0++) {
		const char & c = s[u0];
		if (quote) {
			if (u0 == endIndex) {
				quote = false;
				std::string & str = tokens.back();
				str.append(1, c);
				continue;
			}
			if (!token) {
				token = true;
				tokens.push_back("");
				std::string & str = tokens.back();
				str.append(1, c);
			} else {
				std::string & str = tokens.back();
				str.append(1, c);
			}
			continue;
		}
		if (qindex.find(u0) != qindex.end()) {
			quote = true;
			endIndex = qindex[u0];
			if (!token) {
				token = true;
				tokens.push_back("");
				std::string & str = tokens.back();
				str.append(1, c);
			} else {
				std::string & str = tokens.back();
				str.append(1, c);
			}
			continue;
		}
		if (isDelim(m_delim,c)) {
			if (token) {
				token = false;
			}
		} else {
			if (!token) {
				token = true;
				tokens.push_back("");
				std::string & str = tokens.back();
				str.append(1, c);
			} else {
				std::string & str = tokens.back();
				str.append(1, c);
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
Tokenizer::getTokens(int argc, char* argv[], std::vector<std::string> & tokens)
{
	tokens.clear();

	std::string::size_type sz;
	std::string s;

	for (int i=0; i<argc; i++) {

		s = argv[i];

		sz = s.find_first_of(" \t");

		if (sz == std::string::npos) {
			tokens.push_back(s);
		} else {
			sz = s.find_first_of("\"");
			if (sz == std::string::npos) {
				tokens.push_back("\"" + s + "\"");
			} else {
				tokens.push_back("'" + s + "'");
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
TokenArgs::getArgs()
{
	std::string switchName;

	for (auto i = tokens.begin(); i != tokens.end(); i++) {
		std::string & s = *i;
		if (s.size() == 0) continue;
		if (s[0] == '-') {
			if (s.size() == 1) {
				args.push_back(s);
				continue;
			}
			switchName = s.c_str()+1;

			if (singularSwitches.find(switchName) != singularSwitches.end()) {
				switches[switchName] = "";
				continue;
			}

			i++;
			if (i==tokens.end()) return false;
			std::string & s1 = *i;
			switches[switchName] = s1;
		} else {
			args.push_back(s);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
TokenArgs::checkSwitches(const std::set<std::string> & allowedSwitches)
{
	for (auto i : switches) {
		if (allowedSwitches.find(i.first) == allowedSwitches.end()) return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
