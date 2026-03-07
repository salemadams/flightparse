#include "analyzer.h"
#include "models/Anomaly.h"
#include "models/FDRRecord.h"
#include <cmath>
#include <vector>

// ── Mathematical constant ───────────────────────────────────────────────────
constexpr double kPi = 3.14159265358979323846;
constexpr double kDegreesToRadians = kPi / 180.0;

// ── Unit conversion factors ─────────────────────────────────────────────────
constexpr double kFeetToMeters = 0.3048;
constexpr double kMpsToKnots = 1.94384;
constexpr double kCelsiusToKelvinOffset = 273.15;
constexpr double kSecondsPerMinute = 60.0;

// ── ISA constants (ICAO Doc 7488 / ISO 2533) ───────────────────────────────
constexpr double kIsaSeaLevelTempK = 288.15;    // Sea-level temperature (K)
constexpr double kIsaLapseRate = 0.0065;         // Troposphere lapse rate (K/m)
constexpr double kTropopauseAltM = 11000.0;      // Tropopause altitude (m)
constexpr double kTropopauseTempK = 216.65;      // Temperature at tropopause (K)
constexpr double kSeaLevelDensity = 1.225;       // Sea-level air density (kg/m^3)
constexpr double kGravity = 9.80665;             // Standard gravitational accel (m/s^2)

// ── Gas constants for dry air ───────────────────────────────────────────────
constexpr double kGasConstantDryAir = 287.05;    // Specific gas constant R (J/(kg*K))
constexpr double kHeatCapacityRatio = 1.4;       // Ratio of specific heats (gamma)

// ── Density altitude rule-of-thumb (FAA PHAK Ch. 4) ────────────────────────
constexpr double kIsaSeaLevelTempC = 15.0;              // ISA sea-level temp (C)
constexpr double kIsaLapseRateCPer1000Ft = 2.0;         // Lapse rate (C per 1000 ft)
constexpr double kDensityAltCorrectionFtPerC = 120.0;   // Correction (ft per C)

// ── Haversine distance ──────────────────────────────────────────────────────
constexpr double kEarthRadiusNm = 3440.065;      // Mean Earth radius (nm)

// ── Anomaly detection thresholds ────────────────────────────────────────────
constexpr double kSevereHardLandingG = 2.7;
constexpr double kHardLandingG = 2.0;
constexpr double kNegativeGThreshold = 0.0;
constexpr double kExcessiveBankDeg = 35.0;
constexpr double kOverspeedKts = 350.0;
constexpr double kPitchUpLimitDeg = 25.0;
constexpr double kPitchDownLimitDeg = -15.0;
constexpr double kHighDescentRateFpm = -1000.0;

Analyzer::Analyzer(const std::vector<FDRRecord> &data) : m_data{data} {
  computeMetrics();
}

void Analyzer::computeMetrics() {
  computeClimbRate();
  computeTrueAirspeed();
  computeDensityAltitude();
  computeMachNumber();
  computeSegmentDistances();
  detectAnomalies();
}

void Analyzer::detectAnomalies() {
  for (const auto &row : m_data) {
    if (row.vert_accel_g > kSevereHardLandingG) {
      m_anomalies.push_back(
          Anomaly{row.time_s, AnomalyType::SevereHardLanding, "vert_accel_g",
                  row.vert_accel_g, kSevereHardLandingG});
    } else if (row.vert_accel_g > kHardLandingG) {
      m_anomalies.push_back(
          Anomaly{row.time_s, AnomalyType::HardLanding, "vert_accel_g",
                  row.vert_accel_g, kHardLandingG});
    }

    if (row.vert_accel_g < kNegativeGThreshold) {
      m_anomalies.push_back(
          Anomaly{row.time_s, AnomalyType::NegativeG, "vert_accel_g",
                  row.vert_accel_g, kNegativeGThreshold});
    }

    if (std::abs(row.roll_att_deg) > kExcessiveBankDeg) {
      m_anomalies.push_back(
          Anomaly{row.time_s, AnomalyType::ExcessiveBank, "roll_att_deg",
                  row.roll_att_deg, kExcessiveBankDeg});
    }

    if (row.indicated_airspeed_kts > kOverspeedKts) {
      m_anomalies.push_back(
          Anomaly{row.time_s, AnomalyType::Overspeed, "indicated_airspeed_kts",
                  row.indicated_airspeed_kts, kOverspeedKts});
    }

    if (row.pitch_att_deg > kPitchUpLimitDeg) {
      m_anomalies.push_back(
          Anomaly{row.time_s, AnomalyType::PitchUp, "pitch_att_deg",
                  row.pitch_att_deg, kPitchUpLimitDeg});
    } else if (row.pitch_att_deg < kPitchDownLimitDeg) {
      m_anomalies.push_back(
          Anomaly{row.time_s, AnomalyType::PitchDown, "pitch_att_deg",
                  row.pitch_att_deg, kPitchDownLimitDeg});
    }
  }

  for (int i = 0; i < static_cast<int>(m_reportMetrics.climb_rate.size());
       i++) {
    if (m_reportMetrics.climb_rate[i] < kHighDescentRateFpm) {
      m_anomalies.push_back(
          Anomaly{m_data[i + 1].time_s, AnomalyType::HighDescentRate,
                  "climb_rate_fpm", m_reportMetrics.climb_rate[i],
                  kHighDescentRateFpm});
    }
  }
}

void Analyzer::computeClimbRate() {
  std::vector<double> climb_rate{};
  for (int i = 1; i < static_cast<int>(m_data.size()); i++) {
    double delta_time{m_data[i].time_s - m_data[i - 1].time_s};
    if (delta_time <= 0) {
      climb_rate.push_back(0.0);
      continue;
    }
    climb_rate.push_back(
        (m_data[i].pressure_alt_ft - m_data[i - 1].pressure_alt_ft) /
        delta_time * kSecondsPerMinute);
  }
  m_reportMetrics.climb_rate = climb_rate;
}

// TAS = IAS * sqrt(rho_0 / rho_actual)
// uses ISA barometric density model, corrected for actual OAT
void Analyzer::computeTrueAirspeed() {
  // Exponent for ISA troposphere density: g / (L * R) - 1
  const double density_exp =
      kGravity / (kIsaLapseRate * kGasConstantDryAir) - 1.0;

  std::vector<double> tas{};
  for (int i = 0; i < static_cast<int>(m_data.size()); i++) {
    double h_m{m_data[i].pressure_alt_ft * kFeetToMeters};

    // ISA temperature at this altitude
    double t_isa{(h_m > kTropopauseAltM)
                     ? kTropopauseTempK
                     : kIsaSeaLevelTempK - kIsaLapseRate * h_m};

    // ISA density at this altitude (barometric formula)
    double rho_isa{};
    if (h_m > kTropopauseAltM) {
      // Stratosphere: isothermal layer, exponential density decay
      double rho_at_tropopause{
          kSeaLevelDensity *
          std::pow(kTropopauseTempK / kIsaSeaLevelTempK, density_exp)};
      rho_isa = rho_at_tropopause *
                std::exp(-kGravity * (h_m - kTropopauseAltM) /
                         (kGasConstantDryAir * kTropopauseTempK));
    } else {
      // Troposphere: power-law density relationship
      rho_isa = kSeaLevelDensity *
                std::pow(t_isa / kIsaSeaLevelTempK, density_exp);
    }

    // Correct ISA density for actual OAT (ideal gas law at same pressure)
    double t_act{m_data[i].oat_c + kCelsiusToKelvinOffset};
    double rho_actual{rho_isa * (t_isa / t_act)};

    tas.push_back(m_data[i].indicated_airspeed_kts *
                  std::sqrt(kSeaLevelDensity / rho_actual));
  }
  m_reportMetrics.true_airspeed = tas;
}

// mach = TAS / speed_of_sound, where a = sqrt(gamma * R * T)
void Analyzer::computeMachNumber() {
  std::vector<double> mach{};
  for (int i = 0; i < static_cast<int>(m_reportMetrics.true_airspeed.size());
       i++) {
    double t_act{m_data[i].oat_c + kCelsiusToKelvinOffset};
    double speed_of_sound_mps{
        std::sqrt(kHeatCapacityRatio * kGasConstantDryAir * t_act)};
    double speed_of_sound_kts{speed_of_sound_mps * kMpsToKnots};
    mach.push_back(m_reportMetrics.true_airspeed[i] / speed_of_sound_kts);
  }
  m_reportMetrics.mach_number = mach;
}

// DA = PA + 120 * (OAT - ISA_temp) — FAA rule of thumb
void Analyzer::computeDensityAltitude() {
  std::vector<double> density_alt{};
  for (int i = 0; i < static_cast<int>(m_data.size()); i++) {
    double t_isa_c{kIsaSeaLevelTempC -
                   kIsaLapseRateCPer1000Ft *
                       (m_data[i].pressure_alt_ft / 1000.0)};
    density_alt.push_back(m_data[i].pressure_alt_ft +
                          kDensityAltCorrectionFtPerC *
                              (m_data[i].oat_c - t_isa_c));
  }
  m_reportMetrics.density_altitude = density_alt;
}

// great-circle distance via Haversine, atan2 form for numerical stability
void Analyzer::computeSegmentDistances() {
  std::vector<double> segment_distances{};
  for (int i = 1; i < static_cast<int>(m_data.size()); i++) {
    double d_lat{(m_data[i].latitude - m_data[i - 1].latitude) *
                 kDegreesToRadians};
    double d_lon{(m_data[i].longitude - m_data[i - 1].longitude) *
                 kDegreesToRadians};
    double lat1_r{m_data[i - 1].latitude * kDegreesToRadians};
    double lat2_r{m_data[i].latitude * kDegreesToRadians};
    double a{std::pow(std::sin(d_lat / 2), 2) +
             std::cos(lat1_r) * std::cos(lat2_r) *
                 std::pow(std::sin(d_lon / 2), 2)};
    double c{2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a))};
    segment_distances.push_back(kEarthRadiusNm * c);
  }
  m_reportMetrics.segment_distances = segment_distances;
}
