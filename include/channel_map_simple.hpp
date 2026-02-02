#ifndef CHANNEL_MAP_SIMPLE_HPP_
#define CHANNEL_MAP_SIMPLE_HPP_

#include <vector>
#include <string>
#include <filesystem>

namespace chmap {

    class ChannelMapSimple {
        public:
            static ChannelMapSimple& get_instance();
            ~ChannelMapSimple();

            void initialize(const std::string& file_path);
    };// class ChannelMapSimple
}// namespace chmap

#endif // CHANNEL_MAP_SIMPLE_HPP_