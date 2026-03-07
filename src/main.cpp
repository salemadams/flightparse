#include "analyzer.h"
#include "fdr_parser.h"
#include "validator.h"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: flightparse <path-to-csv>\n";
    return 1;
  }
  try {
    FdrParser fdr_parser(argv[1]);
    Validator validator(fdr_parser.getCSVData());
    Analyzer analyzer(fdr_parser.getCSVData());
    std::cout << "Parsed " << fdr_parser.getCSVData().size() << " records ("
              << fdr_parser.getErrors().size() << " rejected, "
              << validator.getWarnings().size() << " warnings, "
              << analyzer.getAnomalies().size() << " anomalies)\n";
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
  return 0;
}
