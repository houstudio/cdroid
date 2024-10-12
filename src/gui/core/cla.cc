#include <core/cla.h>
#include <stdio.h>
#include <cctype>
#include <algorithm> // std::min
#include <sstream>   // std::stringstream
#include <iomanip>   // std::left, std::setw
#include <stdarg.h>
#include <assert.h>
#define error_if(x, err, msg, ...) do { if((x)) { mError = FormatString(msg, ## __VA_ARGS__); return err; } } while (false)

namespace cdroid {

static std::string TypeToString(CLA::ValueType type) {
    switch (type) {
    case CLA::ValueType::Bool:      return std::string("Bool");
    case CLA::ValueType::Double:    return std::string("Double");
    case CLA::ValueType::Float:     return std::string("Float");
    case CLA::ValueType::Int:       return std::string("Int");
    case CLA::ValueType::String:    return std::string("String");
    case CLA::ValueType::None:      return std::string("None");
    default:        return std::string("Invalid");
    }
}

static std::string FormatString(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char strBuffer[1024];
    vsnprintf(strBuffer, sizeof(strBuffer), format, args);
    va_end(args);
    std::string returnValue(strBuffer);
    return std::move(returnValue);
}
static std::string ExtractFilename(const std::string &path) {
    std::string result = path;
    //ReplaceCharacters(result, '/', '\\');
    size_t filenameStart = result.find_last_of('/');
    filenameStart = (filenameStart == std::string::npos) ? 0 : filenameStart + 1;
    result.erase(0, filenameStart);
    return result;
}

static std::string ToLower(const std::string &str) {
    std::string retValue=str;
    std::transform(retValue.begin(), retValue.end(), retValue.begin(), [](char c) {
         return std::tolower(c);
    });
    return retValue;
}

CLA::Argument::Argument(CLA::EntryType entryType, const std::string &shortName, const std::string &longName, const std::string &description, 
	CLA::ValueType valueType, int entryFlags)
    : mEntryType(entryType)
    , mShortName(shortName)
    , mLongName(longName)
    , mDescription(description)
    , mValueType(valueType)
    , mEntryFlags(entryFlags) {

}

CLA::CLA():mSwitchChars(std::string("-/")){
}

CLA::CLA(const std::vector<Argument>& arguments)
    : mArguments(arguments)
    , mSwitchChars(std::string("-/")) {
}

CLA::CLA(const Argument *arguments, size_t argumentCount) {
    for (unsigned i = 0; i < argumentCount; ++i)
        mArguments.push_back(arguments[i]);
}

int CLA::addArguments(const std::vector<Argument>& arguments){
    return addArguments(arguments.data(),arguments.size());//mArguments.insert(mArguments.end(),arguments.begin(),arguments.end());
}

int CLA::addArguments(const Argument *arguments, size_t argumentCount){
    for (unsigned i = 0; i < argumentCount; ++i)
        mArguments.push_back(arguments[i]);
    return argumentCount;
}

bool CLA::setArgument(const std::string&arg,const std::string&value){
    auto valueItr = mArgumentValues.find(_getLongArgName(arg));
    if(valueItr==mArgumentValues.end()){
	valueItr = mArgumentValues.find(_getShortArgName(arg));
    }
    if(valueItr!=mArgumentValues.end()){
        valueItr->second=value;
    }
    return valueItr!=mArgumentValues.end();
}

CLA::~CLA() {
    mArguments.clear();
}

CLA::Result CLA::parse(int argc, const char **argv) {
    error_if(argv == nullptr, Result::ErrorNullArgument, "Null arguments passed");
    error_if(argc <= 0, Result::ErrorArgCount, "Invalid number of argmuments passed");

    error_if(argv[0] == nullptr, Result::ErrorNullArgument, "Null executable name (argument 0)");
    mApplicationName = ExtractFilename(argv[0]);

    for (int i = 1; i < argc; ++i) {
        error_if(argv[i]== nullptr, Result::ErrorNullArgument, "Null argument passed");
        std::string currentArg(argv[i]);

        bool hasSwitch = false;
        bool hasLongSwitch = false;
        for (auto &switchChar : mSwitchChars) {
            size_t switchPos = currentArg.find(switchChar);
            if (switchPos == 0) {
                hasSwitch = true;
                if (currentArg.find(switchChar, switchPos + 1) == 1)
                    hasLongSwitch = true;
            }
        }

        if (hasLongSwitch || hasSwitch) {
            std::string parsedArg;

            if (hasLongSwitch) {
                parsedArg = currentArg.substr(2, std::string::npos);
            } else {
                parsedArg = currentArg.substr(1, std::string::npos);
            }

            std::string argName, argValue;

            auto eqPos = parsedArg.find_first_of('=');
            if (eqPos == std::string::npos){ // Did not find '=' (ex: -o file.cpp, -editor)
                argName = parsedArg;
            }else if (eqPos != std::string::npos){ // Look for '=' (ex: -o=file.cpp)
                argName = parsedArg.substr(0, eqPos);
                argValue = parsedArg.substr(eqPos + 1);
            }else{ // Assumed to be switch
                argName = argValue = parsedArg;
            }

            if (argName.empty()){ // Empty arg (ex: my.exe -)
                mError = std::string("Invalid empty argument");
                return Result::ErrorUnkown;
            }

            // Search for description with the same name
            Argument *argDesc = nullptr;
            for (auto &argDescItr : mArguments) {
                if ((argDescItr.mShortName == argName) || (argDescItr.mLongName == argName)) {
                    argDesc = &argDescItr;
                    break;
                }
            }

            if (!argDesc) { // Invalid argument given
                mError = std::string("Invalid argument flag '") + argName + std::string("'");
                return Result::ErrorUnkown;
            }

            EntryType argType = argDesc->mEntryType;

            if (argType == EntryType::Option){ // Type logic
                if (argValue.empty()){ // Argument is in two parts (ex: -o file.cpp)
                    if (i >= argc - 1){ // No value passed to arg
                        mError = std::string("Invalid argument value for flag '") + argName + std::string("'");
                        return Result::ErrorUnkown;
                    }
                    argValue = std::string(argv[++i]);
                }
            }

            if ((argDesc->mShortName == argName) && hasLongSwitch){ // Long switch with short name
                mError = std::string("Invalid argument flag '") + argName + std::string("'");
                return Result::ErrorUnkown;
            }

            if (argType == EntryType::Switch){ // Switches
                mSwitches.push_back(argName);
            }else{ // Options
                // Store values
                if (!argDesc->mShortName.empty())
                    mArgumentValues[argDesc->mShortName] = argValue;
                if (!argDesc->mLongName.empty())
                    mArgumentValues[argDesc->mLongName] = argValue;
            }
        }else{ // Assumed to be parameter
            mParameters.push_back(currentArg);
        }
    }

    // Check for all mandatory flags
    size_t paramCount = 0;
    for (auto &i : mArguments) {
        if (i.mEntryFlags & (int)EntryFlags::Manditory){ // Only check mandatory options
            switch (i.mEntryType) {
            case EntryType::Switch:
                // Short and long not find
                error_if(!(findSwitch(i.mLongName) || findSwitch(i.mShortName)), Result::ErrorMissingArg, "Missing required switch %s", i.mShortName.c_str());
                break;
            case EntryType::Parameter:{
                paramCount++;
                error_if(paramCount > mParameters.size(), Result::ErrorTooManyParams, "Too many parameters given");
                break;
            }
            default:{
                auto shortItr = mArgumentValues.find(i.mShortName);
                auto longItr = mArgumentValues.find(i.mLongName);

                // Short and long not find
                error_if((longItr == mArgumentValues.end()) && (shortItr == mArgumentValues.end()), Result::ErrorMissingArg, "Missing required option %s", i.mShortName.c_str());
            }
            break;
            }
        }
    }

    return Result::OK;
}

bool CLA::find(const std::string&arg)const{
   return _getArgumentValue(arg)!=nullptr;     
}

bool CLA::find(const std::string &argument, std::string &destination) const {
    auto valueStr = _getArgumentValue(argument);
    if (valueStr == nullptr) {
        return false;
    }
    destination = *valueStr;
    return true;
}

bool CLA::find(const std::string &argument, bool &destination) const {
    auto valueStr = _getArgumentValue(argument);
    if (valueStr == nullptr) {
        return false;
    }
    // Parse bool value from string
    std::string lowerValue(ToLower(*valueStr));
    if (lowerValue == std::string("true"))
        destination = true;
    else if (lowerValue == std::string("false"))
        destination = false;
    return true;
}

bool CLA::find(const std::string &argument, int &destination) const {
    auto valueStr = _getArgumentValue(argument);
    if (valueStr == nullptr) {
        return false;
    }
    destination = std::stoi(valueStr->c_str());
    return true;
}

bool CLA::find(const std::string &argument, float &destination) const {
    auto valueStr = _getArgumentValue(argument);
    if (valueStr == nullptr) {
        return false;
    }
    destination = static_cast<float>(std::stof(valueStr->c_str()));
    return true;
}

bool CLA::find(const std::string &argument, double &destination) const {
    auto valueStr = _getArgumentValue(argument);
    if (valueStr == nullptr) {
        return false;
    }
    destination = std::stof(valueStr->c_str());
    return true;
}

bool CLA::find(const std::string &argument, unsigned &destination) const {
    auto valueStr = _getArgumentValue(argument);
    if (valueStr == nullptr) {
        return false;
    }
    destination = static_cast<unsigned>(std::stoi(valueStr->c_str()));
    return true;
}

bool CLA::find(const std::string &argument, char &destination) const {
    auto valueStr = _getArgumentValue(argument);
    if (valueStr == nullptr) {
        return false;
    }
    destination = static_cast<char>(std::stoi(valueStr->c_str()));
    return true;
}

bool CLA::find(const std::string &argument, unsigned char &destination) const {
    auto valueStr = _getArgumentValue(argument);
    if (valueStr == nullptr) {
        return false;
    }
    destination = static_cast<unsigned char>(std::stoi(valueStr->c_str()));
    return true;
}

void CLA::setSwitchChars(const std::string &switchChars) {
    mSwitchChars = switchChars;
}

void CLA::setSwitchChars(char c) {
    mSwitchChars.resize(1);
    mSwitchChars[0] = c;
}

bool CLA::findSwitch(const std::string &argument) const {
    return (std::find(mSwitches.begin(), mSwitches.end(), _getLongArgName(argument)) != mSwitches.end()) ||
	   (std::find(mSwitches.begin(), mSwitches.end(), _getShortArgName(argument)) != mSwitches.end());
}

size_t CLA::getParamCount() const {
    return mParameters.size();
}

CLA::Result CLA::getParam(size_t paramIndex, std::string &destination) const {
    error_if(paramIndex >= mParameters.size(), Result::ErrorArgCount, "Invalid param. index");
    destination = mParameters[paramIndex];
    return CLA::Result::OK;
}

const std::string* CLA::_getArgumentValue(const std::string& argName) const {
    auto valueItr = mArgumentValues.find(_getLongArgName(argName));
    if (valueItr == mArgumentValues.end()) {
        valueItr = mArgumentValues.find(_getShortArgName(argName));
        if (valueItr == mArgumentValues.end())
            return nullptr;
    }
    return &valueItr->second;
}

std::string CLA::_getLongArgName(const std::string& argName) const {
    for (auto&& arg : mArguments) {
        if ((arg.mLongName == argName) || (arg.mShortName == argName)) {
            return arg.mLongName;
        }
    }
    return std::string();
}

std::string CLA::_getShortArgName(const std::string& argName) const {
    for (auto&& arg : mArguments) {
        if ((arg.mLongName == argName) || (arg.mShortName == argName)) {
            return arg.mShortName;
        }
    }
    return std::string();
}

void CLA::_generateUsageString() {
    std::vector<std::string> flagNames;
    std::vector<std::string> flagDescriptions;
    std::vector<std::string> argumentStrings;

    size_t longestName = 0;

    // Initial command line
    for (auto &i : mArguments) {
        bool optional = (i.mEntryFlags | (int)EntryFlags::Optional) != 0;
        std::string argString = std::string(" ");

        // Prologue
        if (optional)
            argString += std::string("[");

        argString += mSwitchChars[0];
        argString += i.mShortName;

        // Generate usage name
        std::string flagName = mSwitchChars.substr(0, 1) + i.mShortName;
        if (!i.mLongName.empty()) {
            flagName += (std::string(", ") + mSwitchChars.substr(0, 1)) + mSwitchChars.substr(0, 1);
            flagName += i.mLongName;

            argString += (std::string("|") + mSwitchChars.substr(0, 1)) + mSwitchChars.substr(0, 1) + i.mLongName;
        }


        switch (i.mEntryType) {
        case EntryType::Option:
            break;
        case EntryType::Parameter:
            argString += std::string(" <") + TypeToString(i.mValueType) + std::string(">");
            flagName += std::string("=<") + TypeToString(i.mValueType) + std::string(">");
            break;
        case EntryType::Switch:
            break;
        case EntryType::UsageText:
            break;
        default:
            break;
        }

        // Epilogue
        if (optional) {
            argString += std::string("]");
        }

        flagDescriptions.push_back(i.mDescription);
        flagNames.push_back(flagName);
        argumentStrings.push_back(argString);

        size_t flagLength = flagName.length();
        if (flagLength > longestName)
            longestName = flagLength;
    }

    std::stringstream usageStream;
    usageStream << std::string("Usage: ") + mApplicationName;
    for (auto &i : argumentStrings)
        usageStream << i + std::string(" ");

    usageStream << std::endl;
    for (unsigned i = 0; i < flagNames.size(); ++i) {
        usageStream << std::left << std::setw(longestName) << flagNames[i] << std::string("\t") << flagDescriptions[i] << std::endl;
    }

    auto output = usageStream.str();
    mUsageString = output;
}

const std::string &CLA::getUsageString() {
    if (mUsageString.empty()) {
        _generateUsageString();
    }
    return mUsageString;
}

}

