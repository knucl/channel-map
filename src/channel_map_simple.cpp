#include "channel_map_simple.hpp"
#include "channel_map_simple_item.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace chmap {
    void ChannelMapSimple::initialize(const std::string& file_path) {
        simplify_detector_names(); // prepare detname_simplify_map
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "file open fail : " << file_path << std::endl;
            std::exit(1);
        }

        std::string line;
        if (std::getline(file, line)) {
            // load header, as template, assuming "fe.id, fe.channel, fe.data, detector.id, detector.plane, detector.segment, detector.channel, detector.data"
            for(const auto& header_part : split_line(line)) {
                if (std::count(m_header.begin(), m_header.end(), header_part) > 0) {
                    std::cerr << "found duplicate header : " << header_part << std::endl;
                }
                m_header.push_back(header_part);
                auto type = split_line(header_part, '.')[0];
                m_element_type.push_back(type);
                if (std::find(m_unique_types.begin(), m_unique_types.end(), type) == m_unique_types.end()) {
                    m_unique_types.push_back(type);
                }
            }
        }// if getline(file, line) for loeading header
        // load mapdata lines
        while (std::getline(file, line)) {
            if(line.back() == '\r'){ // for Windwos-style line ending
            line.pop_back();
            }

            auto tokens = split_line(line);
            if (tokens.size() != m_header.size()) {
                std::cerr << "bad file format : " << line << std::endl;
                continue;
            }

            
        }

    }// void ChannelMapSimple::initialize
    std::vector<std::string> ChannelMapSimple::split_line(const std::string& line, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(line);
        while (std::getline(iss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }// std::vector<std::string> ChannelMapSimple::split_line
    ChannelMapSimpleItem ChannelMapSimple::makeSimpleItem(const std::vector<std::string>& tokens) {
        int len_tokens = tokens.size();
        uint64_t fe_ip_full;
        uint32_t fe_ip_3rd_4th;
        uint16_t fe_channel;
        uint32_t det_name;
        uint16_t det_plane;
        uint8_t det_segment;
        uint32_t det_channel;
        std::string det_name_str;
        std::string det_plane_str;
        std::string det_channel_str;

        int fe_magic = 3; // id, channel, data
        int fe_count = 0;
        int det_magic = 5; // id, plane, segment, channel, data
        int det_count = 0;
        for(int i=0; i<len_tokens; ++i) {
            std::string type = m_element_type[i];
            if(type == "fe") {
                // parse front-end related tokens
                if(fe_count == 0) {
                    fe_ip_full = static_cast<uint64_t>(std::stoull(tokens[i]));
                    fe_ip_3rd_4th = parse_to32(
                        std::to_string( (fe_ip_full >> 16) & 0xFFFF )
                    );
                } else if(fe_count == 1) {
                    fe_channel = parse_to16(tokens[i]);
                }
                fe_count++;
            } else if(type == "detector") {
                // parse detector related tokens
                if(det_count == 0) {
                    det_name_str = tokens[i];
                    det_name = parse_to32(det_name_str);
                } else if(det_count == 1) {
                    det_plane_str = tokens[i];
                    det_plane = parse_to16(det_plane_str);
                } else if(det_count == 2) {
                    det_segment = parse_to8(tokens[i]);
                } else if(det_count == 3) {
                    det_channel_str = tokens[i];
                    det_channel = parse_to32(det_channel_str);
                }
                det_count++;
            } // if type is fe or detector
        } // for loop for tokens

        ChannelMapSimpleItem_FE fe_item(fe_ip_3rd_4th >> 8, fe_ip_3rd_4th & 0xFF, fe_channel);
        ChannelMapSimpleItem_DET det_item;
        det_item.name = det_name;
        det_item.plane = det_plane;
        det_item.segment = det_segment;
        det_item.channel = det_channel;
        return ChannelMapSimpleItem{fe_item, det_item};
    }// ChannelMapSimpleItem ChannelMapSimple::makeSimpleItem

    void ChannelMapSimple::simplify_detector_names(){
        // 1st is original name, 2nd is simplified name(uint32_t)
        // if simplified name is shrter than 4 char, fill with space char in the end
        const std::vector<std::pair<std::string, std::string>> detnames = {
            {"utof", "UTOF"},
            {"dtof", "DTOF"},
            {"ltof", "LTOF"},
            {"t0", "T0  "},
            {"t0ref", "T0RF"},
            {"t1", "T1  "},
            {"all_charged", "ALCH"},
            {"bftref", "BFTR"},
            {"bdc", "BDC "},
            {"kldc", "KLDC"},
            {"left", "LEFT"},
            {"right", "RIGT"},
            {"bottom", "BOTM"},
            {"upstream", "UPST"},
            {"downstream", "DOST"},
        };
        // make simplified map
        for(const auto& name_pair : detnames){
            uint32_t simplified = four_char_to_uint32(
                name_pair.second[0],
                name_pair.second[1],
                name_pair.second[2],
                name_pair.second[3]
            );
            mapdata_string_simplify_map32[name_pair.first] = simplified;
        }

        // 1st is original name, 2nd is simplified name(uint16_t)
        const std::vector<std::pair<std::string, std::string>> detplanes = {
            {"X", "X "},
            {"U", "U "},
            {"V", "V "},
            {"Xp", "XP"},
            {"Up", "UP"},
            {"Vp", "VP"},
            
        };
        // make simplified map
        for(const auto& plane_pair : detplanes){
            uint16_t simplified = four_char_to_uint16(
                plane_pair.second[0],
                plane_pair.second[1]
            );
            mapdata_string_simplify_map16[plane_pair.first] = simplified;
        }
    }// void ChannelMapSimple::simplify_detector_names

    uint32_t ChannelMapSimple::four_char_to_uint32(char a, char b, char c, char d) {
        // 4つのcharをuint32_tに変換するルールを規定
        return (uint32_t(uint8_t(a)) << 24) | (uint32_t(uint8_t(b)) << 16) | (uint32_t(uint8_t(c)) << 8) | uint32_t(uint8_t(d));
    }// uint32_t ChannelMapSimple::four_char_to_uint32

    uint16_t ChannelMapSimple::four_char_to_uint16(char a, char b) {
        // 2つのcharをuint16_tに変換するルールを規定
        return (uint16_t(uint8_t(a)) << 8) | uint16_t(uint8_t(b));
    }// uint16_t ChannelMapSimple::four_char_to_uint16

    bool ChannelMapSimple::isTokenNumeric(const std::string& token) {
        // return true if token is numeric
        return !token.empty() && std::all_of(token.begin(), token.end(), ::isdigit);
    } // bool ChannelMapSimple::isTokenNumeric

    uint32_t ChannelMapSimple::parse_to32(const std::string& token) {
        // assuming token is for example "0", "utof", "t0", "all_charged", "200", and parse to "00000000", "55544F46", "54302020", "414C4348", "000000C8" respectively
        if (isTokenNumeric(token)) {
            return static_cast<uint32_t>(std::stoul(token));
        } else {
            auto it = mapdata_string_simplify_map32.find(token);// check in detname_simplify_map
            if(it == mapdata_string_simplify_map32.end()) {
                std::cerr << "unknown token for uint32_t conversion: " << token << std::endl;
                std::exit(1);
            } else {
                uint32_t simplified = it->second;
                return simplified;
            }
        }
    }// uint32_t ChannelMapSimple::parse_to32

    uint16_t ChannelMapSimple::parse_to16(const std::string& token) {
        if (isTokenNumeric(token)) {
            return static_cast<uint16_t>(std::stoul(token));
        } else {
            auto it = mapdata_string_simplify_map16.find(token);// check in detname_simplify_map
            if(it == mapdata_string_simplify_map16.end()) {
                std::cerr << "unknown token for uint16_t conversion: " << token << std::endl;
                std::exit(1);
            } else {
                uint16_t simplified = it->second;
                return simplified;
            }
        }
    }// uint16_t ChannelMapSimple::parse_to16
}// namespace chmap