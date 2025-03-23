//////////////////////////////////////////////////////////////////////////////

#include <LuaConfig/LuaConfig.h>

using namespace msglib;

#include <iostream>

//////////////////////////////////////////////////////////////////////////////

bool get_table(lua_State *L, const std::string name, const int i, LuaValueMap & m);

//////////////////////////////////////////////////////////////////////////////

bool get_array(lua_State *L, const std::string name, const int i, LuaValueMap & m)
{
	int top = lua_gettop(L);
	if (top==0) return true;

	int arrayindex = 1;
	int type = lua_rawgeti(L, i, arrayindex);
	if (type!=LUA_TNIL) {

		while (true) {

			if (arrayindex!=1) type = lua_rawgeti(L, i, arrayindex);  // lua_rawgeti lua_geti
			if (type == LUA_TNIL) break;

			switch (type) {
				case LUA_TNUMBER:
					{
					LuaValueArray & vl = m[name];
					vl.push_back(LuaValue());
					LuaValue & v = vl.back();
					v.m_type = LuaValue::NUMBER;
					v.m_number = lua_tonumber(L, top+1);
					}
					break;
				case LUA_TBOOLEAN:
					{
					LuaValueArray & vl = m[name];
					vl.push_back(LuaValue());
					LuaValue & v = vl.back();
					v.m_type = LuaValue::BOOL;
					v.m_bool = lua_toboolean(L, top+1);
					}
					break;
				case LUA_TSTRING:
					{
					LuaValueArray & vl = m[name];
					vl.push_back(LuaValue());
					LuaValue & v = vl.back();
					v.m_type = LuaValue::STRING;
					v.m_string = lua_tostring(L, top+1);
					}
					break;
				case LUA_TTABLE:
					get_table(L, name, top+1, m);
					break;
				default:
					std::cout << "Unknown type\n";
					break;
			}
			lua_pop(L,1);
			arrayindex++;
		}

		lua_pop(L,1);

		return true;
	}

	lua_pop(L,1);

	return false;
}

//////////////////////////////////////////////////////////////////////////////

bool get_table(lua_State *L, const std::string name, const int i, LuaValueMap & m)
{
	int top = lua_gettop(L);
	if (top==0) return true;
	int status;
	size_t msize = m.size();

	bool isarray = get_array(L, name, i, m);
	if (isarray) return true;

	lua_pushnil(L); 

	int nameIndex = top+1;
	int valueIndex = top+2;

	std::string fldname;
	std::string n;

	while (true) {

		status = lua_next(L, i);
		if (status==0) break;
		if (lua_isnil(L, nameIndex)) {lua_pop(L,1); continue;}
		if (lua_isnil(L, valueIndex)) {lua_pop(L,1); continue;}

		fldname = lua_tostring(L,nameIndex);
		if (name.size()==0) n = fldname; else n = name + "." + fldname;

		if (lua_istable(L, valueIndex)) get_table(L, n, valueIndex, m);

		if (lua_isboolean(L, valueIndex)) {
			LuaValueArray & vl = m[n];
			vl.push_back(LuaValue());
			LuaValue & v = vl.back();
			v.m_type = LuaValue::BOOL;
			v.m_bool = lua_toboolean(L, valueIndex);
		}

		if (lua_isinteger(L, valueIndex)) {
			LuaValueArray & vl = m[n];
			vl.push_back(LuaValue());
			LuaValue & v = vl.back();
			v.m_type = LuaValue::INT;
			v.m_int = lua_tointeger(L, valueIndex);
		}

		if (lua_isnumber(L, valueIndex)) {
			LuaValueArray & vl = m[n];
			vl.push_back(LuaValue());
			LuaValue & v = vl.back();
			v.m_type = LuaValue::NUMBER;
			v.m_number = lua_tonumber(L, valueIndex);
		}

		if (lua_isstring(L, valueIndex)) {
			LuaValueArray & vl = m[n];
			vl.push_back(LuaValue());
			LuaValue & v = vl.back();
			v.m_type = LuaValue::STRING;
			v.m_string = lua_tostring(L, valueIndex);
		}

		lua_pop(L,1);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
LuaConfig::load(const std::string & filename, std::string & error)
{
	lua_State *L = luaL_newstate();
	int status;

	status = luaL_loadfile(L, filename.c_str());

	if (status != LUA_OK) {
		error = "Error from luaL_loadfile";
		return false;
	}

	status = lua_pcall(L, 0, LUA_MULTRET, 0);

	if (status != LUA_OK) {
		error = "Error from lua_pcall";
		return false;
	}

	bool ok = get_table(L, "", 1, m_valuemap);

	return ok;
}

//////////////////////////////////////////////////////////////////////////////

LuaValue::Type
LuaConfig::getType(const std::string & name)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getType : Name not found");
	if (vi->second.size()!=1) throw std::runtime_error("LuaConfig::getType : Value is an Array");
	return vi->second[0].m_type;
}

//////////////////////////////////////////////////////////////////////////////

LuaValue::Type
LuaConfig::getType(const std::string & name, const unsigned arrayindex)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getType : Name not found");
	if (vi->second.size()==0) throw std::runtime_error("LuaConfig::getType : No value present");
	if (arrayindex >= vi->second.size()) throw std::runtime_error("LuaConfig::getType : Index out of range");
	return vi->second[arrayindex].m_type;
}

//////////////////////////////////////////////////////////////////////////////

bool
LuaConfig::isArray(const std::string & name)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::isArray : Name not found");
	if (vi->second.size()!=1) return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////////

size_t
LuaConfig::getArraySize(const std::string & name)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getArraySize : Name not found");
	return vi->second.size();
}

//////////////////////////////////////////////////////////////////////////////

bool
LuaConfig::getBool(const std::string & name)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getBool : Name not found");
	if (vi->second.size()!=1) throw std::runtime_error("LuaConfig::getBool : Value is an Array");
	if (vi->second[0].m_type!=LuaValue::BOOL) throw std::runtime_error("LuaConfig::getBool : Value has wrong type");
	return vi->second[0].m_bool;
}

//////////////////////////////////////////////////////////////////////////////

double
LuaConfig::getDouble(const std::string & name)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getDouble : Name not found");
	if (vi->second.size()!=1) throw std::runtime_error("LuaConfig::getDouble : Value is an Array");
	if (vi->second[0].m_type!=LuaValue::NUMBER) throw std::runtime_error("LuaConfig::getDouble : Value has wrong type");
	return vi->second[0].m_number;
}

//////////////////////////////////////////////////////////////////////////////

int
LuaConfig::getInt(const std::string & name)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getInt : Name not found");
	if (vi->second.size()!=1) throw std::runtime_error("LuaConfig::getInt : Value is an Array");
	if (vi->second[0].m_type!=LuaValue::INT) throw std::runtime_error("LuaConfig::getInt : Value has wrong type");
	return vi->second[0].m_int;
}

//////////////////////////////////////////////////////////////////////////////

std::string
LuaConfig::getString(const std::string & name)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getString : Name not found");
	if (vi->second.size()!=1) throw std::runtime_error("LuaConfig::getString : Value is an Array");
	if (vi->second[0].m_type!=LuaValue::STRING) throw std::runtime_error("LuaConfig::getString : Value has wrong type");
	return vi->second[0].m_string;
}

//////////////////////////////////////////////////////////////////////////////

bool
LuaConfig::getBool(const std::string & name, const unsigned arrayindex)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getBool : Name not found");
	if (vi->second.size()==0) throw std::runtime_error("LuaConfig::getBool : No value present");
	if (arrayindex >= vi->second.size()) throw std::runtime_error("LuaConfig::getBool : Index out of range");
	if (vi->second[arrayindex].m_type!=LuaValue::BOOL) throw std::runtime_error("LuaConfig::getBool : Value has wrong type");
	return vi->second[arrayindex].m_bool;
}

//////////////////////////////////////////////////////////////////////////////

double
LuaConfig::getDouble(const std::string & name, const unsigned arrayindex)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getDouble : Name not found");
	if (vi->second.size()==0) throw std::runtime_error("LuaConfig::getDouble : No value present");
	if (arrayindex >= vi->second.size()) throw std::runtime_error("LuaConfig::getDouble : Index out of range");
	if (vi->second[arrayindex].m_type!=LuaValue::NUMBER) throw std::runtime_error("LuaConfig::getDouble : Value has wrong type");
	return vi->second[arrayindex].m_number;
}

//////////////////////////////////////////////////////////////////////////////

int
LuaConfig::getInt(const std::string & name, const unsigned arrayindex)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getInt : Name not found");
	if (vi->second.size()==0) throw std::runtime_error("LuaConfig::getInt : No value present");
	if (arrayindex >= vi->second.size()) throw std::runtime_error("LuaConfig::getInt : Index out of range");
	if (vi->second[arrayindex].m_type!=LuaValue::INT) throw std::runtime_error("LuaConfig::getInt : Value has wrong type");
	return vi->second[arrayindex].m_int;
}

//////////////////////////////////////////////////////////////////////////////

std::string
LuaConfig::getString(const std::string & name, const unsigned arrayindex)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getString : Name not found");
	if (vi->second.size()==0) throw std::runtime_error("LuaConfig::getString : No value present");
	if (arrayindex >= vi->second.size()) throw std::runtime_error("LuaConfig::getString : Index out of range");
	if (vi->second[arrayindex].m_type!=LuaValue::STRING) throw std::runtime_error("LuaConfig::getString : Value has wrong type");
	return vi->second[arrayindex].m_string;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<bool>
LuaConfig::getBoolArray(const std::string & name)
{
	std::vector<bool> ret;
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getBoolArray : Name not found");
	const LuaValueArray & l = vi->second;
	for (auto & i : l) {
		if (i.m_type!=LuaValue::BOOL) throw std::runtime_error("LuaConfig::getBoolArray : Value has wrong type");
		ret.push_back(i.m_bool);
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<double>
LuaConfig::getDoubleArray(const std::string & name)
{
	std::vector<double> ret;
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getDoubleArray : Name not found");
	const LuaValueArray & l = vi->second;
	for (auto & i : l) {
		if (i.m_type!=LuaValue::NUMBER) throw std::runtime_error("LuaConfig::getDoubleArray : Value has wrong type");
		ret.push_back(i.m_number);
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<int>
LuaConfig::getIntArray(const std::string & name)
{
	std::vector<int> ret;
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getIntArray : Name not found");
	const LuaValueArray & l = vi->second;
	for (auto & i : l) {
		if (i.m_type!=LuaValue::INT) throw std::runtime_error("LuaConfig::getIntArray : Value has wrong type");
		ret.push_back(i.m_int);
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<std::string>
LuaConfig::getStringArray(const std::string & name)
{
	std::vector<std::string> ret;
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getStringArray : Name not found");
	const LuaValueArray & l = vi->second;
	for (auto & i : l) {
		if (i.m_type!=LuaValue::STRING) throw std::runtime_error("LuaConfig::getStringArray : Value has wrong type");
		ret.push_back(i.m_string);
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

LuaValueArray
LuaConfig::getValueArray(const std::string & name)
{
	auto vi = m_valuemap.find(name);
	if (vi==m_valuemap.end()) throw std::runtime_error("LuaConfig::getValueArray : Name not found");
	return vi->second;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<std::string>
LuaConfig::getNames() const
{
	std::vector<std::string> ret;
	for (auto v : m_valuemap) ret.push_back(v.first);
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

