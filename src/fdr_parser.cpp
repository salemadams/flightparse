#include "fdr_parser.h"
#include "models/FDRRecord.h"
#include "models/FieldMetaData.h"
#include <array>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

FdrParser::FdrParser(const std::string &path)
    : m_path{path}, m_file(openFile()) {
  parseFlightData();
}

std::ifstream FdrParser::openFile() {
  std::ifstream file{m_path};
  if (!file) {
    throw std::runtime_error("error: could not open file: " + m_path);
  }
  return file;
}

bool FdrParser::tryParseDouble(const std::string &str, double &out) {
  try {
    out = std::stod(str);
    return true;
  } catch (const std::invalid_argument &) {
    return false;
  } catch (const std::out_of_range &) {
    return false;
  }
}

std::vector<std::string> FdrParser::getHeader() {
  std::string header{};
  std::getline(m_file, header);
  std::stringstream hs{header};
  std::vector<std::string> fieldNames{};
  std::string field{};
  while (std::getline(hs, field, ',')) {
    fieldNames.push_back(field);
  }
  return fieldNames;
}

// assigns parsed values to FDRRecord fields via pointer-to-member
FDRRecord FdrParser::mapFlightData(const std::vector<double> &values) {
  FDRRecord rec{};
  int i{0};
  for (const auto &meta : fieldMetaData) {
    rec.*meta.member = values[i++];
  }
  return rec;
}

void FdrParser::parseDataRows(const std::vector<std::string> &fieldNames) {
  std::string row{};
  int lineNum{1};
  while (std::getline(m_file, row)) {
    lineNum++;
    std::vector<ValidationError> errors{};
    bool parseOk{true};
    std::stringstream ss{row};
    std::array<std::string, kFieldCount> fields{};
    std::vector<double> values(kFieldCount, 0.0);
    for (int i = 0; i < kFieldCount; i++) {
      std::getline(ss, fields[i], ',');
      if (!tryParseDouble(fields[i], values[i])) {
        parseOk = false;
        std::string msg = fields[i].empty()
                              ? "Missing field value"
                              : "Non-numeric value: '" + fields[i] + "'";
        errors.push_back(ValidationError{{fieldNames[i], lineNum}, msg});
      }
    }
    if (!parseOk) {
      m_errors[lineNum] = errors;
      continue;
    }
    FDRRecord rec = mapFlightData(values);
    rec.line_number = lineNum;
    m_data.push_back(rec);
  }
}

void FdrParser::parseFlightData() {
  std::vector<std::string> fieldNames{getHeader()};
  parseDataRows(fieldNames);
}
