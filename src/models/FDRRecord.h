#ifndef FDRRECORD_H
#define FDRRECORD_H

// single FDR telemetry row per 14 CFR 121.344 / Appendix B
struct FDRRecord {
  double time_s;                  // param 1: seconds since recording start
  double pressure_alt_ft;         // param 2: pressure altitude (ft)
  double indicated_airspeed_kts;  // param 3: indicated airspeed (kts)
  double heading_deg;             // param 4: heading (0-360 deg)
  double latitude;                // param 39: latitude (deg)
  double longitude;               // param 39: longitude (deg)
  double ground_speed_kts;        // param 34: ground speed (kts)
  double pitch_att_deg;           // param 6: pitch attitude (deg)
  double roll_att_deg;            // param 7: roll attitude (deg)
  double vert_accel_g;            // param 5: normal acceleration (g)
  double long_accel_g;            // param 11: longitudinal acceleration (g)
  double lat_accel_g;             // param 18: lateral acceleration (g)
  double thrust_pct;              // param 9: thrust per engine (%)
  double oat_c;                   // param 24: outside air temperature (C)
  double pitch_input_pct;         // param 12: pitch control input (%)
  double lat_input_pct;           // param 13: lateral control input (%)
  double rudder_input_pct;        // param 14: rudder pedal input (%)
  int line_number;                // source CSV line (1-indexed)
};
#endif
