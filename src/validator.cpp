#include "validator.h"
#include "models/FDRRecord.h"
#include "models/FieldMetaData.h"
#include "models/ValidationWarning.h"
#include <map>
#include <vector>

Validator::Validator(const std::vector<FDRRecord> &data) : m_data{data} {
  validateData();
}

const std::map<int, std::vector<ValidationWarning>> &
Validator::getWarnings() const {
  return m_warnings;
}

void Validator::validateData() {
  for (const auto &row : m_data) {
    std::vector<ValidationWarning> warnings = validateRow(row);
    if (!warnings.empty()) {
      m_warnings[row.line_number] = warnings;
    }
  }
}

std::vector<ValidationWarning> Validator::validateRow(const FDRRecord &row) {
  std::vector<ValidationWarning> warnings{};
  for (const auto &meta : fieldMetaData) {
    if (!meta.validate) {
      continue;
    }
    double curr{row.*meta.member};
    if (curr < meta.min || curr > meta.max) {
      warnings.push_back(ValidationWarning{
          {meta.name, row.line_number}, curr, meta.min, meta.max});
    }
  }
  return warnings;
}
