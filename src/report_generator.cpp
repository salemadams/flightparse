#include "report_generator.h"
#include "models/FieldMetaData.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <stdexcept>

static std::string anomalyTypeName(AnomalyType t) {
  switch (t) {
  case AnomalyType::HardLanding:
    return "HardLanding";
  case AnomalyType::SevereHardLanding:
    return "SevereHardLanding";
  case AnomalyType::NegativeG:
    return "NegativeG";
  case AnomalyType::ExcessiveBank:
    return "ExcessiveBank";
  case AnomalyType::HighDescentRate:
    return "HighDescentRate";
  case AnomalyType::Overspeed:
    return "Overspeed";
  case AnomalyType::PitchUp:
    return "PitchUp";
  case AnomalyType::PitchDown:
    return "PitchDown";
  default:
    return "Unknown";
  }
}

ReportGenerator::ReportGenerator(
    const std::string &path, const ReportMetrics &metrics,
    const std::vector<FDRRecord> &data,
    const std::map<int, std::vector<ValidationError>> &errors,
    const std::map<int, std::vector<ValidationWarning>> &warnings,
    const std::vector<Anomaly> &anomalies)
    : m_path{path}, m_metrics{metrics}, m_data{data}, m_errors{errors},
      m_warnings{warnings}, m_anomalies{anomalies} {
  generateFiles();
}

void ReportGenerator::generateFiles() {
  generateCSV();
  generateReport();
  generateMetrics();
}

void ReportGenerator::generateCSV() {
  std::string fileName{m_path + "_data.csv"};
  std::ofstream fout{fileName};
  if (!fout) {
    throw std::runtime_error("error: could not create file: " + fileName);
  }
  bool first{true};
  for (const auto &meta : fieldMetaData) {
    if (!first) {
      fout << ",";
    }
    first = false;
    fout << meta.name;
  }
  fout << '\n';
  for (const auto &row : m_data) {
    first = true;
    for (const auto &meta : fieldMetaData) {
      if (!first) {
        fout << ",";
      }
      first = false;
      fout << row.*meta.member;
    }
    fout << '\n';
  }
}

void ReportGenerator::generateReport() {
  std::string fileName{m_path + "_summary.txt"};
  std::ofstream fout{fileName};
  if (!fout) {
    throw std::runtime_error("error: could not create file: " + fileName);
  }
  fout << "Anomalies [" << m_anomalies.size() << "]" << '\n';
  for (const auto &anomaly : m_anomalies) {
    fout << "  t=" << anomaly.time_s << "s | " << anomalyTypeName(anomaly.type)
         << " | " << anomaly.field << " = " << anomaly.value
         << " (threshold: " << anomaly.threshold << ")\n";
  }
  fout << '\n';
  fout << "Errors [" << m_errors.size() << "]" << '\n';
  for (const auto &[_, elist] : m_errors) {
    for (const auto &e : elist) {
      fout << "  Line " << e.line_number << " | " << e.field_name << " | "
           << e.message << "\n";
    }
  }
  fout << '\n';
  fout << "Warnings [" << m_warnings.size() << "]" << '\n';
  for (const auto &[_, wlist] : m_warnings) {
    for (const auto &w : wlist) {
      fout << "  Line " << w.line_number << " | " << w.field_name << " = "
           << w.actual_value << " (allowed: " << w.min_allowed << " to "
           << w.max_allowed << ")\n";
    }
  }
}

void ReportGenerator::generateMetrics() {
  std::string fileName{m_path + "_metrics.txt"};
  std::ofstream fout{fileName};
  if (!fout) {
    throw std::runtime_error("error: could not create file: " + fileName);
  }

  fout << std::fixed << std::setprecision(2);
  fout << "=== Flight Metrics ===" << '\n' << '\n';

  // Flight duration
  if (!m_data.empty()) {
    fout << "Flight Duration: "
         << (m_data.back().time_s - m_data.front().time_s) << " s" << '\n'
         << '\n';
  }

  // min, max, avg for a metric vector
  struct Stats {
    double min;
    double max;
    double avg;
  };

  auto computeStats = [](const std::vector<double> &v) -> Stats {
    double min{*std::min_element(v.begin(), v.end())};
    double max{*std::max_element(v.begin(), v.end())};
    double sum{0.0};
    for (const auto &val : v) {
      sum += val;
    }
    double avg{sum / static_cast<double>(v.size())};
    return Stats{min, max, avg};
  };

  if (!m_metrics.climb_rate.empty()) {
    Stats s{computeStats(m_metrics.climb_rate)};
    fout << "Climb Rate (ft/min):" << '\n';
    fout << "  Min: " << s.min << "  Max: " << s.max << "  Avg: " << s.avg
         << '\n' << '\n';
  }

  if (!m_metrics.true_airspeed.empty()) {
    Stats s{computeStats(m_metrics.true_airspeed)};
    fout << "True Airspeed (kts):" << '\n';
    fout << "  Min: " << s.min << "  Max: " << s.max << "  Avg: " << s.avg
         << '\n' << '\n';
  }

  if (!m_metrics.mach_number.empty()) {
    Stats s{computeStats(m_metrics.mach_number)};
    fout << "Mach Number:" << '\n';
    fout << "  Min: " << s.min << "  Max: " << s.max << "  Avg: " << s.avg
         << '\n' << '\n';
  }

  if (!m_metrics.density_altitude.empty()) {
    Stats s{computeStats(m_metrics.density_altitude)};
    fout << "Density Altitude (ft):" << '\n';
    fout << "  Min: " << s.min << "  Max: " << s.max << "  Avg: " << s.avg
         << '\n' << '\n';
  }

  if (!m_metrics.segment_distances.empty()) {
    double totalDist{0.0};
    for (const auto &d : m_metrics.segment_distances) {
      totalDist += d;
    }
    fout << "Total Distance Flown: " << totalDist << " nm" << '\n';
  }
}
