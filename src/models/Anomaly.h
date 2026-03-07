#ifndef ANOMALY_H
#define ANOMALY_H
#include <string>
// categories of flagged flight anomalies
enum class AnomalyType {
  HardLanding,
  SevereHardLanding,
  NegativeG,
  ExcessiveBank,
  HighDescentRate,
  Overspeed,
  PitchUp,
  PitchDown,
};
// single detected anomaly with context for reporting
struct Anomaly {
  double time_s;
  AnomalyType type;
  std::string field;
  double value;
  double threshold;
};
#endif
