#include <channel_map_simple.hpp>
#include <channel_map_simple_item.hpp>
#include <string>
#include <iostream>

/*
mapdata.csvのファイルパスを与えるとchannel-map-simpleの動作テストをする
*/
int main(int argc, char* argv[]) {
    std::string input_file_path = argv[1];
    chmap::ChannelMapSimple& channel_map_simple = chmap::ChannelMapSimple::get_instance();
    channel_map_simple.initialize(input_file_path);

    uint8_t test_ip3rd_T1right = 0x02;
    uint8_t test_ip4th_T1right = 0xAA;
    uint16_t test_ch_T1right = 12;

    chmap::ChannelMapSimpleItem_DET& det_item_T1right = channel_map_simple.getDETItem(test_ip3rd_T1right, test_ip4th_T1right, test_ch_T1right);
    std::cout << "DET item for FE IP 2.170 and channel 12:" << std::endl;
    std::cout << "  name: " << std::hex << det_item_T1right.name << std::dec << std::endl;
    std::cout << "  plane: " << std::hex << det_item_T1right.plane << std::dec << std::endl;
    std::cout << "  segment: " << static_cast<uint32_t>(det_item_T1right.segment) << std::endl;
    std::cout << "  channel: " << std::hex << det_item_T1right.channel << std::dec << std::endl;

    return 0;
}