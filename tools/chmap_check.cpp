// chmap-check: checks a detector definition and a channel map (format v0).
//
//   chmap-check detectors.csv map.csv [--expand expanded.csv]
//
// Prints every problem with its file and line. Exit status 0 when there is
// no error (warnings allowed), 1 otherwise. --expand writes the map with
// one line per FE channel, for checking the wiring by eye or in a spreadsheet.

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "detector_map.hpp"

int main(int argc, char* argv[]) {
  if (argc != 3 && !(argc == 5 && std::string(argv[3]) == "--expand")) {
    std::cerr << "usage: " << argv[0] << " detectors.csv map.csv [--expand expanded.csv]\n";
    return 2;
  }
  chmap::DetectorMap map;
  const bool ok = map.load(argv[1], argv[2]);

  int n_error = 0, n_warning = 0;
  for (const auto& d : map.diagnostics()) {
    const bool is_error = d.level == chmap::Diagnostic::Level::kError;
    (is_error ? n_error : n_warning)++;
    std::cout << d.file;
    if (d.line > 0) std::cout << ":" << d.line;
    std::cout << (is_error ? ": error: " : ": warning: ") << d.message << "\n";
  }

  if (ok) {
    std::cout << "\n" << map.entries().size() << " FE channels mapped to "
              << map.devices().size() << " detectors\n";
    for (std::size_t i = 0; i < map.devices().size(); ++i) {
      const auto& d = map.devices()[i];
      std::size_t n = 0;
      for (const auto& e : map.entries()) n += (e.address.device == i);
      std::cout << "  " << d.name << ": " << n << " / " << d.size() << "\n";
    }
    if (argc == 5) {
      std::ofstream out(argv[4]);
      out << "fe,fe.ch,device,address,line\n";
      for (const auto& e : map.entries()) {
        out << chmap::DetectorMap::ip_to_string(e.fe_ip) << "," << e.fe_ch << ","
            << map.devices()[e.address.device].name << "," << map.describe(e.address) << ","
            << e.line << "\n";
      }
      std::cout << "expanded map written to " << argv[4] << "\n";
    }
  }
  std::cout << (ok ? "OK" : "NG") << ": " << n_error << " errors, " << n_warning << " warnings\n";
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
