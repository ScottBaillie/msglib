//////////////////////////////////////////////////////////////////////////////

#include <map>
#include <string>
#include <vector>

#include <cstddef>

#include <stdint.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

//////////////////////////////////////////////////////////////////////////////

class LuaValue
{
public:
	enum Type
	{
		BOOL,
		NUMBER,
		INT,
		STRING,
	};

	LuaValue() {}

public:
	enum Type	m_type = BOOL;
	bool		m_bool = false;
	double		m_number = 0.0;
	int		m_int = 0;
	std::string	m_string;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::vector<LuaValue> LuaValueArray;
typedef std::map<std::string,LuaValueArray> LuaValueMap;

//////////////////////////////////////////////////////////////////////////////

class LuaConfig
{
public:
	bool		load(const std::string & filename, std::string & error);

	LuaValue::Type	getType(const std::string & name);
	LuaValue::Type	getType(const std::string & name, const unsigned arrayindex);
	bool		isArray(const std::string & name);
	size_t		getArraySize(const std::string & name);

	bool		getBool(const std::string & name);
	double		getDouble(const std::string & name);
	int		getInt(const std::string & name);
	std::string	getString(const std::string & name);

	bool		getBool(const std::string & name, const unsigned arrayindex);
	double		getDouble(const std::string & name, const unsigned arrayindex);
	int		getInt(const std::string & name, const unsigned arrayindex);
	std::string	getString(const std::string & name, const unsigned arrayindex);

	std::vector<bool>		getBoolArray(const std::string & name);
	std::vector<double>		getDoubleArray(const std::string & name);
	std::vector<int>		getIntArray(const std::string & name);
	std::vector<std::string>	getStringArray(const std::string & name);
	LuaValueArray			getValueArray(const std::string & name);

	std::vector<std::string>	getNames() const;

private:
	LuaValueMap	m_valuemap;
};

//////////////////////////////////////////////////////////////////////////////

 
