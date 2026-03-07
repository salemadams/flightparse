#ifndef FIELDMETADATA_H
#define FIELDMETADATA_H

#include "FDRRecord.h"
#include <iterator>
#include <string>

// maps a CSV column to its FDRRecord field and validation range
struct FieldMetaData {
  std::string name;
  double FDRRecord::*member;
  double min;
  double max;
  bool validate;
};

// column definitions — order must match CSV header
inline const FieldMetaData fieldMetaData[] = {
    {"time_s",                 &FDRRecord::time_s,                 0,    0,     false},
    {"pressure_alt_ft",        &FDRRecord::pressure_alt_ft,        -1000, 50000, true},
    {"indicated_airspeed_kts", &FDRRecord::indicated_airspeed_kts, 0,    500,   true},
    {"heading_deg",            &FDRRecord::heading_deg,            0,    360,   true},
    {"latitude",               &FDRRecord::latitude,               -90,  90,    true},
    {"longitude",              &FDRRecord::longitude,              -180, 180,   true},
    {"ground_speed_kts",       &FDRRecord::ground_speed_kts,       0,    0,     false},
    {"pitch_att_deg",          &FDRRecord::pitch_att_deg,          -75,  75,    true},
    {"roll_att_deg",           &FDRRecord::roll_att_deg,           -180, 180,   true},
    {"vert_accel_g",           &FDRRecord::vert_accel_g,           -3,   6,     true},
    {"long_accel_g",           &FDRRecord::long_accel_g,           -1,   1,     true},
    {"lat_accel_g",            &FDRRecord::lat_accel_g,            -1,   1,     true},
    {"thrust_pct",             &FDRRecord::thrust_pct,             0,    100,   true},
    {"oat_c",                  &FDRRecord::oat_c,                  -50,  90,    true},
    {"pitch_input_pct",        &FDRRecord::pitch_input_pct,        -100, 100,   true},
    {"lat_input_pct",          &FDRRecord::lat_input_pct,          -100, 100,   true},
    {"rudder_input_pct",       &FDRRecord::rudder_input_pct,       -100, 100,   true},
};

constexpr int kFieldCount{static_cast<int>(std::size(fieldMetaData))};

#endif
