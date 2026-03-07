#ifndef REPORTMETRICS_H
#define REPORTMETRICS_H
#include <vector>
// computed flight metrics, one value per row (or per adjacent pair)
struct ReportMetrics {
  std::vector<double> climb_rate;
  std::vector<double> true_airspeed;
  std::vector<double> mach_number;
  std::vector<double> density_altitude;
  std::vector<double> segment_distances;
};
#endif
