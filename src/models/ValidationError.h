#ifndef VALIDATIONERROR_H
#define VALIDATIONERROR_H
#include "ValidationIssue.h"
#include <string>
// parse-time error — row rejected from dataset
struct ValidationError : ValidationIssue {
  std::string message;
};
#endif
