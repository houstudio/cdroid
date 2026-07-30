#include <navigation/navtype.h>
#include <stdexcept>

namespace cdroid{

void IntNavType::put(Bundle& b, const std::string& k, const int& v){ b.putInt(k, v); }
int IntNavType::get(const Bundle& b, const std::string& k){ return b.getInt(k); }
int IntNavType::parseValue(const std::string& v){ return std::stoi(v); }
std::string IntNavType::serializeAsValue(const int& v){ return std::to_string(v); }

void LongNavType::put(Bundle& b, const std::string& k, const long& v){ b.putLong(k, v); }
long LongNavType::get(const Bundle& b, const std::string& k){ return b.getLong(k); }
long LongNavType::parseValue(const std::string& v){ return std::stol(v); }
std::string LongNavType::serializeAsValue(const long& v){ return std::to_string(v); }

void FloatNavType::put(Bundle& b, const std::string& k, const float& v){ b.putFloat(k, v); }
float FloatNavType::get(const Bundle& b, const std::string& k){ return b.getFloat(k); }
float FloatNavType::parseValue(const std::string& v){ return std::stof(v); }
std::string FloatNavType::serializeAsValue(const float& v){ return std::to_string(v); }

void BoolNavType::put(Bundle& b, const std::string& k, const bool& v){ b.putBoolean(k, v); }
bool BoolNavType::get(const Bundle& b, const std::string& k){ return b.getBoolean(k); }
bool BoolNavType::parseValue(const std::string& v){ return v == "true"; }
std::string BoolNavType::serializeAsValue(const bool& v){ return v ? "true" : "false"; }

void StringNavType::put(Bundle& b, const std::string& k, const std::string& v){ b.putString(k, v); }
std::string StringNavType::get(const Bundle& b, const std::string& k){ return b.getString(k); }
std::string StringNavType::parseValue(const std::string& v){ return v; }
std::string StringNavType::serializeAsValue(const std::string& v){ return v; }

NavTypeKind navTypeKindFromName(const std::string& type){
    if(type == "integer" || type == "int") return NavTypeKind::INT;
    if(type == "long")                    return NavTypeKind::LONG;
    if(type == "float")                   return NavTypeKind::FLOAT;
    if(type == "boolean" || type == "bool") return NavTypeKind::BOOL;
    if(type == "string")                  return NavTypeKind::STRING;
    if(type == "integer[]" || type == "int[]") return NavTypeKind::INT_ARRAY;
    if(type == "long[]")                  return NavTypeKind::LONG_ARRAY;
    if(type == "float[]")                 return NavTypeKind::FLOAT_ARRAY;
    if(type == "boolean[]")               return NavTypeKind::BOOL_ARRAY;
    if(type == "string[]")                return NavTypeKind::STRING_ARRAY;
    return NavTypeKind::UNKNOWN;
}

}//namespace cdroid
