# flightparse

A C++ command-line tool that ingests Flight Data Recorder (FDR) telemetry in CSV format, validates parameter ranges per **14 CFR § 121.344 / Appendix B to Part 121**, computes derived flight metrics, detects anomalies, and generates structured reports.

## Overview

Commercial aircraft are required by FAA regulation to record a minimum set of flight parameters on their FDR. flightparse processes this telemetry data by validating each parameter against the ranges specified in Appendix B to Part 121, computing metrics such as true airspeed and Mach number from raw sensor readings, and flagging anomalies like hard landings or excessive bank angles.

## Features

- **CSV ingestion** — Parses 17-field FDR telemetry records with structured error handling for malformed or missing data
- **Range validation** — Checks each parameter against 14 CFR Appendix B limits (e.g. vertical acceleration: -3g to +6g)
- **Anomaly detection** — Flags hard landings, excessive bank, overspeed, pitch exceedances, negative-g events, and high descent rates
- **True airspeed** — Computed from IAS using ISA barometric density model with OAT correction
- **Mach number** — Derived from TAS and local speed of sound
- **Density altitude** — FAA rule-of-thumb method (PA + 120 × OAT deviation from ISA)
- **Climb rate** — Computed from pressure altitude deltas between samples
- **Haversine distance** — Great-circle segment distances from GPS coordinates
- **Multi-format output** — Summary report, flight metrics, and enriched CSV

## Output

flightparse generates three files in `output/`:

| File | Contents |
|------|----------|
| `report_summary.txt` | Anomalies, parse errors, and validation warnings |
| `report_metrics.txt` | Computed flight metrics with min/max/avg statistics |
| `report_data.csv` | Cleaned and validated telemetry data |

Sample summary output:

```
Anomalies [2]
  t=66s | SevereHardLanding | vert_accel_g = 6.5 (threshold: 2.7)
  t=122s | ExcessiveBank | roll_att_deg = -185 (threshold: 35)

Errors [3]
  Line 103 | long_accel_g | Missing field value
  Line 123 | oat_c | Non-numeric value: 'FAULT'
  Line 190 | time_s | Missing field value

Warnings [2]
  Line 131 | vert_accel_g = 6.5 (allowed: -3 to 6)
  Line 241 | roll_att_deg = -185 (allowed: -180 to 180)
```

## Build & Run

**Prerequisites:** C++17 compiler, CMake 3.10+

```bash
cmake -S . -B build
cmake --build build
./build/flightparse data/sample_flight.csv
```

## Project Structure

```
flightparse/
  src/
    main.cpp                  Entry point and pipeline orchestration
    fdr_parser.h / .cpp       CSV parsing with error collection
    validator.h / .cpp        Parameter range validation
    analyzer.h / .cpp         Metric computation and anomaly detection
    report_generator.h / .cpp Multi-format report output
    models/
      FDRRecord.h             Telemetry record struct (17 fields)
      FieldMetaData.h         Column definitions and validation ranges
      Anomaly.h               Anomaly types and detection results
      ReportMetrics.h         Computed metric vectors
      ValidationError.h       Parse-time error (row rejected)
      ValidationWarning.h     Out-of-range warning (row kept)
      ValidationIssue.h       Shared diagnostic fields
  data/
    sample_flight.csv         268-row simulated flight with seeded errors
  CMakeLists.txt
```

## Regulatory Basis

**14 CFR § 121.344** requires transport-category aircraft to install a Flight Data Recorder capable of recording a minimum set of parameters. **Appendix B to Part 121** specifies the range, resolution, and sampling rate for each parameter. flightparse validates against these ranges — for example, pressure altitude must fall between -1,000 ft and 50,000 ft, and vertical acceleration between -3g and +6g. Out-of-range values are flagged as warnings; structurally malformed records are rejected as errors.
