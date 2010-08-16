#ifndef __CONFIGURATION_FILE_PARSER_H_
#define __CONFIGURATION_FILE_PARSER_H_


#include <string>
#include <map>

typedef std::map<std::string, std::string> ConfigurationFileSectionStruct;


/** 
 * Loads and parses a configuration file and provides methods for accessing
 * settings values. The format of a configuration file follows the INI de facto 
 * standard (http://en.wikipedia.org/wiki/Ini_file). Keys must contain no spaces,
 * but values can.
 */

class ConfigurationFileParser {
 public:
  ConfigurationFileParser();
  ~ConfigurationFileParser();

  void Parse(std::string fileName);

  std::string GetValue(std::string section, std::string key);

  bool GetValueAsBool(std::string section, std::string key, bool defaultValue=false);

  int GetValueAsInt(std::string section, std::string key, int defaultValue=0);

  double GetValueAsDouble(std::string section, std::string key, double defaultValue=0.0);

  void GetValueAsDouble3(std::string section, std::string key, double values[3]);

 protected:


 private:
  std::map<std::string, ConfigurationFileSectionStruct> sectionMap;

};

// __CONFIGURATION_FILE_PARSER
#endif
