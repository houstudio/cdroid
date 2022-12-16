#pragma once
// STL
#include <unordered_map>
#include <vector>

namespace cdroid{


  class CLA{
  public:	  
      // Types of arguments
      enum class EntryType{
          Switch,
          Option,
          Parameter,
          UsageText,
          None
      };

      // Types of values for options and parameters
      enum class ValueType{
          Int,
          Float,
          Double,
          String,
          Bool,
          None
      };

      enum class EntryFlags{
          None = 0,
          Optional =    1 << 1,
          Manditory =   1 << 2,
          Multiple =    1 << 3,
          Help =        1 << 4,
          Negatable =   1 << 5
      };
      // Describes an argument to look for
      struct Argument{
          EntryType     mEntryType;
          const std::string   mShortName;
          const std::string   mLongName;
          const std::string   mDescription;
          ValueType     mValueType;
          int           mEntryFlags;
	  Argument(EntryType, const std::string &shortName, const std::string &longName, 
             const std::string &des,ValueType valueType, int entryFlags);
      };

      enum Result{
          OK,
          Help,
          ErrorUnkown,
          ErrorNullArgument,
          ErrorArgCount,
          ErrorMissingArg,
          ErrorTooManyParams,
          None
      };
  public:
      CLA();
      CLA(const Argument *arguments, size_t argumentCount);
      CLA(const std::vector<Argument>& arguments);
      ~CLA();
      Result parse(int argc, const char **argv);
      int addArguments(const std::vector<Argument>& arguments);
      int addArguments(const Argument *arguments, size_t argumentCount);
      bool setArgument(const std::string&arg,const std::string&value);
      void setSwitchChars(const std::string &switchChars);
      void setSwitchChars(char c);

      bool findSwitch(const std::string &argument) const;
      bool find(const std::string&)const;
      bool find(const std::string &argument, std::string   &value) const;
      bool find(const std::string &argument, bool          &value) const;
      bool find(const std::string &argument, int           &value) const;
      bool find(const std::string &argument, unsigned      &value) const;
      bool find(const std::string &argument, float         &value) const;
      bool find(const std::string &argument, double        &value) const;
      bool find(const std::string &argument, char          &value) const;
      bool find(const std::string &argument, unsigned char &value) const;

      size_t getParamCount() const;
      Result getParam(size_t paramIndex, std::string &destination) const;
      const std::string &getError() const { return mError; };
      const std::string &getUsageString();

  private:
      const std::string* _getArgumentValue(const std::string& argName) const;
      std::string _getLongArgName(const std::string& argName) const;
      std::string _getShortArgName(const std::string& argName) const;

      // Generates usage based off of given values
      void _generateUsageString();

      // Arguments stored by their option
      typedef std::unordered_map<std::string, std::string> ArgumentMap;
      ArgumentMap mArgumentValues;

      // Switches found
      std::vector<std::string> mSwitches;

      // Parameters found
      std::vector<std::string> mParameters;

      std::vector<Argument> mArguments;

      // Error string if any occur
      mutable std::string mError;

      // Characters that are used as switches
      std::string mSwitchChars;

      // Use string displayed if queried
      std::string mUsageString;
    
      // Name of application
      std::string mApplicationName;
  };

} //endof namespace
