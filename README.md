# channel-map

Channel mapping library between Detector and Readout FEE

Development and test environment: KEKCC Intel Xeon Gold 6230 20C40Tx2 @2.10 GHz, CentOS Linux 7.9.2009, gcc/g++ 8.3.0 (c++17 required)

## Install

Build channel-map.

```sh
cmake -S . -B .build
cmake --build .build --target install
```

Add the following to your project's CMakeLists.txt.

```sh
list(APPEND CMAKE_PREFIX_PATH /path/to/channel-map)
find_package(ChannelMap REQUIRED)
target_link_libraries(YourProject ChannelMap::ChannelMap)
```

For Makefile, the following.

```make
chmap_config	= /path/to/channel-map/bin/chmap-config
CFLAGS			+= $(shell $(chmap_config) --include)
LDFLAGS			+= $(shell $(chmap_config) --libs)
```

## Uninstall

```sh
xargs rm < .build/install_manifest.txt
rm -rf .build
```

## Parameter format

Only CSV is supported.
Write the element title of each column in the heading line.
Titles are classified into *Readout FEE* or *Detector* tuples using the first dot-delimited string.

Elements can be numbers or strings.
Prefixes such as `0xff`, `0b101`, `077`, etc. are interpreted as hexadecimal, binary, or octal numbers, respectively.
If they cannot be interpreted as numbers, they are assumed to be strings.

```test.csv
fe.id,fe.channel,fe.data,detector.id,detector.plane,detector.segment,detector.channel,detector.data
30,262,adc,170,30,0,262,0
30,263,0,170,30,0,263,0
30,264,0,170,30,0,264,0
30,265,0,170,30,0,265,0
30,266,0,170,30,0,266,0
30,267,0,170,30,0,267,0
30,268,0,170,30,0,268,0
30,269,0,170,30,0,269,0xff
30,270,0,170,30,0,270,A
30,271,0,170,30,0,271,C
```

## Usage

Initialize the singleton instance of the `ChannelMap` class
and use the `get` method to get a tuple from *FE* to *Detector* or the other way around.

`std::get<T>` returns the value if the specified type T matches the current type of `std::variant`,
otherwise it throws a `std::bad_variant_access` exception.

`std::get_if<T>` returns a pointer to the value if the specified type T
matches the current type of `std::variant`, nullptr otherwise.

```cpp
#include <channel_map.hpp>

void
your_function()
{
  auto& channel_map = chmap::ChannelMap::get_instance();
  channel_map.initialize("foo.csv");

  // std::get<T>
  {
    chmap::ChannelTuple detector(1, 2, 3, 4, 5);
    const auto& fe = channel_map.get("fe", detector);
    try {
      auto fe_id = std::get<chmap::number_t>(fe["id"]);
      auto fe_ch = std::get<chmap::number_t>(fe["channel"]);
    } catch (const std::bad_variant_access& e) {
      std::cerr << "Bad variand access : " << e << std::endl;
    }
  }

  // std::get_if<T>
  {
    const auto& fe = channel_map.get("fe", {0, "a", 3, "x"});
    if (auto value_as_pointer = std::get_if<chmap::number_t>(&fe[0])) {
      std::cout << "The value is number : *value_as_pointer << std::endl;
    } else if (auto fe_id_as_pointer = std::get_if<std::string>(&fe[0])) {
      std::cout << "The value is string : *value_as_pointer << std::endl;
    } else {
      std::cerr << "The variant does not hold number or string" << std::endl;
    }
  }
    ...
  }
}
```

See also `skeleton.cpp` for implementation details.

## Format v0: detector definition + channel map

`chmap::DetectorMap` (`detector_map.hpp`) reads two CSV files.
Detectors are written by name and FE boards by IP address; nothing is hard-coded in the source.

`detectors.csv` (changed only when a detector is added):

```
device, plane,    segment, ch,   data
TOF,    -,        0-15,    U/D,  tdc
SDC,    X/Xp/Y/Yp, -,      0-31, tdc
```

`map.csv` (edited when cables move):

```
fe,    fe.ch,  device, plane, segment, ch
2.169, 4-19,   TOF,    -,     0-15,    U
2.170, 32-63,  SDC,    Xp,    -,       31-0
```

- A field is one value, `-` (not used), a list `U/D`, a range `0-15` (descending `15-0` for reversed wiring) or a mix `0-3/8`.
- In a map row `fe.ch` may hold N channels; each detector field holds 1 value (used for all) or exactly N values.
- `fe` is `2.169` (= 192.168.2.169), `192.168.2.169` or `0xc0a802a9`. `data` is optional (default: the first data of the device, `tdc`).
- Blank lines and lines starting with `#` are ignored. Excel CSV (BOM, CRLF) is accepted.

Check before use:

```sh
chmap-check detectors.csv map.csv [--expand expanded.csv]
```

Every unknown name, undefined segment/channel, FE channel mapped twice, detector channel wired twice and count mismatch is reported with its line; channels defined but not mapped are warnings. `load()` returns false on any error and leaves the object empty, so a running program can load into a new object and swap it in only on success. See `examples/`.
