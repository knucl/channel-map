#ifndef CHANNEL_MAP_SIMPLE_HPP_
#define CHANNEL_MAP_SIMPLE_HPP_

#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>

namespace chmap {

    class ChannelMapSimple {
        public:
            static ChannelMapSimple& get_instance();
            ~ChannelMapSimple();

            void initialize(const std::string& file_path);
            std::vector<ChannelMapSimpleItem> ChannelMapSimple::channel_map_simple_items;
            std::unordered_map<std::string, uint32_t> mapdata_string_simplify_map32;
            std::unordered_map<std::string, uint16_t> mapdata_string_simplify_map16;
        private:
            std::vector<std::string> split_line(const std::string& line, char delimiter = ',');
            std::vector<std::string> m_header, m_element_type, m_unique_types;
            ChannelMapSimpleItem makeSimpleItem(const std::vector<std::string>& tokens);
            void simplify_detector_names();
            uint32_t four_char_to_uint32(char a, char b, char c, char d);
            uint16_t four_char_to_uint16(char a, char b);
            bool isTokenNumeric(const std::string& token);
            uint32_t parse_to32(const std::string& token);
            uint16_t parse_to16(const std::string& token);
            uint8_t parse_to8(const std::string& token);


            
    };// class ChannelMapSimple
}// namespace chmap

#endif // CHANNEL_MAP_SIMPLE_HPP_