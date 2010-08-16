#include "ConfigurationFileParser.h"

#include <cstdio>


ConfigurationFileParser
::ConfigurationFileParser() {

}


ConfigurationFileParser
::~ConfigurationFileParser() {
  
}


void
ConfigurationFileParser
::Parse(std::string fileName) {
  FILE* fp = fopen(fileName.c_str(), "r");
  if (fp == NULL)
    return;

  std::string currentSection;
  char line[1024];
  while (fgets(line, 1023, fp)) {

    if (strlen(line) == 1) {
      // Empty line
      continue;
    } else if (line[0] == ';') {
      // Comment
      continue;
    } else if (line[0] == '[') {
      // Start new section
      char* rightBracket = strstr(line, "]");
      *rightBracket = '\0';

      ConfigurationFileSectionStruct newStruct;
      currentSection = std::string(line+1);
      this->sectionMap[currentSection] = newStruct;
    } else {
      // New entry

      // Zap newline from the string
      line[strlen(line)-1] = '\0';

      char* equalSign = strstr(line, "=");
      *equalSign = '\0';

      std::string key = std::string(line);
      std::string val = std::string(equalSign+1);

      this->sectionMap[currentSection][key] = val;
    }
  }

  fclose(fp);
}


std::string
ConfigurationFileParser
::GetValue(std::string section, std::string key) {
  return this->sectionMap[section][key];
}


bool
ConfigurationFileParser
::GetValueAsBool(std::string section, std::string key, bool defaultValue) {
  std::string strValue = this->GetValue(section, key);
  bool value = defaultValue;
  if (strValue == "true")
    value = true;
  else if (strValue == "false")
    value = false;

  return value;
}


int
ConfigurationFileParser
::GetValueAsInt(std::string section, std::string key, int defaultValue) {
  std::string strValue = this->GetValue(section, key);
  int value = defaultValue;
  sscanf(strValue.c_str(), "%d", &value);

  return value;
}


double
ConfigurationFileParser
::GetValueAsDouble(std::string section, std::string key, double defaultValue) {
  std::string strValue = this->GetValue(section, key);
  double value = defaultValue;
  sscanf(strValue.c_str(), "%lf", &value);

  return value;
}


void
ConfigurationFileParser
::GetValueAsDouble3(std::string section, std::string key, double values[3]) {
  std::string value = this->GetValue(section, key);
  values[0] = values[1] = values[2] = 0.0;
  sscanf(value.c_str(), "%lf %lf %lf", values+0, values+1, values+2);
}
