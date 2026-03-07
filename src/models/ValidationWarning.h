#ifndef VALIDATIONWARNING_H
#define VALIDATIONWARNING_H
#include "ValidationIssue.h"
// out-of-range value — row kept but flagged
struct ValidationWarning : ValidationIssue {
  double actual_value;
  double min_allowed;
  double max_allowed;
};
#endif
