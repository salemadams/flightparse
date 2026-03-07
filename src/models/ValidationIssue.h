#ifndef VALIDATIONISSUE_H
#define VALIDATIONISSUE_H
#include <string>
// shared fields for all validation diagnostics
struct ValidationIssue {
  std::string field_name;
  int line_number;
};
#endif
