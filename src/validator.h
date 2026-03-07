#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "models/FDRRecord.h"
#include "models/ValidationWarning.h"
#include <map>
#include <vector>

// checks parsed records against 14 CFR Appendix B parameter ranges
class Validator {
public:
  Validator(const std::vector<FDRRecord> &data);
  const std::map<int, std::vector<ValidationWarning>> &getWarnings() const;

private:
  const std::vector<FDRRecord> &m_data;
  std::map<int, std::vector<ValidationWarning>> m_warnings;

  void validateData();
  std::vector<ValidationWarning> validateRow(const FDRRecord &row);
};

#endif
