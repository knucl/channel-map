#include "detector_map.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <map>
#include <sstream>

namespace chmap {

namespace {

struct Line {
  int number;
  std::string text;
};

std::string trim(const std::string& s) {
  const char* ws = " \t\r\n";
  const auto b = s.find_first_not_of(ws);
  if (b == std::string::npos) return "";
  const auto e = s.find_last_not_of(ws);
  return s.substr(b, e - b + 1);
}

std::string lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

// Lines that are neither blank nor comments ("#" first), with the UTF-8 BOM
// that Excel writes and Windows line endings removed.
bool read_lines(const std::string& path, std::vector<Line>& lines) {
  std::ifstream file(path);
  if (!file.is_open()) return false;
  std::string text;
  int number = 0;
  while (std::getline(file, text)) {
    ++number;
    if (number == 1 && text.rfind("\xEF\xBB\xBF", 0) == 0) text.erase(0, 3);
    if (!text.empty() && text.back() == '\r') text.pop_back();
    const auto t = trim(text);
    if (t.empty() || t[0] == '#') continue;
    lines.push_back({number, text});
  }
  return true;
}

std::vector<std::string> split_csv(const std::string& line) {
  std::vector<std::string> cells;
  std::string cell;
  std::istringstream iss(line);
  while (std::getline(iss, cell, ',')) {
    cell = trim(cell);
    if (cell.size() >= 2 && cell.front() == '"' && cell.back() == '"')
      cell = trim(cell.substr(1, cell.size() - 2));
    cells.push_back(cell);
  }
  if (!line.empty() && line.back() == ',') cells.push_back("");
  return cells;
}

bool parse_uint(const std::string& s, unsigned long& value) {
  if (s.empty()) return false;
  auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
  return ec == std::errc() && p == s.data() + s.size();
}

// "-" | "a" | "a/b/c" | "0-15" | "15-0" | "0-3/8"  ->  list of values.
bool expand_field(const std::string& text, std::vector<std::string>& out,
                  std::string& why) {
  out.clear();
  if (text.empty()) { why = "empty field (write - when it is not used)"; return false; }
  if (text == "-") { out.push_back("-"); return true; }
  std::istringstream iss(text);
  std::string part;
  while (std::getline(iss, part, '/')) {
    part = trim(part);
    if (part.empty() || part == "-") {
      why = "bad list '" + text + "'";
      return false;
    }
    const auto dash = part.find('-');
    unsigned long lo = 0, hi = 0;
    if (dash != std::string::npos && dash > 0 &&
        parse_uint(part.substr(0, dash), lo) &&
        parse_uint(part.substr(dash + 1), hi)) {
      if (lo <= hi) for (auto v = lo; v <= hi; ++v) out.push_back(std::to_string(v));
      else          for (auto v = lo; v + 1 > hi; --v) out.push_back(std::to_string(v));
    } else {
      out.push_back(part);
    }
  }
  return true;
}

bool all_numeric(const std::vector<std::string>& v) {
  unsigned long x;
  return std::all_of(v.begin(), v.end(),
                     [&](const std::string& s) { return parse_uint(s, x); });
}

// "0-15", "U/D", "0-3/8" ... a short form of a value list, for messages.
std::string summarize(const std::vector<std::string>& v) {
  if (v.empty()) return "";
  std::string out;
  unsigned long a, b;
  for (std::size_t i = 0; i < v.size();) {
    std::size_t j = i;
    if (parse_uint(v[i], a)) {
      while (j + 1 < v.size() && parse_uint(v[j + 1], b) && b == a + (j + 1 - i)) ++j;
    }
    if (!out.empty()) out += "/";
    out += (j > i + 1) ? v[i] + "-" + v[j] : v[i];
    if (j == i + 1) out += "/" + v[j];
    i = j + 1;
  }
  return out;
}

// Index of value in list. A plain number also selects a named entry
// ("0"/"1" for "U"/"D") when the list itself holds no numbers.
bool resolve(const std::string& value, const std::vector<std::string>& list,
             uint16_t& index) {
  auto it = std::find(list.begin(), list.end(), value);
  if (it != list.end()) { index = static_cast<uint16_t>(it - list.begin()); return true; }
  unsigned long n;
  if (!all_numeric(list) && parse_uint(value, n) && n < list.size()) {
    index = static_cast<uint16_t>(n);
    return true;
  }
  return false;
}

std::map<std::string, int> read_header(const std::vector<std::string>& cells) {
  std::map<std::string, int> col;
  for (int i = 0; i < static_cast<int>(cells.size()); ++i) col[lower(cells[i])] = i;
  return col;
}

} // namespace

// _____________________________________________________________________________
void DetectorMap::clear() {
  m_devices.clear();
  m_entries.clear();
  m_diagnostics.clear();
  m_index.clear();
}

bool DetectorMap::has_errors() const {
  return std::any_of(m_diagnostics.begin(), m_diagnostics.end(),
                     [](const Diagnostic& d) { return d.level == Diagnostic::Level::kError; });
}

void DetectorMap::error(const std::string& file, int line, const std::string& msg) {
  m_diagnostics.push_back({Diagnostic::Level::kError, file, line, msg});
}

void DetectorMap::warning(const std::string& file, int line, const std::string& msg) {
  m_diagnostics.push_back({Diagnostic::Level::kWarning, file, line, msg});
}

bool DetectorMap::load(const std::string& definition_path, const std::string& map_path) {
  clear();
  read_definition(definition_path);
  if (has_errors()) {
    error(map_path, 0, "not read, because the definition file has errors");
  } else {
    read_map(map_path);
    if (!has_errors()) report_unmapped(map_path);
  }
  if (has_errors()) {
    auto diagnostics = std::move(m_diagnostics);
    clear();
    m_diagnostics = std::move(diagnostics);
    return false;
  }
  return true;
}

// _____________________________________________________________________________
std::string DetectorMap::ip_to_string(uint32_t ip) {
  return std::to_string(ip >> 24) + "." + std::to_string((ip >> 16) & 0xff) + "." +
         std::to_string((ip >> 8) & 0xff) + "." + std::to_string(ip & 0xff);
}

bool DetectorMap::parse_ip(const std::string& text, uint32_t& ip) {
  const auto t = trim(text);
  if (t.rfind("0x", 0) == 0 || t.rfind("0X", 0) == 0) {
    unsigned long v = 0;
    auto [p, ec] = std::from_chars(t.data() + 2, t.data() + t.size(), v, 16);
    if (ec != std::errc() || p != t.data() + t.size() || v > 0xffffffffUL) return false;
    ip = static_cast<uint32_t>(v);
    return true;
  }
  std::vector<unsigned long> octets;
  std::istringstream iss(t);
  std::string part;
  while (std::getline(iss, part, '.')) {
    unsigned long v;
    if (!parse_uint(part, v) || v > 255) return false;
    octets.push_back(v);
  }
  if (octets.size() == 2) octets.insert(octets.begin(), {192, 168});
  if (octets.size() != 4) return false;
  ip = static_cast<uint32_t>((octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3]);
  return true;
}

std::string DetectorMap::describe(const Address& a) const {
  if (a.device >= m_devices.size()) return "?";
  const auto& d = m_devices[a.device];
  std::string s = d.name;
  if (d.planes.size()   > 1 || d.planes[0]   != "-") s += " plane=" + d.planes[a.plane];
  if (d.segments.size() > 1 || d.segments[0] != "-") s += " seg="   + d.segments[a.segment];
  if (d.chs.size()      > 1 || d.chs[0]      != "-") s += " ch="    + d.chs[a.ch];
  if (d.datas.size()    > 1)                         s += " data="  + d.datas[a.data];
  return s;
}

// _____________________________________________________________________________
void DetectorMap::read_definition(const std::string& path) {
  std::vector<Line> lines;
  if (!read_lines(path, lines)) { error(path, 0, "cannot open the file"); return; }
  if (lines.empty())            { error(path, 0, "the file has no header line"); return; }

  auto col = read_header(split_csv(lines[0].text));
  for (const char* name : {"device", "plane", "segment", "ch"}) {
    if (!col.count(name)) {
      error(path, lines[0].number, std::string("column '") + name + "' is missing in the header");
    }
  }
  if (has_errors()) return;
  const bool has_data = col.count("data") > 0;

  for (std::size_t i = 1; i < lines.size(); ++i) {
    const int ln = lines[i].number;
    const auto cells = split_csv(lines[i].text);
    auto cell = [&](const char* name) -> std::string {
      const int c = col[name];
      return c < static_cast<int>(cells.size()) ? cells[c] : "";
    };
    DeviceDef d;
    d.name = cell("device");
    d.line = ln;
    if (d.name.empty() || d.name == "-") { error(path, ln, "device name is empty"); continue; }
    for (const auto& other : m_devices) {
      if (other.name == d.name) {
        error(path, ln, "device '" + d.name + "' is already defined at line " +
                        std::to_string(other.line));
      }
    }
    std::string why;
    bool ok = true;
    for (auto [name, list] : {std::pair{"plane", &d.planes}, std::pair{"segment", &d.segments},
                              std::pair{"ch", &d.chs}}) {
      if (!expand_field(cell(name), *list, why)) {
        error(path, ln, std::string(name) + ": " + why);
        ok = false;
      }
    }
    const std::string data = has_data ? cell("data") : "";
    if (data.empty() || data == "-") {
      d.datas = {"tdc"};
    } else if (!expand_field(data, d.datas, why)) {
      error(path, ln, "data: " + why);
      ok = false;
    }
    if (!ok) continue;
    for (auto [name, list] : {std::pair{"plane", &d.planes}, std::pair{"segment", &d.segments},
                              std::pair{"ch", &d.chs}, std::pair{"data", &d.datas}}) {
      auto sorted = *list;
      std::sort(sorted.begin(), sorted.end());
      auto dup = std::adjacent_find(sorted.begin(), sorted.end());
      if (dup != sorted.end()) {
        error(path, ln, std::string(name) + ": '" + *dup + "' appears twice");
        ok = false;
      }
    }
    if (ok) m_devices.push_back(std::move(d));
  }
  if (m_devices.empty() && !has_errors()) error(path, 0, "no detector is defined");
}

// _____________________________________________________________________________
void DetectorMap::read_map(const std::string& path) {
  std::vector<Line> lines;
  if (!read_lines(path, lines)) { error(path, 0, "cannot open the file"); return; }
  if (lines.empty())            { error(path, 0, "the file has no header line"); return; }

  auto col = read_header(split_csv(lines[0].text));
  for (const char* name : {"fe", "fe.ch", "device", "plane", "segment", "ch"}) {
    if (!col.count(name)) {
      error(path, lines[0].number, std::string("column '") + name + "' is missing in the header");
    }
  }
  if (has_errors()) return;
  const bool has_data = col.count("data") > 0;

  // packed detector address -> map line, to find two FE channels on one address
  std::unordered_map<uint64_t, int> used_address;
  auto pack = [](const Address& a) {
    return (static_cast<uint64_t>(a.device) << 48) | (static_cast<uint64_t>(a.plane) << 36) |
           (static_cast<uint64_t>(a.segment) << 24) | (static_cast<uint64_t>(a.ch) << 12) |
           static_cast<uint64_t>(a.data);
  };

  for (std::size_t i = 1; i < lines.size(); ++i) {
    const int ln = lines[i].number;
    const auto cells = split_csv(lines[i].text);
    auto cell = [&](const char* name) -> std::string {
      const int c = col[name];
      return c < static_cast<int>(cells.size()) ? cells[c] : "";
    };
    std::string why;

    uint32_t ip = 0;
    if (!parse_ip(cell("fe"), ip)) {
      error(path, ln, "fe: '" + cell("fe") + "' is not an IP address (write 2.169 or 192.168.2.169)");
      continue;
    }
    std::vector<std::string> fe_chs;
    if (!expand_field(cell("fe.ch"), fe_chs, why) || fe_chs[0] == "-") {
      error(path, ln, "fe.ch: " + (why.empty() ? "a channel number is needed" : why));
      continue;
    }
    std::vector<unsigned long> fe_ch_numbers;
    bool ok = true;
    for (const auto& s : fe_chs) {
      unsigned long v;
      if (!parse_uint(s, v)) { error(path, ln, "fe.ch: '" + s + "' is not a number"); ok = false; break; }
      fe_ch_numbers.push_back(v);
    }
    if (!ok) continue;
    const std::size_t n = fe_ch_numbers.size();

    const std::string device_name = cell("device");
    auto dev = std::find_if(m_devices.begin(), m_devices.end(),
                            [&](const DeviceDef& d) { return d.name == device_name; });
    if (dev == m_devices.end()) {
      std::string hint;
      for (const auto& d : m_devices)
        if (lower(d.name) == lower(device_name)) hint = " (did you mean '" + d.name + "'?)";
      error(path, ln, "device '" + device_name + "' is not in the definition file" + hint);
      continue;
    }

    // each detector field: one value for all channels, or exactly n values
    struct Field { const char* name; const std::vector<std::string>* defined; std::vector<uint16_t> index; };
    std::vector<Field> fields = {{"plane", &dev->planes, {}}, {"segment", &dev->segments, {}},
                                 {"ch", &dev->chs, {}}, {"data", &dev->datas, {}}};
    for (auto& f : fields) {
      std::string text = (std::string(f.name) == "data" && !has_data) ? "" : cell(f.name);
      std::vector<std::string> values;
      if (std::string(f.name) == "data" && (text.empty() || text == "-")) {
        values = {f.defined->front()};
      } else if (!expand_field(text, values, why)) {
        error(path, ln, std::string(f.name) + ": " + why);
        ok = false;
        continue;
      }
      if (values.size() != 1 && values.size() != n) {
        error(path, ln, std::string(f.name) + ": " + std::to_string(values.size()) +
                        " values for " + std::to_string(n) + " FE channels (give 1 or " +
                        std::to_string(n) + ")");
        ok = false;
        continue;
      }
      for (const auto& v : values) {
        uint16_t idx;
        if (!resolve(v, *f.defined, idx)) {
          error(path, ln, std::string(f.name) + " '" + v + "' is not defined for " + dev->name +
                          " (defined: " + summarize(*f.defined) + ")");
          ok = false;
          break;
        }
        f.index.push_back(idx);
      }
    }
    if (!ok) continue;

    const auto device_index = static_cast<uint16_t>(dev - m_devices.begin());
    for (std::size_t k = 0; k < n; ++k) {
      auto pick = [&](const Field& f) { return f.index.size() == 1 ? f.index[0] : f.index[k]; };
      Entry e;
      e.fe_ip = ip;
      e.fe_ch = static_cast<uint32_t>(fe_ch_numbers[k]);
      e.address = {device_index, pick(fields[0]), pick(fields[1]), pick(fields[2]), pick(fields[3])};
      e.line = ln;

      const auto fe_key = key(e.fe_ip, e.fe_ch);
      if (auto it = m_index.find(fe_key); it != m_index.end()) {
        const auto& first = m_entries[it->second];
        error(path, ln, "FE " + ip_to_string(ip) + " ch " + std::to_string(e.fe_ch) +
                        " is already mapped at line " + std::to_string(first.line) +
                        " (to " + describe(first.address) + ")");
        continue;
      }
      if (auto it = used_address.find(pack(e.address)); it != used_address.end()) {
        error(path, ln, describe(e.address) + " is already wired at line " +
                        std::to_string(it->second) + " (FE " + ip_to_string(ip) + " ch " +
                        std::to_string(e.fe_ch) + " would be a second channel on it)");
        continue;
      }
      used_address[pack(e.address)] = ln;
      m_index[fe_key] = m_entries.size();
      m_entries.push_back(e);
    }
  }
  if (m_entries.empty() && !has_errors()) error(path, 0, "no channel is mapped");
}

// _____________________________________________________________________________
void DetectorMap::report_unmapped(const std::string& path) {
  std::vector<std::vector<bool>> mapped(m_devices.size());
  for (std::size_t i = 0; i < m_devices.size(); ++i) mapped[i].assign(m_devices[i].size(), false);
  auto flat = [&](const Address& a) {
    const auto& d = m_devices[a.device];
    return ((a.plane * d.segments.size() + a.segment) * d.chs.size() + a.ch) * d.datas.size() + a.data;
  };
  for (const auto& e : m_entries) mapped[e.address.device][flat(e.address)] = true;

  for (std::size_t i = 0; i < m_devices.size(); ++i) {
    const auto& d = m_devices[i];
    std::vector<std::string> missing;
    std::size_t count = 0;
    for (uint16_t p = 0; p < d.planes.size(); ++p)
      for (uint16_t s = 0; s < d.segments.size(); ++s)
        for (uint16_t c = 0; c < d.chs.size(); ++c)
          for (uint16_t x = 0; x < d.datas.size(); ++x) {
            Address a{static_cast<uint16_t>(i), p, s, c, x};
            if (mapped[i][flat(a)]) continue;
            if (++count <= 6) missing.push_back(describe(a));
          }
    if (count == 0) continue;
    std::string msg = d.name + ": " + std::to_string(count) + " of " + std::to_string(d.size()) +
                      " defined channels are not in the map (";
    for (std::size_t k = 0; k < missing.size(); ++k) msg += (k ? ", " : "") + missing[k];
    msg += count > missing.size() ? ", ...)" : ")";
    warning(path, 0, msg);
  }
}

} // namespace chmap
