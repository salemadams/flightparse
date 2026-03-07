#include "analyzer.h"
#include "fdr_parser.h"
#include "report_generator.h"
#include "validator.h"
#include <filesystem>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: flightparse <path-to-csv>\n";
    return 1;
  }
  try {
    std::filesystem::create_directories("output");
    FdrParser fdr_parser(argv[1]);
    Validator validator(fdr_parser.getCSVData());
    Analyzer analyzer(fdr_parser.getCSVData());
    ReportGenerator generator("output/report", analyzer.getReportMetrics(),
                              fdr_parser.getCSVData(), fdr_parser.getErrors(),
                              validator.getWarnings(), analyzer.getAnomalies());
    std::cout << "Parsed " << fdr_parser.getCSVData().size() << " records ("
              << fdr_parser.getErrors().size() << " rejected)\n";
    std::cout << "Report written to output/\n";
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
  return 0;
}
