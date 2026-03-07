#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H
#include "models/Anomaly.h"
#include "models/FDRRecord.h"
#include "models/ReportMetrics.h"
#include "models/ValidationError.h"
#include "models/ValidationWarning.h"
#include <map>
#include <string>
#include <vector>
// writes analysis results to output files (CSV, summary, metrics)
class ReportGenerator {
public:
  ReportGenerator(const std::string &path, const ReportMetrics &metrics,
                  const std::vector<FDRRecord> &data,
                  const std::map<int, std::vector<ValidationError>> &errors,
                  const std::map<int, std::vector<ValidationWarning>> &warnings,
                  const std::vector<Anomaly> &anomalies);

private:
  std::string m_path;
  const ReportMetrics &m_metrics;
  const std::vector<FDRRecord> &m_data;
  const std::map<int, std::vector<ValidationError>> &m_errors;
  const std::map<int, std::vector<ValidationWarning>> &m_warnings;
  const std::vector<Anomaly> &m_anomalies;
  void generateFiles();
  void generateReport();
  void generateCSV();
  void generateMetrics();
};
#endif
