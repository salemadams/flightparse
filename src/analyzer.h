#ifndef ANALYZER_H
#define ANALYZER_H
#include "models/Anomaly.h"
#include "models/FDRRecord.h"
#include "models/ReportMetrics.h"
#include <vector>
// computes flight metrics and detects anomalies from parsed FDR data
class Analyzer {
public:
  Analyzer(const std::vector<FDRRecord> &data);
  const std::vector<Anomaly> &getAnomalies() const { return m_anomalies; }
  const ReportMetrics &getReportMetrics() const { return m_reportMetrics; }

private:
  const std::vector<FDRRecord> &m_data;
  ReportMetrics m_reportMetrics;
  std::vector<Anomaly> m_anomalies;
  void computeMetrics();
  void detectAnomalies();
  void computeClimbRate();
  void computeTrueAirspeed();
  void computeMachNumber();
  void computeDensityAltitude();
  void computeSegmentDistances();
};
#endif
