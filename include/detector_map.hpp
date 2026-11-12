#ifndef CHANNELMAP_DETECTOR_MAP_HPP_
#define CHANNELMAP_DETECTOR_MAP_HPP_

// Channel map, format v0.
//
// Two CSV files:
//   detector definition : which detectors exist and the shape of each one
//                         (columns: device, plane, segment, ch[, data])
//   channel map         : which FE channel is wired to which detector channel
//                         (columns: fe, fe.ch, device, plane, segment, ch[, data])
//
// A field holds one value, "-" (the field is not used), a list "U/D",
// a numeric range "0-15" (descending "15-0" allowed), or a mix "0-3/8".
// In a map row, fe.ch may hold N values; every detector field then holds
// either one value (used for all N) or exactly N values (taken in order).
//
// Loading never throws. Problems are collected as diagnostics with the file
// and line they come from; load() returns false when there is any error, and
// the object is then left empty. To reload safely, load into a new object
// and swap it in only when load() succeeded.

// Use the default struct packing whatever was set before this header: a
// "#pragma pack" left open by an earlier header would otherwise lay these
// classes (and the standard containers they hold) out differently from the
// compiled library, and load() would crash.
#pragma pack(push)
#pragma pack()

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace chmap {

struct DeviceDef {
  std::string name;
  std::vector<std::string> planes;    // {"-"} when not used
  std::vector<std::string> segments;  // {"-"} when not used
  std::vector<std::string> chs;       // {"-"} when not used
  std::vector<std::string> datas;     // default {"tdc"}
  int line = 0;                       // line in the definition file
  std::size_t size() const {
    return planes.size() * segments.size() * chs.size() * datas.size();
  }
};

// A detector address, as indexes into the lists of its DeviceDef.
struct Address {
  uint16_t device  = 0;
  uint16_t plane   = 0;
  uint16_t segment = 0;
  uint16_t ch      = 0;
  uint16_t data    = 0;
};

struct Entry {
  uint32_t fe_ip = 0;   // full IPv4 address, e.g. 192.168.2.169 = 0xc0a802a9
  uint32_t fe_ch = 0;
  Address  address;
  int      line  = 0;   // line in the map file
};

struct Diagnostic {
  enum class Level { kError, kWarning };
  Level       level;
  std::string file;
  int         line;     // 0 when it is about the whole file
  std::string message;
};

class DetectorMap {
public:
  // Reads both files. Returns true when there is no error (warnings allowed).
  bool load(const std::string& definition_path, const std::string& map_path);

  // Detector address of an FE channel, or nullptr when it is not in the map.
  const Address* find(uint32_t fe_ip, uint32_t fe_ch) const {
    auto it = m_index.find(key(fe_ip, fe_ch));
    return it == m_index.end() ? nullptr : &m_entries[it->second].address;
  }

  const std::vector<DeviceDef>&  devices()     const { return m_devices; }
  const std::vector<Entry>&      entries()     const { return m_entries; }
  const std::vector<Diagnostic>& diagnostics() const { return m_diagnostics; }
  bool  has_errors() const;
  void  clear();

  // "Name/plane/segment/ch/data" with unused fields left out, for messages.
  std::string describe(const Address& a) const;

  static std::string ip_to_string(uint32_t ip);
  // Accepts "2.169" (= 192.168.2.169), "192.168.2.169" or "0xc0a802a9".
  static bool parse_ip(const std::string& text, uint32_t& ip);

private:
  static uint64_t key(uint32_t ip, uint32_t ch) {
    return (static_cast<uint64_t>(ip) << 32) | ch;
  }
  void read_definition(const std::string& path);
  void read_map(const std::string& path);
  void report_unmapped(const std::string& path);
  void error(const std::string& file, int line, const std::string& msg);
  void warning(const std::string& file, int line, const std::string& msg);

  std::vector<DeviceDef>  m_devices;
  std::vector<Entry>      m_entries;
  std::vector<Diagnostic> m_diagnostics;
  std::unordered_map<uint64_t, std::size_t> m_index;  // FE key -> m_entries index
};

} // namespace chmap

#pragma pack(pop)

#endif
