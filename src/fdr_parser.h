#ifndef FDR_PARSER_H
#define FDR_PARSER_H
#include "models/FDRRecord.h"
#include "models/ValidationError.h"
#include <fstream>
#include <map>
#include <string>
#include <vector>

// reads and parses FDR CSV files into validated records
class FdrParser {
public:
  FdrParser(const std::string &path);
  const std::vector<FDRRecord> &getCSVData() const { return m_data; }
  const std::map<int, std::vector<ValidationError>> &getErrors() const {
    return m_errors;
  }

private:
  std::string m_path;
  std::ifstream m_file;
  std::vector<FDRRecord> m_data;
  std::map<int, std::vector<ValidationError>> m_errors;
  std::ifstream openFile();
  bool tryParseDouble(const std::string &str, double &out);
  std::vector<std::string> getHeader();
  FDRRecord mapFlightData(const std::vector<double> &values);
  void parseDataRows(const std::vector<std::string> &fieldNames);
  void parseFlightData();
};
#endif
